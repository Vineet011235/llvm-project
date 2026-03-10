#include "llvm/Analysis/CustomDataFlow.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"  // *** PHI HANDLING: For detecting debug intrinsics ***
#include "llvm/IR/CFG.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include <vector>
#include <set>

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

static cl::opt<bool> DebugMode(
    "df-debug", cl::Hidden, cl::desc("Enable debug output to stderr"));

// --- Debug Output Macro ---
// Returns either errs() for debug output or nulls() to discard output
#define DEBUG_OUT (DebugMode ? errs() : llvm::nulls())


// --- Main Dispatcher ---
PreservedAnalyses CustomDataFlowPass::run(Function &F, FunctionAnalysisManager &FAM) {
    if (RunLiveVars) {
        DEBUG_OUT << "--- Running Live Variable Analysis ---\n";
        computeLiveVariables(F);
    }
    
    if (RunReachingDefs) {
        DEBUG_OUT << "--- Running Reaching Definitions ---\n";
        computeReachingDefinitions(F);
    }
    
    if (RunAvailExprs) {
        DEBUG_OUT << "--- Running Available Expressions ---\n";
        computeAvailableExpressions(F);
    }
    
    if (RunAnticExprs) {
        DEBUG_OUT << "--- Running Anticipable Expressions ---\n";
        computeAnticipableExpressions(F);
    }

    // Since this pass only analyzes and prints, it doesn't modify the IR
    return PreservedAnalyses::all();
}

// --- Utility Functions ---
BitVector UNION(const BitVector &A, const BitVector &B) {
    BitVector Result = A;
    Result |= B;
    return Result;
}

BitVector INTERSECTION(const BitVector &A, const BitVector &B) {
    BitVector Result = A;
    Result &= B;
    return Result;
}

BitVector DIFFERENCE(const BitVector &A, const BitVector &B) {
    BitVector Result = A;
    Result.reset(B);
    return Result;
}

void UNION(const BitVector &A, const BitVector &B, BitVector &Result) {
    Result = A;
    Result |= B;
}

void INTERSECTION(const BitVector &A, const BitVector &B, BitVector &Result) {
    Result = A;
    Result &= B;
}

void DIFFERENCE(const BitVector &A, const BitVector &B, BitVector &Result) {
    Result = A;
    BitVector Temp = B;
    Result.reset(Temp);
} 

// In-place versions for efficiency
void UNION_INPLACE(BitVector &Result, const BitVector &Input) {
    Result |= Input;
}

void INTERSECTION_INPLACE(BitVector &Result, const BitVector &Input) {
    Result &= Input;
}

void DIFFERENCE_INPLACE(BitVector &Result, const BitVector &Input) {
    Result.reset(Input);
}

// Initialization Function (if needed)
BitVector PHI_INIT(unsigned Size) {
    return BitVector(Size, false);
}

BitVector UNIVERSAL_INIT(unsigned Size) {
    return BitVector(Size, true);
}


// Print Utility for BitVector (for debugging)
void printBitVector(const BitVector &BV, const std::vector<Value*> &IndexToValue, raw_ostream &OS = outs()) {
    OS << "{ ";
    bool first = true;
    for (unsigned i = 0; i < BV.size(); ++i) {
        if (BV.test(i)) {
            if (!first) {
                OS << ", ";
            }
            first = false;
            Value *V = IndexToValue[i];
            V->printAsOperand(OS, false);
        }
    }
    if(BV.count() == 0) {
        OS << "  ";
    }
    OS << " }\n";
}

void showBitVector(const BitVector &BV, const std::vector<Value*> &IndexToValue, raw_ostream &OS = outs()) {
    // Just Show 0 1
    OS << "BitVector: |";
    for (unsigned i = 0; i < BV.size(); ++i) {
        // Value *V = IndexToValue[i];
        // V->printAsOperand(OS, false);
        // OS << ": " << (BV.test(i) ? "1" : "0") << "| ";

        OS << (BV.test(i) ? "1" : "0") ;
    }
    OS << "\n";

}

// --- Live Variable Analysis Implementation ---
void CustomDataFlowPass::computeLiveVariables(Function &F) {
    // 1. How to represent the domain of variables?
        llvm::DenseSet<Value*> Domain; // Using a set to avoid duplicates and maintain order
        // 1.1 Process all the variables in the function to build the domain
        // 1.1.1 Arguments
        for (Argument &Arg : F.args()) {
            Domain.insert(&Arg);
            DEBUG_OUT << "➕ Arg : " << Arg.getName() << "\n";
        }

        // 1.1.2 Instructions
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB)
                if (!I.getType()->isVoidTy()) // Variable Definitions are those that produce a value (non-void)
                Domain.insert(&I);
            DEBUG_OUT << "---------\n";
        }

        // 1.2 Create mappings for BitVector indices
        std::vector<Value*> IndexToValue;
        DenseMap<Value*, unsigned> ValueToIndex;
        
        // Copy domain to vector for sorting
        for (Value *V : Domain) {
            IndexToValue.push_back(V);
        }
        
        // Sort the domain elements for better visualization
        // Sort order: Arguments first (alphabetically), then Instructions (by type, then name)
        std::sort(IndexToValue.begin(), IndexToValue.end(), [](Value *A, Value *B) {
            bool AIsArg = isa<Argument>(A);
            bool BIsArg = isa<Argument>(B);
            
            // Arguments come first
            if (AIsArg && !BIsArg) return true;
            if (!AIsArg && BIsArg) return false;
            
            // Both are arguments - sort by name
            if (AIsArg && BIsArg) {
                return cast<Argument>(A)->getName() < cast<Argument>(B)->getName();
            }
            
            // Both are instructions - sort by opcode name, then by value name
            Instruction *InstA = cast<Instruction>(A);
            Instruction *InstB = cast<Instruction>(B);
            
            std::string OpcodeA = InstA->getOpcodeName();
            std::string OpcodeB = InstB->getOpcodeName();
            
            if (OpcodeA != OpcodeB) {
                return OpcodeA < OpcodeB;
            }
            
            // Same opcode - compare by string representation for consistency
            std::string StrA, StrB;
            llvm::raw_string_ostream OSA(StrA), OSB(StrB);
            A->printAsOperand(OSA, false);
            B->printAsOperand(OSB, false);
            return OSA.str() < OSB.str();
        });
        
        // Build index mapping after sorting
        unsigned Index = 0;
        for (Value *V : IndexToValue) {
            ValueToIndex[V] = Index++;
        }

        // Debug: Print the mappings
        DEBUG_OUT << "Value to Index Mapping:\n";
        for (const auto &Pair : ValueToIndex) {
            DEBUG_OUT << "  " << *Pair.first << " -> " << Pair.second << "\n";
        }

    // 2. Define IN, OUT, GEN, KILL sets for each basic block

        // 2.1 IN, OUT are to be initialized 
            // Since it's backward with ANY-PATH
            DenseMap<BasicBlock*, BitVector> IN, OUT;
            for (BasicBlock &BB : F) {
                IN[&BB] = PHI_INIT(Domain.size()); // Start with empty set
                OUT[&BB] = PHI_INIT(Domain.size()); // Start with empty since meet Operation is UNION
            }

        // 2.2 GEN, KILL are to be computed based on the instructions in each block
            DenseMap<BasicBlock*, BitVector> GEN, KILL;
            for (BasicBlock &BB : F) {
                DEBUG_OUT << "🥹 Processing BB: " << BB.getName() << "\n";
                BitVector GenSet = PHI_INIT(Domain.size());
                BitVector KillSet = PHI_INIT(Domain.size());
                for (Instruction &I : BB) {

                    DEBUG_OUT << "  👀 Processing Instruction : " << I << "\n";
                    
                    // Debug info must not affect liveness analysis
                    if (isa<DbgInfoIntrinsic>(&I)) {
                        DEBUG_OUT << "   ⚠️  Skipping debug intrinsic\n";
                        continue;
                    }
                    
                    // FORWARD : Check GEN first (uses) then KILL (defs) to allow for self-use in the same instruction (e.g., x = x + 1)

                    // GEN (uses)
                    // *** PHI HANDLING: Skip PHI node operands - they belong to predecessor edges, not this block ***
                    if (!isa<PHINode>(&I)) {
                        for (Use &U : I.operands()) {
                            Value *Op = U.get();
                            auto OpIt = ValueToIndex.find(Op);
                            if (OpIt != ValueToIndex.end()) {
                                unsigned idx = OpIt->second;
                                if (!KillSet.test(idx)) {
                                    GenSet.set(idx);
                                    DEBUG_OUT << "   ➕ Adding to GEN: ";
                                    Op->printAsOperand(DEBUG_OUT, false);
                                    DEBUG_OUT << "\n";
                                }
                                else{
                                    DEBUG_OUT << "   ⚠️  Operand is killed in the same block, not adding to GEN: ";
                                    Op->printAsOperand(DEBUG_OUT, false);
                                    DEBUG_OUT << "\n";
                                }
                            } else {
                                DEBUG_OUT << "   ❌ Operand not in domain: ";
                                Op->printAsOperand(DEBUG_OUT, false);
                                DEBUG_OUT << "\n";
                            }
                        }
                    } else {
                        // *** PHI HANDLING: PHI operands are skipped here ***
                        DEBUG_OUT << "   ⚠️  Skipping PHI operands (will be handled on edges)\n";
                    }
                    
                    // KILL (defs)
                        if (!I.getType()->isVoidTy()) { // If it produces a value, it can be killed
                            auto InstIndexIt = ValueToIndex.find(&I);
                            if(InstIndexIt == ValueToIndex.end()) {
                                DEBUG_OUT << "   ❌ Instruction not in domain (should not happen): " << I << "\n";
                                continue;
                            }
                            unsigned InstIndex = InstIndexIt->second;
                            KillSet.set(InstIndex);
                            DEBUG_OUT << "   ➕ Adding to KILL: ";
                            I.printAsOperand(DEBUG_OUT, false);
                            DEBUG_OUT << "\n";
                        }           
                }    
                GEN[&BB] = GenSet;
                KILL[&BB] = KillSet;
            }

            //DEBUG : Print GEN/KILL sets
            for (BasicBlock &BB : F) {
                DEBUG_OUT << "\nBB: " << BB.getName() << "\n";
                DEBUG_OUT << "  GEN: ";
                printBitVector(GEN[&BB], IndexToValue, DEBUG_OUT);
                DEBUG_OUT << "  KILL: ";
                printBitVector(KILL[&BB], IndexToValue, DEBUG_OUT);
            }

    // 3. Iteratively solve the data flow equations until convergence

        // 3.1 Using PostOrder Traversal for Backward Analysis
            auto PostOrderBlocks = llvm::post_order(&F);

            // Debug: Print BitVector Order
            for(int i = 0; i < IndexToValue.size(); ++i) {
                DEBUG_OUT << i << " -> ";
                IndexToValue[i]->printAsOperand(DEBUG_OUT, false);
                DEBUG_OUT << "\n";
            }

        bool Changed;
        int IterationCount = 0;
        // 3.2 Iteration Loop
            do {
                DEBUG_OUT << "\n--- Iteration " << ++IterationCount << " ---\n";
                if(IterationCount > 1000) { // Safety check to prevent infinite loops
                    DEBUG_OUT << "⚠️  Iteration limit reached, breaking out to prevent infinite loop.\n";
                    break;
                }
                Changed = false;
                for (llvm::BasicBlock *BB : PostOrderBlocks) {
                    // You are visiting BB in perfect Post-Order!
                    DEBUG_OUT << "\nProcessing BB: " << BB->getName() << "\n";
                    DEBUG_OUT << "  Current IN: ";
                    showBitVector(IN[BB], IndexToValue, DEBUG_OUT);
                    DEBUG_OUT << "  Current OUT: ";
                    showBitVector(OUT[BB], IndexToValue, DEBUG_OUT);
                    DEBUG_OUT << "  GEN: ";
                    showBitVector(GEN[BB], IndexToValue, DEBUG_OUT);
                    DEBUG_OUT << "  KILL: ";
                    showBitVector(KILL[BB], IndexToValue, DEBUG_OUT);

                    // Compute new OUT as the union of IN of successors
                    BitVector NewOUT = PHI_INIT(Domain.size());
                    for (BasicBlock *Succ : successors(BB)) {
                        UNION_INPLACE(NewOUT, IN[Succ]);
                        
                        // *** PHI HANDLING: Inject PHI operands on the correct edges ***
                        // For each PHI node in successor, add the incoming value from this block to OUT
                        for (Instruction &SuccInst : *Succ) {
                            PHINode *Phi = dyn_cast<PHINode>(&SuccInst);
                            if (!Phi) {
                                // PHI nodes are always at the beginning of a block
                                break;
                            }
                            
                            // Get the value coming from this block (BB)
                            Value *IncomingVal = Phi->getIncomingValueForBlock(BB);
                            auto IncomingIt = ValueToIndex.find(IncomingVal);
                            if (IncomingIt != ValueToIndex.end()) {
                                unsigned idx = IncomingIt->second;
                                NewOUT.set(idx);
                                DEBUG_OUT << "   🔵 PHI: Adding ";
                                IncomingVal->printAsOperand(DEBUG_OUT, false);
                                DEBUG_OUT << " to OUT[" << BB->getName() << "] for PHI in " 
                                         << Succ->getName() << "\n";
                            }
                        }
                    }

                    // Compute new IN using the data flow equation: IN = GEN ∪ (OUT - KILL)
                    BitVector NewIN = PHI_INIT(Domain.size());
                    DIFFERENCE(OUT[BB], KILL[BB], NewIN); // NewIN = OUT - KILL
                    UNION_INPLACE(NewIN, GEN[BB]); // GEN ∪ (NewIN)
                    
                    // Check for changes
                    if( NewIN != IN[BB] || NewOUT != OUT[BB]) {
                        Changed = true;
                        IN[BB] = NewIN;
                        OUT[BB] = NewOUT;
                        DEBUG_OUT << "  Updated IN: ";
                        showBitVector(IN[BB], IndexToValue, DEBUG_OUT);
                        DEBUG_OUT << "  Updated OUT: ";
                        showBitVector(OUT[BB], IndexToValue, DEBUG_OUT);
                    }
                    else{
                        DEBUG_OUT << "  No change in IN/OUT.\n";
                    }
                }
            } while (Changed);

    // 4. Print the results for each basic block
    
        // 4.0 Print Function Header
            outs() << "\n";
            outs() << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
            outs() << "║                          LIVE VARIABLE ANALYSIS                              ║\n";
            outs() << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
            outs() << "║  Function: " << F.getName() << "\n";
            outs() << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
            
        // 4.1 Print Variable Domain
            outs() << "\n╔══════════════════════════════════════════════════════════════════════════════╗\n";
            outs() << "║ VARIABLE DOMAIN\n";
            outs() << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
            outs() << "║  Size: " << IndexToValue.size() << "\n";
            outs() << "║  Elements:\n";
            outs() << "║    ";
            
            unsigned count = 0;
            unsigned itemsPerLine = 6;
            bool first = true;
            for (Value *V : IndexToValue) {
                if (!first) {
                    outs() << ", ";
                }
                if (count > 0 && count % itemsPerLine == 0) {
                    outs() << "\n║    ";
                }
                first = false;
                V->printAsOperand(outs(), false);
                count++;
            }
            outs() << "\n";
            outs() << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
            
            for (BasicBlock &BB : F) {
                outs() << "\n╔══════════════════════════════════════════════════════════════════════════════╗\n";
                outs() << "║ Basic Block: " << BB.getName() << "\n";
                outs() << "╠══════════════════════════════════════════════════════════════════════════════╣\n";

                // 4.1 Print GEN & KILL sets
                outs() << "║  GEN:  ";
                printBitVector(GEN[&BB], IndexToValue);

                outs() << "║  KILL: ";
                printBitVector(KILL[&BB], IndexToValue);

                outs() << "╟──────────────────────────────────────────────────────────────────────────────╢\n";
                outs() << "║  IN:   ";
                printBitVector(IN[&BB], IndexToValue);

                outs() << "║  OUT:  ";
                printBitVector(OUT[&BB], IndexToValue);
                outs() << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
            }
    return;
}

// --- Blank Stubs for Future Passes ---
void CustomDataFlowPass::computeReachingDefinitions(Function &F) {
    
}

void CustomDataFlowPass::computeAvailableExpressions(Function &F) {
    // TODO: Implement Available Expressions
}

void CustomDataFlowPass::computeAnticipableExpressions(Function &F) {
    // TODO: Implement Anticipable Expressions
}