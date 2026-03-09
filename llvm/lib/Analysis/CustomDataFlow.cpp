#include "llvm/Analysis/CustomDataFlow.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include <vector>

using namespace llvm;

// --- Command Line Flags ---
// These automatically register with the 'opt' tool
static cl::opt<bool> RunLiveVars(
    "df-live", cl::Hidden, cl::desc("Run Live Variable Analysis"));

static cl::opt<bool> RunReachingDefs(
    "df-reach", cl::Hidden, cl::desc("Run Reaching Definitions Analysis"));

static cl::opt<bool> RunAvailExprs(
    "df-avail", cl::Hidden, cl::desc("Run Available Expressions Analysis"));

static cl::opt<bool> RunAnticExprs(
    "df-antic", cl::Hidden, cl::desc("Run Anticipable Expressions Analysis"));


// --- Main Dispatcher ---
PreservedAnalyses CustomDataFlowPass::run(Function &F, FunctionAnalysisManager &FAM) {
    if (RunLiveVars) {
        errs() << "--- Running Live Variable Analysis ---\n";
        computeLiveVariables(F);
    }
    
    if (RunReachingDefs) {
        errs() << "--- Running Reaching Definitions ---\n";
        computeReachingDefinitions(F);
    }
    
    if (RunAvailExprs) {
        errs() << "--- Running Available Expressions ---\n";
        computeAvailableExpressions(F);
    }
    
    if (RunAnticExprs) {
        errs() << "--- Running Anticipable Expressions ---\n";
        computeAnticipableExpressions(F);
    }

    // Since this pass only analyzes and prints, it doesn't modify the IR
    return PreservedAnalyses::all();
}

// --- Live Variable Analysis Implementation ---
void CustomDataFlowPass::computeLiveVariables(Function &F) {
    std::vector<Value*> IndexToValue;
    DenseMap<Value*, unsigned> ValueToIndex;
    DenseMap<BasicBlock*, BitVector> GenSets;
    DenseMap<BasicBlock*, BitVector> KillSets;
    DenseMap<BasicBlock*, BitVector> InSets;
    DenseMap<BasicBlock*, BitVector> OutSets;

    unsigned DomainSize = 0;
    
    // 1. Build Domain Mapping
    for (Argument &Arg : F.args()) {
        ValueToIndex[&Arg] = DomainSize++;
        IndexToValue.push_back(&Arg);
    }
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (!I.getType()->isVoidTy()) {
                ValueToIndex[&I] = DomainSize++;
                IndexToValue.push_back(&I);
            }
        }
    }

    // 2. Compute GEN and KILL
    for (BasicBlock &BB : F) {
        GenSets[&BB] = BitVector(DomainSize, false);
        KillSets[&BB] = BitVector(DomainSize, false);
        InSets[&BB] = BitVector(DomainSize, false);
        OutSets[&BB] = BitVector(DomainSize, false);

        for (Instruction &I : BB) {
            for (Use &U : I.operands()) {
                Value *Op = U.get();
                auto It = ValueToIndex.find(Op);
                if (It != ValueToIndex.end() && !KillSets[&BB].test(It->second)) {
                    GenSets[&BB].set(It->second);
                }
            }
            auto It = ValueToIndex.find(&I);
            if (It != ValueToIndex.end()) {
                KillSets[&BB].set(It->second);
            }
        }
    }

    // 3. Backward Worklist Algorithm
    bool Changed = true;
    std::vector<BasicBlock*> Blocks;
    for (BasicBlock &BB : F) Blocks.push_back(&BB);

    while (Changed) {
        Changed = false;
        for (auto It = Blocks.rbegin(); It != Blocks.rend(); ++It) {
            BasicBlock *BB = *It;

            BitVector NewOut(DomainSize, false);
            for (BasicBlock *Succ : successors(BB)) {
                NewOut |= InSets[Succ];
            }
            OutSets[BB] = NewOut;

            BitVector NewIn = OutSets[BB];
            NewIn.reset(KillSets[BB]); 
            NewIn |= GenSets[BB];

            if (NewIn != InSets[BB]) {
                InSets[BB] = NewIn;
                Changed = true;
            }
        }
    }

    // 4. Print Results
    auto PrintSet = [&](const BitVector &BV, StringRef SetName) {
        errs() << "  " << SetName << ": ";
        bool First = true;
        for (int i = BV.find_first(); i != -1; i = BV.find_next(i)) {
            if (!First) errs() << ", ";
            IndexToValue[i]->printAsOperand(errs(), false);
            First = false;
        }
        errs() << "\n";
    };

    errs() << "Live Variable Results for function: " << F.getName() << "\n";
    for (BasicBlock &BB : F) {
        errs() << "Basic Block: ";
        BB.printAsOperand(errs(), false);
        errs() << "\n";
        PrintSet(GenSets[&BB], "GEN ");
        PrintSet(KillSets[&BB], "KILL");
        PrintSet(InSets[&BB], "IN  ");
        PrintSet(OutSets[&BB], "OUT ");
        errs() << "\n";
    }
}

// --- Blank Stubs for Future Passes ---
void CustomDataFlowPass::computeReachingDefinitions(Function &F) {
    // TODO: Implement Reaching Definitions
}

void CustomDataFlowPass::computeAvailableExpressions(Function &F) {
    // TODO: Implement Available Expressions
}

void CustomDataFlowPass::computeAnticipableExpressions(Function &F) {
    // TODO: Implement Anticipable Expressions
}