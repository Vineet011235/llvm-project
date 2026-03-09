#ifndef LLVM_ANALYSIS_CUSTOMDATAFLOW_H
#define LLVM_ANALYSIS_CUSTOMDATAFLOW_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"

namespace llvm {

class CustomDataFlowPass : public PassInfoMixin<CustomDataFlowPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

private:
    // The specific analysis implementations
    void computeLiveVariables(Function &F);
    void computeReachingDefinitions(Function &F);
    void computeAvailableExpressions(Function &F);
    void computeAnticipableExpressions(Function &F);
};

} // namespace llvm

#endif // LLVM_ANALYSIS_CUSTOMDATAFLOW_H