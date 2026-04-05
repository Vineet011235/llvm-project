//===- CustomIntervalAnalysis.cpp - Block interval analysis --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CustomIntervalAnalysis.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#define DEBUG_TYPE "interval-analysis"

using namespace llvm;

namespace {

using Bound = int64_t;

struct Interval {
  std::optional<Bound> Lower;
  std::optional<Bound> Upper;

  static Interval top() { return Interval{std::nullopt, std::nullopt}; }

  static Interval constant(Bound V) { return Interval{V, V}; }

  bool isTop() const { return !Lower.has_value() && !Upper.has_value(); }

  bool isConstant() const {
    return Lower.has_value() && Upper.has_value() && *Lower == *Upper;
  }

  bool operator==(const Interval &Other) const {
    return Lower == Other.Lower && Upper == Other.Upper;
  }

  bool operator!=(const Interval &Other) const { return !(*this == Other); }

  std::string str() const {
    std::string S;
    raw_string_ostream OS(S);
    OS << '[';
    if (Lower.has_value())
      OS << *Lower;
    else
      OS << "-INF";
    OS << ", ";
    if (Upper.has_value())
      OS << *Upper;
    else
      OS << "INF";
    OS << ']';
    return S;
  }
};

using BlockState = DenseMap<const Value *, Interval>;
using StateMap = DenseMap<const BasicBlock *, BlockState>;

static cl::opt<bool> EnableIntervalAnalysis(
    "df-interval",
    cl::desc("Enable interval analysis execution details (compat flag)"),
    cl::init(true), cl::Hidden);

static std::optional<Bound> safeAdd(Bound A, Bound B) {
  if ((B > 0 && A > std::numeric_limits<Bound>::max() - B) ||
      (B < 0 && A < std::numeric_limits<Bound>::min() - B))
    return std::nullopt;
  return A + B;
}

static std::optional<Bound> safeSub(Bound A, Bound B) {
  if ((B > 0 && A < std::numeric_limits<Bound>::min() + B) ||
      (B < 0 && A > std::numeric_limits<Bound>::max() + B))
    return std::nullopt;
  return A - B;
}

static std::optional<Bound> safeMul(Bound A, Bound B) {
  bool Overflow = false;
  APInt LHS(64, static_cast<uint64_t>(A), true);
  APInt RHS(64, static_cast<uint64_t>(B), true);
  APInt Product = LHS.smul_ov(RHS, Overflow);
  if (Overflow)
    return std::nullopt;
  return Product.getSExtValue();
}

static Interval joinIntervals(const Interval &A, const Interval &B) {
  Interval Result;

  if (!A.Lower.has_value() || !B.Lower.has_value())
    Result.Lower = std::nullopt;
  else
    Result.Lower = std::min(*A.Lower, *B.Lower);

  if (!A.Upper.has_value() || !B.Upper.has_value())
    Result.Upper = std::nullopt;
  else
    Result.Upper = std::max(*A.Upper, *B.Upper);

  return Result;
}

static Interval widenInterval(const Interval &OldI, const Interval &NewI) {
  Interval Result = OldI;

  if (!OldI.Lower.has_value()) {
    Result.Lower = std::nullopt;
  } else if (!NewI.Lower.has_value() || *NewI.Lower < *OldI.Lower) {
    Result.Lower = std::nullopt;
  } else {
    Result.Lower = OldI.Lower;
  }

  if (!OldI.Upper.has_value()) {
    Result.Upper = std::nullopt;
  } else if (!NewI.Upper.has_value() || *NewI.Upper > *OldI.Upper) {
    Result.Upper = std::nullopt;
  } else {
    Result.Upper = OldI.Upper;
  }

  return Result;
}

static bool tryGetInt64Constant(const Value *V, Bound &Out) {
  auto *CI = dyn_cast<ConstantInt>(V);
  if (!CI)
    return false;

  const APInt &Val = CI->getValue();
  if (Val.getBitWidth() > 64)
    return false;

  Out = Val.getSExtValue();
  return true;
}

static Interval getValueInterval(const Value *V, const BlockState &State) {
  Bound C = 0;
  if (tryGetInt64Constant(V, C))
    return Interval::constant(C);

  auto It = State.find(V);
  if (It != State.end())
    return It->second;

  if (V->getType()->isIntegerTy())
    return Interval::top();

  return Interval::top();
}

static Interval addIntervals(const Interval &A, const Interval &B) {
  if (!A.Lower.has_value() || !B.Lower.has_value())
    return Interval::top();
  if (!A.Upper.has_value() || !B.Upper.has_value())
    return Interval::top();

  auto Lower = safeAdd(*A.Lower, *B.Lower);
  auto Upper = safeAdd(*A.Upper, *B.Upper);
  if (!Lower.has_value() || !Upper.has_value())
    return Interval::top();

  return Interval{Lower, Upper};
}

static Interval subIntervals(const Interval &A, const Interval &B) {
  if (!A.Lower.has_value() || !B.Lower.has_value())
    return Interval::top();
  if (!A.Upper.has_value() || !B.Upper.has_value())
    return Interval::top();

  auto Lower = safeSub(*A.Lower, *B.Upper);
  auto Upper = safeSub(*A.Upper, *B.Lower);
  if (!Lower.has_value() || !Upper.has_value())
    return Interval::top();

  return Interval{Lower, Upper};
}

static Interval mulIntervals(const Interval &A, const Interval &B) {
  if (!A.Lower.has_value() || !A.Upper.has_value() || !B.Lower.has_value() ||
      !B.Upper.has_value())
    return Interval::top();

  std::optional<Bound> C1 = safeMul(*A.Lower, *B.Lower);
  std::optional<Bound> C2 = safeMul(*A.Lower, *B.Upper);
  std::optional<Bound> C3 = safeMul(*A.Upper, *B.Lower);
  std::optional<Bound> C4 = safeMul(*A.Upper, *B.Upper);
  if (!C1.has_value() || !C2.has_value() || !C3.has_value() || !C4.has_value())
    return Interval::top();

  Bound Lower = std::min(std::min(*C1, *C2), std::min(*C3, *C4));
  Bound Upper = std::max(std::max(*C1, *C2), std::max(*C3, *C4));
  return Interval{Lower, Upper};
}

static Interval castInterval(const CastInst &CI, const Interval &Input) {
  if (Input.isTop())
    return Input;

  auto *SrcTy = dyn_cast<IntegerType>(CI.getSrcTy());
  auto *DstTy = dyn_cast<IntegerType>(CI.getDestTy());
  if (!SrcTy || !DstTy)
    return Interval::top();

  if (!Input.Lower.has_value() || !Input.Upper.has_value())
    return Interval::top();

  auto convertSigned = [&](Bound V) -> std::optional<Bound> {
    APInt Value(SrcTy->getBitWidth(), static_cast<uint64_t>(V), true);
    APInt Converted = Value;
    if (CI.getOpcode() == Instruction::SExt)
      Converted = Value.sext(DstTy->getBitWidth());
    else if (CI.getOpcode() == Instruction::ZExt)
      Converted = Value.zext(DstTy->getBitWidth());
    else if (CI.getOpcode() == Instruction::Trunc)
      Converted = Value.trunc(DstTy->getBitWidth());
    else
      return std::nullopt;

    if (Converted.getBitWidth() > 64)
      return std::nullopt;
    return Converted.getSExtValue();
  };

  auto Lower = convertSigned(*Input.Lower);
  auto Upper = convertSigned(*Input.Upper);
  if (!Lower.has_value() || !Upper.has_value())
    return Interval::top();

  Bound MinV = std::min(*Lower, *Upper);
  Bound MaxV = std::max(*Lower, *Upper);
  return Interval{MinV, MaxV};
}

static bool statesEqual(const BlockState &A, const BlockState &B) {
  if (A.size() != B.size())
    return false;

  for (const auto &KV : A) {
    auto It = B.find(KV.first);
    if (It == B.end() || It->second != KV.second)
      return false;
  }

  return true;
}

static BlockState mergePredecessorOut(const BasicBlock *BB, const StateMap &Out,
                                      const BlockState &EntrySeed,
                                      bool IsEntry) {
  BlockState In;
  bool HasAnyPredState = false;

  for (const BasicBlock *Pred : predecessors(BB)) {
    auto PredOutIt = Out.find(Pred);
    if (PredOutIt == Out.end())
      continue;

    const BlockState &PredOut = PredOutIt->second;
    if (!HasAnyPredState) {
      In = PredOut;
      HasAnyPredState = true;
      continue;
    }

    for (const auto &KV : PredOut) {
      auto It = In.find(KV.first);
      if (It == In.end())
        In[KV.first] = KV.second;
      else
        It->second = joinIntervals(It->second, KV.second);
    }
  }

  if (IsEntry) {
    if (!HasAnyPredState)
      return EntrySeed;

    for (const auto &KV : EntrySeed) {
      auto It = In.find(KV.first);
      if (It == In.end())
        In[KV.first] = KV.second;
      else
        It->second = joinIntervals(It->second, KV.second);
    }
  }

  return In;
}

static BlockState widenState(const BlockState &OldState, const BlockState &NewState,
                             const BasicBlock *BB) {
  BlockState Result = OldState;

  for (const auto &KV : NewState) {
    auto It = Result.find(KV.first);
    if (It == Result.end()) {
      Result[KV.first] = KV.second;
      continue;
    }

    Interval Widened = widenInterval(It->second, KV.second);
    if (Widened != It->second) {
      LLVM_DEBUG(dbgs() << "[interval] widen in block " << BB->getName() << ": "
                        << KV.first->getNameOrAsOperand() << " "
                        << It->second.str() << " -> " << Widened.str() << "\n");
    }
    It->second = Widened;
  }

  return Result;
}

static BlockState transferBlock(const BasicBlock &BB, const BlockState &In) {
  BlockState Current = In;

  for (const Instruction &I : BB) {
    if (!I.getType()->isIntegerTy())
      continue;

    Interval Result = Interval::top();

    if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
      Interval LHS = getValueInterval(BO->getOperand(0), Current);
      Interval RHS = getValueInterval(BO->getOperand(1), Current);
      switch (BO->getOpcode()) {
      case Instruction::Add:
        Result = addIntervals(LHS, RHS);
        break;
      case Instruction::Sub:
        Result = subIntervals(LHS, RHS);
        break;
      case Instruction::Mul:
        Result = mulIntervals(LHS, RHS);
        break;
      default:
        Result = Interval::top();
        break;
      }
    } else if (auto *CI = dyn_cast<CastInst>(&I)) {
      Interval Input = getValueInterval(CI->getOperand(0), Current);
      Result = castInterval(*CI, Input);
    } else if (auto *PN = dyn_cast<PHINode>(&I)) {
      // Path-insensitive phi handling by joining incoming value ranges.
      bool Initialized = false;
      for (unsigned Idx = 0; Idx < PN->getNumIncomingValues(); ++Idx) {
        Interval Incoming = getValueInterval(PN->getIncomingValue(Idx), Current);
        if (!Initialized) {
          Result = Incoming;
          Initialized = true;
        } else {
          Result = joinIntervals(Result, Incoming);
        }
      }
      if (!Initialized)
        Result = Interval::top();
    } else {
      Result = Interval::top();
    }

    Current[&I] = Result;
  }

  return Current;
}

static std::vector<const BasicBlock *> computeRPO(const Function &F) {
  std::vector<const BasicBlock *> Order;
  if (F.empty())
    return Order;

  const BasicBlock *Entry = &F.getEntryBlock();
  for (const BasicBlock *BB : depth_first(Entry))
    Order.push_back(BB);

  std::reverse(Order.begin(), Order.end());

  // Add unreachable blocks in function order to keep deterministic traversal.
  for (const BasicBlock &BB : F) {
    const BasicBlock *BBPtr = &BB;
    if (std::find(Order.begin(), Order.end(), BBPtr) == Order.end())
      Order.push_back(BBPtr);
  }

  return Order;
}

static std::string getStableValueName(const Value *V) {
  if (V->hasName())
    return ("0_" + V->getName()).str();

  std::string S;
  raw_string_ostream OS(S);
  V->printAsOperand(OS, false);
  return "1_" + OS.str();
}

static void printOutState(const Function &F, const StateMap &Out) {
  for (const BasicBlock &BB : F) {
    const BasicBlock *BBPtr = &BB;
    outs() << "### Basic Block: ";
    if (BB.hasName())
      outs() << '%' << BB.getName();
    else
      outs() << "<unnamed>";
    outs() << '\n';

    outs() << "| Value | Interval |\n";
    outs() << "|---|---|\n";

    auto It = Out.find(BBPtr);
    if (It == Out.end()) {
      outs() << '\n';
      continue;
    }

    std::vector<std::pair<std::string, const Value *>> Keys;
    Keys.reserve(It->second.size());
    for (const auto &KV : It->second)
      Keys.push_back({getStableValueName(KV.first), KV.first});

    llvm::sort(Keys, [](const auto &A, const auto &B) { return A.first < B.first; });

    for (const auto &Entry : Keys) {
      const Value *V = Entry.second;
      auto Sit = It->second.find(V);
      if (Sit == It->second.end())
        continue;

      std::string VS;
      raw_string_ostream VOS(VS);
      V->printAsOperand(VOS, false);

      outs() << "| " << VOS.str() << " | " << Sit->second.str() << " |\n";
    }
    outs() << '\n';
  }
}

} // namespace

PreservedAnalyses CustomIntervalAnalysisPass::run(Function &F,
                                                  FunctionAnalysisManager &AM) {
  if (!EnableIntervalAnalysis)
    return PreservedAnalyses::all();

  LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

  StateMap IN;
  StateMap OUT;

  BlockState EntrySeed;
  for (const Argument &Arg : F.args()) {
    if (Arg.getType()->isIntegerTy())
      EntrySeed[&Arg] = Interval::top();
  }

  std::vector<const BasicBlock *> RPO = computeRPO(F);
  SetVector<const BasicBlock *> Worklist;
  for (const BasicBlock *BB : RPO)
    Worklist.insert(BB);

  const BasicBlock *EntryBB = F.empty() ? nullptr : &F.getEntryBlock();

  while (!Worklist.empty()) {
    const BasicBlock *BB = Worklist.pop_back_val();
    LLVM_DEBUG(dbgs() << "[interval] process block " << BB->getName() << "\n");

    BlockState NewIn = mergePredecessorOut(BB, OUT, EntrySeed, BB == EntryBB);

    if (LI.isLoopHeader(BB)) {
      auto OldInIt = IN.find(BB);
      if (OldInIt != IN.end())
        NewIn = widenState(OldInIt->second, NewIn, BB);
    }

    IN[BB] = NewIn;

    BlockState NewOut = transferBlock(*BB, NewIn);

    auto OldOutIt = OUT.find(BB);
    if (OldOutIt == OUT.end() || !statesEqual(NewOut, OldOutIt->second)) {
      OUT[BB] = std::move(NewOut);
      for (const BasicBlock *Succ : successors(BB))
        Worklist.insert(Succ);
    }
  }

  // One bounded narrowing sweep over loop headers.
  for (const BasicBlock &BBRef : F) {
    const BasicBlock *BB = &BBRef;
    if (!LI.isLoopHeader(BB))
      continue;

    BlockState NarrowIn = mergePredecessorOut(BB, OUT, EntrySeed, BB == EntryBB);
    BlockState NarrowOut = transferBlock(*BB, NarrowIn);

    bool InChanged = true;
    auto InIt = IN.find(BB);
    if (InIt != IN.end())
      InChanged = !statesEqual(NarrowIn, InIt->second);

    bool OutChanged = true;
    auto OutIt = OUT.find(BB);
    if (OutIt != OUT.end())
      OutChanged = !statesEqual(NarrowOut, OutIt->second);

    if (InChanged || OutChanged) {
      LLVM_DEBUG(dbgs() << "[interval] narrowing update in loop header "
                        << BB->getName() << "\n");
      IN[BB] = std::move(NarrowIn);
      OUT[BB] = std::move(NarrowOut);
    }
  }

  printOutState(F, OUT);

  return PreservedAnalyses::all();
}
