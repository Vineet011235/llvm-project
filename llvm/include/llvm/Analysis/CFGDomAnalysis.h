#ifndef LLVM_ANALYSIS_CFGDOMANALYSIS_H
#define LLVM_ANALYSIS_CFGDOMANALYSIS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <vector>
#include <map>
#include <set>           
#include <algorithm>
#include <string>

namespace llvm {

class CFGraph; 
class Function;

class CFGDomAnalysisPass : public PassInfoMixin<CFGDomAnalysisPass> {
public:
  PreservedAnalyses run(Function &F, AnalysisManager<Function> &AM);
};

// This will represent a basic block in our CFG
class MyBasicBlock {
  friend class CFGraph;
public:
  MyBasicBlock(BasicBlock &BB);

  const std::string &getName() const { return Name; }
  Instruction *getTerminator() const { return TerminatorInst; }
  const std::vector<Instruction *> &getInstructions() const {
    return BlockInstructions;
  }

  const std::vector<MyBasicBlock *> &getSuccessors() const { return Successors; }
  const std::vector<MyBasicBlock *> &getPredecessors() const { return Predecessors; }

  const std::set<MyBasicBlock *> &getDominators() const { return Dominators; }
  MyBasicBlock *getImmediateDominator() const { return IDom; }

private:
  BasicBlock *BBRef;
  std::string Name;
  std::vector<Instruction *> BlockInstructions;
  Instruction *TerminatorInst;
  std::vector<MyBasicBlock *> Successors;
  std::vector<MyBasicBlock *> Predecessors;

  // Dominator information
  std::set<MyBasicBlock *> Dominators;
  MyBasicBlock *IDom;

  static inline int idCounter = 0;
  static int getNextID() { return idCounter++; }
};



// This will the CFG representation for each Function
class CFGraph {
  friend class DOMTree; // Allow DOMTree to access private members of CFGraph
private:
  std::vector<std::unique_ptr<MyBasicBlock>> Blocks;
  std::vector<std::pair<MyBasicBlock *, MyBasicBlock *>> Edges;
  std::map<BasicBlock *, MyBasicBlock *> BBMap;

  MyBasicBlock* EntryBlock; // Track the entry point

  // Helper methods
  void dfsPostOrder(MyBasicBlock* node, std::set<MyBasicBlock*>& visited, 
                    std::vector<MyBasicBlock*>& postOrder);
  void calculateDominators();

public:
  CFGraph(Function &F);
  void emitDot(const std::string &FilePath) const;
};

class DOMTree {
  // Key = Parent (Dominator), Value = Children (Dominated Nodes)
  std::map<MyBasicBlock*, std::vector<MyBasicBlock*>> adjList;

public:
  DOMTree(const CFGraph &CFG);
  void emitDot(const std::string &FilePath) const;
};

} // namespace llvm

#endif // LLVM_ANALYSIS_CFGDOMANALYSIS_H
