//===- CustomIntervalAnalysis.cpp - Block interval analysis --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CustomIntervalAnalysis.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
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
#include <deque>
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
    if (isConstant())
      return "{" + std::to_string(*Lower) + "}";

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

struct Box {
  std::vector<Interval> Intervals;

  static Box top() { return Box{{Interval::top()}}; }

  static Box constant(Bound V) { return Box{{Interval::constant(V)}}; }

  bool isTop() const {
    return Intervals.size() == 1 && Intervals.front().isTop();
  }

  static int compareLower(const std::optional<Bound> &A,
                          const std::optional<Bound> &B) {
    if (!A.has_value() && !B.has_value())
      return 0;
    if (!A.has_value())
      return -1;
    if (!B.has_value())
      return 1;
    if (*A < *B)
      return -1;
    if (*A > *B)
      return 1;
    return 0;
  }

  static int compareUpper(const std::optional<Bound> &A,
                          const std::optional<Bound> &B) {
    if (!A.has_value() && !B.has_value())
      return 0;
    if (!A.has_value())
      return 1;
    if (!B.has_value())
      return -1;
    if (*A < *B)
      return -1;
    if (*A > *B)
      return 1;
    return 0;
  }

  static std::optional<Bound> minLower(const std::optional<Bound> &A,
                                       const std::optional<Bound> &B) {
    return compareLower(A, B) <= 0 ? A : B;
  }

  static std::optional<Bound> maxUpper(const std::optional<Bound> &A,
                                       const std::optional<Bound> &B) {
    return compareUpper(A, B) >= 0 ? A : B;
  }

  static bool overlapsOrTouches(const Interval &A, const Interval &B) {
    if (!A.Upper.has_value() || !B.Lower.has_value())
      return true;
    if (*B.Lower <= *A.Upper)
      return true;

    if (*A.Upper == std::numeric_limits<Bound>::max())
      return false;
    return *B.Lower <= (*A.Upper + 1);
  }

  void normalize() {
    if (Intervals.empty()) {
      Intervals.push_back(Interval::top());
      return;
    }

    llvm::sort(Intervals, [](const Interval &A, const Interval &B) {
      int CmpLower = compareLower(A.Lower, B.Lower);
      if (CmpLower != 0)
        return CmpLower < 0;
      return compareUpper(A.Upper, B.Upper) < 0;
    });

    std::vector<Interval> Merged;
    Merged.reserve(Intervals.size());
    Merged.push_back(Intervals.front());

    for (size_t I = 1; I < Intervals.size(); ++I) {
      Interval &Last = Merged.back();
      const Interval &Cur = Intervals[I];
      if (overlapsOrTouches(Last, Cur)) {
        Last.Lower = minLower(Last.Lower, Cur.Lower);
        Last.Upper = maxUpper(Last.Upper, Cur.Upper);
      } else {
        Merged.push_back(Cur);
      }
    }

    Intervals = std::move(Merged);
  }

  static Box fromInterval(const Interval &I) {
    Box B{{I}};
    B.normalize();
    return B;
  }

  static Box unite(const Box &A, const Box &B) {
    Box Result;
    Result.Intervals.reserve(A.Intervals.size() + B.Intervals.size());
    Result.Intervals.insert(Result.Intervals.end(), A.Intervals.begin(),
                            A.Intervals.end());
    Result.Intervals.insert(Result.Intervals.end(), B.Intervals.begin(),
                            B.Intervals.end());
    Result.normalize();
    return Result;
  }

  Interval hull() const {
    if (Intervals.empty())
      return Interval::top();

    Interval Result = Intervals.front();
    for (size_t I = 1; I < Intervals.size(); ++I) {
      Result.Lower = minLower(Result.Lower, Intervals[I].Lower);
      Result.Upper = maxUpper(Result.Upper, Intervals[I].Upper);
    }
    return Result;
  }

  bool operator==(const Box &Other) const {
    return Intervals == Other.Intervals;
  }

  bool operator!=(const Box &Other) const { return !(*this == Other); }

  std::string str() const {
    std::string S;
    raw_string_ostream OS(S);
    for (size_t I = 0; I < Intervals.size(); ++I) {
      if (I != 0)
        OS << " U ";
      OS << Intervals[I].str();
    }
    return S;
  }
};

using BlockState = DenseMap<const Value *, Box>;
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

static Interval narrowInterval(const Interval &OldI, const Interval &NewI) {
  Interval Result = OldI;
  Result.Lower = OldI.Lower.has_value() ? OldI.Lower : NewI.Lower;
  Result.Upper = OldI.Upper.has_value() ? OldI.Upper : NewI.Upper;
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

static Box getValueInterval(const Value *V, const BlockState &State) {
  Bound C = 0;
  if (tryGetInt64Constant(V, C))
    return Box::constant(C);

  auto It = State.find(V);
  if (It != State.end())
    return It->second;

  if (V->getType()->isIntegerTy())
    return Box::top();

  return Box::top();
}

static Interval addIntervals(const Interval &A, const Interval &B) {
  std::optional<Bound> Lower = std::nullopt;
  std::optional<Bound> Upper = std::nullopt;

  if (A.Lower.has_value() && B.Lower.has_value()) {
    Lower = safeAdd(*A.Lower, *B.Lower);
    if (!Lower.has_value())
      return Interval::top();
  }

  if (A.Upper.has_value() && B.Upper.has_value()) {
    Upper = safeAdd(*A.Upper, *B.Upper);
    if (!Upper.has_value())
      return Interval::top();
  }

  return Interval{Lower, Upper};
}

static Interval subIntervals(const Interval &A, const Interval &B) {
  std::optional<Bound> Lower = std::nullopt;
  std::optional<Bound> Upper = std::nullopt;

  if (A.Lower.has_value() && B.Upper.has_value()) {
    Lower = safeSub(*A.Lower, *B.Upper);
    if (!Lower.has_value())
      return Interval::top();
  }

  if (A.Upper.has_value() && B.Lower.has_value()) {
    Upper = safeSub(*A.Upper, *B.Lower);
    if (!Upper.has_value())
      return Interval::top();
  }

  return Interval{Lower, Upper};
}

static Interval mulIntervals(const Interval &A, const Interval &B) {
  auto scaleByConstant = [](const Interval &I, Bound C) -> Interval {
    if (C == 0)
      return Interval::constant(0);

    std::optional<Bound> Lower = std::nullopt;
    std::optional<Bound> Upper = std::nullopt;

    if (C > 0) {
      if (I.Lower.has_value()) {
        Lower = safeMul(*I.Lower, C);
        if (!Lower.has_value())
          return Interval::top();
      }
      if (I.Upper.has_value()) {
        Upper = safeMul(*I.Upper, C);
        if (!Upper.has_value())
          return Interval::top();
      }
    } else {
      if (I.Upper.has_value()) {
        Lower = safeMul(*I.Upper, C);
        if (!Lower.has_value())
          return Interval::top();
      }
      if (I.Lower.has_value()) {
        Upper = safeMul(*I.Lower, C);
        if (!Upper.has_value())
          return Interval::top();
      }
    }

    return Interval{Lower, Upper};
  };

  if (A.isConstant())
    return scaleByConstant(B, *A.Lower);
  if (B.isConstant())
    return scaleByConstant(A, *B.Lower);

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

static Box joinBoxes(const Box &A, const Box &B) { return Box::unite(A, B); }

static std::optional<Interval> intersectIntervals(const Interval &A,
                                                  const Interval &B) {
  std::optional<Bound> Lower =
      Box::compareLower(A.Lower, B.Lower) >= 0 ? A.Lower : B.Lower;
  std::optional<Bound> Upper =
      Box::compareUpper(A.Upper, B.Upper) <= 0 ? A.Upper : B.Upper;

  if (Lower.has_value() && Upper.has_value() && *Lower > *Upper)
    return std::nullopt;
  return Interval{Lower, Upper};
}

static std::optional<Box> intersectBoxes(const Box &A, const Box &B) {
  std::vector<Interval> ResultIntervals;
  ResultIntervals.reserve(A.Intervals.size() * B.Intervals.size());
  for (const Interval &IA : A.Intervals) {
    for (const Interval &IB : B.Intervals) {
      std::optional<Interval> Intersected = intersectIntervals(IA, IB);
      if (Intersected.has_value())
        ResultIntervals.push_back(*Intersected);
    }
  }

  if (ResultIntervals.empty())
    return std::nullopt;

  Box Result{std::move(ResultIntervals)};
  Result.normalize();
  return Result;
}

static Box widenBox(const Box &OldB, const Box &NewB) {
  Interval Widened = widenInterval(OldB.hull(), NewB.hull());
  return Box::fromInterval(Widened);
}

static Box narrowBox(const Box &OldB, const Box &NewB) {
  Interval Narrowed = narrowInterval(OldB.hull(), NewB.hull());
  return Box::fromInterval(Narrowed);
}

static Box addBoxes(const Box &A, const Box &B) {
  std::vector<Interval> ResultIntervals;
  ResultIntervals.reserve(A.Intervals.size() * B.Intervals.size());
  for (const Interval &IA : A.Intervals) {
    for (const Interval &IB : B.Intervals) {
      Interval R = addIntervals(IA, IB);
      if (R.isTop())
        return Box::top();
      ResultIntervals.push_back(R);
    }
  }
  Box Result{std::move(ResultIntervals)};
  Result.normalize();
  return Result;
}

static Box subBoxes(const Box &A, const Box &B) {
  std::vector<Interval> ResultIntervals;
  ResultIntervals.reserve(A.Intervals.size() * B.Intervals.size());
  for (const Interval &IA : A.Intervals) {
    for (const Interval &IB : B.Intervals) {
      Interval R = subIntervals(IA, IB);
      if (R.isTop())
        return Box::top();
      ResultIntervals.push_back(R);
    }
  }
  Box Result{std::move(ResultIntervals)};
  Result.normalize();
  return Result;
}

static Box mulBoxes(const Box &A, const Box &B) {
  std::vector<Interval> ResultIntervals;
  ResultIntervals.reserve(A.Intervals.size() * B.Intervals.size());
  for (const Interval &IA : A.Intervals) {
    for (const Interval &IB : B.Intervals) {
      Interval R = mulIntervals(IA, IB);
      if (R.isTop())
        return Box::top();
      ResultIntervals.push_back(R);
    }
  }
  Box Result{std::move(ResultIntervals)};
  Result.normalize();
  return Result;
}

static Box castBox(const CastInst &CI, const Box &Input) {
  std::vector<Interval> ResultIntervals;
  ResultIntervals.reserve(Input.Intervals.size());
  for (const Interval &I : Input.Intervals) {
    Interval R = castInterval(CI, I);
    if (R.isTop())
      return Box::top();
    ResultIntervals.push_back(R);
  }
  Box Result{std::move(ResultIntervals)};
  Result.normalize();
  return Result;
}

static const Value *getMemorySlot(const Value *Ptr) {
  return Ptr ? Ptr->stripPointerCasts() : nullptr;
}

static void applySuccessorPhiEdges(const BasicBlock &BB, BlockState &State) {
  for (const BasicBlock *Succ : successors(&BB)) {
    for (const Instruction &SuccInst : *Succ) {
      const auto *Phi = dyn_cast<PHINode>(&SuccInst);
      if (!Phi)
        break;

      const Value *IncomingValue = Phi->getIncomingValueForBlock(&BB);
      if (!IncomingValue)
        continue;

      State[Phi] = getValueInterval(IncomingValue, State);
    }
  }
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

static std::optional<Box> getConstraintForPredicate(CmpInst::Predicate Pred,
                                                    Bound C) {
  switch (Pred) {
  case CmpInst::ICMP_SLT: {
    auto Upper = safeSub(C, 1);
    if (!Upper.has_value())
      return std::nullopt;
    return Box::fromInterval(Interval{std::nullopt, *Upper});
  }
  case CmpInst::ICMP_SLE:
    return Box::fromInterval(Interval{std::nullopt, C});
  case CmpInst::ICMP_SGT: {
    auto Lower = safeAdd(C, 1);
    if (!Lower.has_value())
      return std::nullopt;
    return Box::fromInterval(Interval{*Lower, std::nullopt});
  }
  case CmpInst::ICMP_SGE:
    return Box::fromInterval(Interval{C, std::nullopt});
  case CmpInst::ICMP_EQ:
    return Box::constant(C);
  case CmpInst::ICMP_NE: {
    std::vector<Interval> Intervals;
    if (auto Upper = safeSub(C, 1); Upper.has_value())
      Intervals.push_back(Interval{std::nullopt, *Upper});
    if (auto Lower = safeAdd(C, 1); Lower.has_value())
      Intervals.push_back(Interval{*Lower, std::nullopt});
    if (Intervals.empty())
      return std::nullopt;
    Box Result{std::move(Intervals)};
    Result.normalize();
    return Result;
  }
  default:
    return std::nullopt;
  }
}

static std::optional<BlockState> refinePredecessorForEdge(const BasicBlock *Pred,
                                                          const BasicBlock *Succ,
                                                          const BlockState &PredOut) {
  BlockState Refined = PredOut;

  const auto *BI = dyn_cast<BranchInst>(Pred->getTerminator());
  if (!BI || !BI->isConditional())
    return Refined;

  unsigned SuccIndex = BI->getSuccessor(0) == Succ ? 0 : BI->getSuccessor(1) == Succ ? 1 : 2;
  if (SuccIndex >= 2)
    return Refined;

  const auto *Cmp = dyn_cast<ICmpInst>(BI->getCondition());
  if (!Cmp)
    return Refined;

  const Value *Var = Cmp->getOperand(0);
  const Value *ConstV = Cmp->getOperand(1);
  CmpInst::Predicate PredKind = Cmp->getPredicate();

  Bound C = 0;
  if (!tryGetInt64Constant(ConstV, C)) {
    Bound LeftC = 0;
    if (!tryGetInt64Constant(Var, LeftC))
      return Refined;

    Var = Cmp->getOperand(1);
    C = LeftC;
    PredKind = ICmpInst::getSwappedPredicate(PredKind);
  }

  if (!Var->getType()->isIntegerTy())
    return Refined;

  if (SuccIndex == 1)
    PredKind = Cmp->getInversePredicate(PredKind);

  std::optional<Box> Constraint = getConstraintForPredicate(PredKind, C);
  if (!Constraint.has_value())
    return Refined;

  Box CurrentValue = getValueInterval(Var, PredOut);
  std::optional<Box> Intersected = intersectBoxes(CurrentValue, *Constraint);
  if (!Intersected.has_value()) {
    LLVM_DEBUG(dbgs() << "[interval] infeasible edge " << Pred->getName() << " -> "
                      << Succ->getName() << " for "
                      << Var->getNameOrAsOperand() << " under predicate "
                      << CmpInst::getPredicateName(PredKind) << " " << C << "\n");
    return std::nullopt;
  }

  if (*Intersected != CurrentValue) {
    LLVM_DEBUG(dbgs() << "[interval] edge refine " << Pred->getName() << " -> "
                      << Succ->getName() << ": "
                      << Var->getNameOrAsOperand() << " " << CurrentValue.str()
                      << " => " << Intersected->str() << "\n");
  }

  Refined[Var] = *Intersected;
  return Refined;
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

    std::optional<BlockState> RefinedPredState =
        refinePredecessorForEdge(Pred, BB, PredOutIt->second);
    if (!RefinedPredState.has_value())
      continue;

    const BlockState &PredOut = *RefinedPredState;
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
        It->second = joinBoxes(It->second, KV.second);
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
        It->second = joinBoxes(It->second, KV.second);
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

    Box Widened = widenBox(It->second, KV.second);
    if (Widened != It->second) {
      LLVM_DEBUG(dbgs() << "[interval] widen in block " << BB->getName() << ": "
                        << KV.first->getNameOrAsOperand() << " "
                        << It->second.str() << " -> " << Widened.str() << "\n");
    }
    It->second = Widened;
  }

  return Result;
}

static BlockState narrowState(const BlockState &OldState,
                              const BlockState &NewState,
                              const BasicBlock *BB) {
  BlockState Result = OldState;

  for (const auto &KV : NewState) {
    auto It = Result.find(KV.first);
    if (It == Result.end()) {
      Result[KV.first] = KV.second;
      continue;
    }

    Box Narrowed = narrowBox(It->second, KV.second);
    if (Narrowed != It->second) {
      LLVM_DEBUG(dbgs() << "[interval] narrow in block " << BB->getName() << ": "
                        << KV.first->getNameOrAsOperand() << " "
                        << It->second.str() << " -> " << Narrowed.str() << "\n");
    }
    It->second = Narrowed;
  }

  return Result;
}

static BlockState transferBlock(const BasicBlock &BB, const BlockState &In,
                                const StateMap &Out) {
  BlockState Current = In;

  for (const Instruction &I : BB) {
    if (auto *AI = dyn_cast<AllocaInst>(&I)) {
      Current[AI] = Box::top();
      continue;
    }

    if (auto *SI = dyn_cast<StoreInst>(&I)) {
      const Value *Slot = getMemorySlot(SI->getPointerOperand());
      if (Slot)
        Current[Slot] = getValueInterval(SI->getValueOperand(), Current);
      continue;
    }

    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      const Value *Slot = getMemorySlot(LI->getPointerOperand());
      if (Slot)
        Current[&I] = getValueInterval(Slot, Current);
      else
        Current[&I] = Box::top();
      continue;
    }

    if (!I.getType()->isIntegerTy())
      continue;

    Box Result = Box::top();

    if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
      Box LHS = getValueInterval(BO->getOperand(0), Current);
      Box RHS = getValueInterval(BO->getOperand(1), Current);
      switch (BO->getOpcode()) {
      case Instruction::Add:
        Result = addBoxes(LHS, RHS);
        break;
      case Instruction::Sub:
        Result = subBoxes(LHS, RHS);
        break;
      case Instruction::Mul:
        Result = mulBoxes(LHS, RHS);
        break;
      default:
        Result = Box::top();
        break;
      }
    } else if (auto *CI = dyn_cast<CastInst>(&I)) {
      Box Input = getValueInterval(CI->getOperand(0), Current);
      Result = castBox(*CI, Input);
    } else if (isa<PHINode>(&I)) {
      // PHI values are attached to predecessor edges. The current block keeps
      // the merged value from IN, so the PHI instruction itself is skipped.
      Result = getValueInterval(&I, Current);
    } else if (auto *SI = dyn_cast<SelectInst>(&I)) {
      Box TrueValue = getValueInterval(SI->getTrueValue(), Current);
      Box FalseValue = getValueInterval(SI->getFalseValue(), Current);
      Result = joinBoxes(TrueValue, FalseValue);
    } else {
      Result = Box::top();
    }

    Current[&I] = Result;
  }

  return Current;
}

static std::vector<const BasicBlock *> computeRPO(const Function &F) {
  std::vector<const BasicBlock *> Order;
  if (F.empty())
    return Order;

  ReversePostOrderTraversal<const Function *> RPO(&F);
  for (const BasicBlock *BB : RPO)
    Order.push_back(BB);

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

static void printOutState(const Function &F, const StateMap &In,
                          const StateMap &Out) {
  for (const BasicBlock &BB : F) {
    const BasicBlock *BBPtr = &BB;

    std::vector<std::pair<std::string, const Value *>> Keys;
    auto InIt = In.find(BBPtr);
    auto OutIt = Out.find(BBPtr);
    if (InIt != In.end()) {
      for (const auto &KV : InIt->second)
        Keys.push_back({getStableValueName(KV.first), KV.first});
    }
    if (OutIt != Out.end()) {
      for (const auto &KV : OutIt->second)
        Keys.push_back({getStableValueName(KV.first), KV.first});
    }

    if (Keys.empty()) {
      outs() << '\n';
      continue;
    }

    llvm::sort(Keys, [](const auto &A, const auto &B) { return A.first < B.first; });
    Keys.erase(std::unique(Keys.begin(), Keys.end(),
                           [](const auto &A, const auto &B) {
                             return A.first == B.first;
                           }),
               Keys.end());

    std::vector<std::tuple<std::string, std::string, std::string>> Rows;
    Rows.reserve(Keys.size());

    size_t ValueWidth = 8;
    size_t InWidth = 5;
    size_t OutWidth = 6;

    auto lookupBoxString = [&](const BlockState *State,
                              const Value *V) -> std::string {
      if (!State)
        return "-";
      auto It = State->find(V);
      if (It == State->end())
        return "-";
      return It->second.str();
    };

    for (const auto &Entry : Keys) {
      const Value *V = Entry.second;
      std::string VS;
      raw_string_ostream VOS(VS);
      V->printAsOperand(VOS, false);

      std::string InText = lookupBoxString(InIt != In.end() ? &InIt->second : nullptr,
                                          V);
      std::string OutText = lookupBoxString(OutIt != Out.end() ? &OutIt->second : nullptr,
                                            V);

      Rows.emplace_back(VOS.str(), InText, OutText);
      ValueWidth = std::max(ValueWidth, std::get<0>(Rows.back()).size());
      InWidth = std::max(InWidth, std::get<1>(Rows.back()).size());
      OutWidth = std::max(OutWidth, std::get<2>(Rows.back()).size());
    }

    std::string Title;
    raw_string_ostream TS(Title);
    TS << " Basic Block: ";
    if (BB.hasName())
      TS << '%' << BB.getName();
    else
      TS << "<unnamed>";
    TS << ' ';
    TS.flush();

    size_t TableWidth = ValueWidth + InWidth + OutWidth + 10;
    size_t BannerWidth = std::max<size_t>(Title.size() + 4, TableWidth);

    auto printLine = [&](char Fill) {
      outs() << '+';
      for (size_t I = 0; I < BannerWidth - 2; ++I)
        outs() << Fill;
      outs() << "+\n";
    };

    auto printCellRow = [&](StringRef Left, size_t LeftWidth, StringRef Middle,
                            size_t MiddleWidth, StringRef Right,
                            size_t RightWidth) {
      outs() << "| " << Left;
      for (size_t I = Left.size(); I < LeftWidth; ++I)
        outs() << ' ';
      outs() << " | " << Middle;
      for (size_t I = Middle.size(); I < MiddleWidth; ++I)
        outs() << ' ';
      outs() << " | " << Right;
      for (size_t I = Right.size(); I < RightWidth; ++I)
        outs() << ' ';
      size_t RowWidth = LeftWidth + MiddleWidth + RightWidth + 10;
      for (size_t I = RowWidth; I < BannerWidth; ++I)
        outs() << ' ';
      outs() << " |\n";
    };

    printLine('-');
    outs() << "| ";
    outs() << Title;
    for (size_t I = Title.size() + 3; I < BannerWidth; ++I)
      outs() << ' ';
    outs() << "|\n";
    printLine('-');

    printCellRow("Variable", ValueWidth, "BoxIn", InWidth, "BoxOut",
                 OutWidth);
    outs() << "| ";
    for (size_t I = 0; I < ValueWidth; ++I)
      outs() << '-';
    outs() << " | ";
    for (size_t I = 0; I < InWidth; ++I)
      outs() << '-';
    outs() << " | ";
    for (size_t I = 0; I < OutWidth; ++I)
      outs() << '-';
    for (size_t I = ValueWidth + InWidth + OutWidth + 10; I < BannerWidth; ++I)
      outs() << ' ';
    outs() << " |\n";

    for (const auto &Row : Rows)
      printCellRow(std::get<0>(Row), ValueWidth, std::get<1>(Row), InWidth,
                   std::get<2>(Row), OutWidth);

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
      EntrySeed[&Arg] = Box::top();
  }

  std::vector<const BasicBlock *> RPO = computeRPO(F);
  std::deque<const BasicBlock *> Worklist;
  DenseSet<const BasicBlock *> InWorklist;
  DenseMap<const BasicBlock *, unsigned> VisitCount;
  for (const BasicBlock *BB : RPO)
    Worklist.push_back(BB), InWorklist.insert(BB);

  const BasicBlock *EntryBB = F.empty() ? nullptr : &F.getEntryBlock();

  while (!Worklist.empty()) {
    const BasicBlock *BB = Worklist.front();
    Worklist.pop_front();
    InWorklist.erase(BB);
    unsigned ThisVisit = ++VisitCount[BB];
    LLVM_DEBUG(dbgs() << "[interval] process block " << BB->getName() << "\n");

    BlockState NewIn = mergePredecessorOut(BB, OUT, EntrySeed, BB == EntryBB);

    if (LI.isLoopHeader(BB)) {
      auto OldInIt = IN.find(BB);
      if (ThisVisit > 2 && OldInIt != IN.end()) {
        LLVM_DEBUG(dbgs() << "[interval] apply widening in loop header "
                          << BB->getName() << " visit=" << ThisVisit << "\n");
        NewIn = widenState(OldInIt->second, NewIn, BB);
      } else {
        LLVM_DEBUG(dbgs() << "[interval] skip widening in loop header "
                          << BB->getName() << " visit=" << ThisVisit << "\n");
      }
    }

    IN[BB] = NewIn;

    BlockState NewOut = transferBlock(*BB, NewIn, OUT);
    applySuccessorPhiEdges(*BB, NewOut);

    auto OldOutIt = OUT.find(BB);
    if (OldOutIt == OUT.end() || !statesEqual(NewOut, OldOutIt->second)) {
      OUT[BB] = std::move(NewOut);
      for (const BasicBlock *Succ : successors(BB)) {
        if (InWorklist.insert(Succ).second)
          Worklist.push_back(Succ);
      }
    }
  }

  // Bounded narrowing sweeps over loop headers.
  constexpr unsigned MaxNarrowingSweeps = 5;
  for (unsigned Sweep = 0; Sweep < MaxNarrowingSweeps; ++Sweep) {
    bool AnyChanged = false;

    for (const BasicBlock &BBRef : F) {
      const BasicBlock *BB = &BBRef;
      if (!LI.isLoopHeader(BB))
        continue;

      BlockState MergedIn =
          mergePredecessorOut(BB, OUT, EntrySeed, BB == EntryBB);
      auto OldInIt = IN.find(BB);
      BlockState NarrowIn =
          OldInIt != IN.end() ? narrowState(OldInIt->second, MergedIn, BB)
                              : std::move(MergedIn);

      BlockState NarrowOut = transferBlock(*BB, NarrowIn, OUT);
      applySuccessorPhiEdges(*BB, NarrowOut);

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
                          << BB->getName() << " sweep=" << (Sweep + 1)
                          << "\n");
        IN[BB] = std::move(NarrowIn);
        OUT[BB] = std::move(NarrowOut);
        AnyChanged = true;
      }
    }

    if (!AnyChanged)
      break;
  }

  // Recompute a no-widening fixpoint so narrowed header facts propagate to
  // the rest of the CFG (e.g. loop exits) instead of leaving stale states.
  std::deque<const BasicBlock *> PropagateWorklist;
  DenseSet<const BasicBlock *> InPropagateWorklist;
  for (const BasicBlock *BB : RPO)
    PropagateWorklist.push_back(BB), InPropagateWorklist.insert(BB);

  while (!PropagateWorklist.empty()) {
    const BasicBlock *BB = PropagateWorklist.front();
    PropagateWorklist.pop_front();
    InPropagateWorklist.erase(BB);

    BlockState NewIn = mergePredecessorOut(BB, OUT, EntrySeed, BB == EntryBB);

    bool InChanged = true;
    auto OldInIt = IN.find(BB);
    if (OldInIt != IN.end())
      InChanged = !statesEqual(NewIn, OldInIt->second);

    if (InChanged)
      IN[BB] = NewIn;

    BlockState NewOut = transferBlock(*BB, NewIn, OUT);
    applySuccessorPhiEdges(*BB, NewOut);

    bool OutChanged = true;
    auto OldOutIt = OUT.find(BB);
    if (OldOutIt != OUT.end())
      OutChanged = !statesEqual(NewOut, OldOutIt->second);

    if (OutChanged) {
      OUT[BB] = std::move(NewOut);
      for (const BasicBlock *Succ : successors(BB)) {
        if (InPropagateWorklist.insert(Succ).second)
          PropagateWorklist.push_back(Succ);
      }
    }
  }

  printOutState(F, IN, OUT);

  return PreservedAnalyses::all();
}
