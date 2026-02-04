#include "llvm/Analysis/CFGDomAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>
#include <vector>
#include <cstdlib>

using namespace llvm;

// Command line options
static cl::opt<std::string> CFG_DOT_FOLDER(
    "cfg-dot-folder",
    cl::desc("Directory to output CFG DOT files (if empty, no files will be generated)"),
    cl::value_desc("directory"),
    cl::init("")
);

static cl::opt<bool> EMIT_DOM_TREE(
    "emit-dom-tree",
    cl::desc("Whether to emit the dominator tree DOT file"),
    cl::init(false)
);

// ============================================================================
// MyBasicBlock Implementation
// ============================================================================

MyBasicBlock::MyBasicBlock(BasicBlock &BB)
    : BBRef(&BB),
      Name("BB_" + std::to_string(getNextID())),
      TerminatorInst(BB.getTerminator()) {

  for (Instruction &I : BB)
    BlockInstructions.push_back(&I);
}

// ============================================================================
// CFGraph Implementation
// ============================================================================

CFGraph::CFGraph(Function &F) {
  errs() << "Constructing CFGraph for function: " << F.getName() << "\n";
  
  // 1. Create blocks
  for (BasicBlock &BB : F) {
    Blocks.push_back(std::make_unique<MyBasicBlock>(BB));
    BBMap[&BB] = Blocks.back().get();

    if (&BB == &F.getEntryBlock()) 
      EntryBlock = Blocks.back().get(); // Capture Entry
  }

  // 2. Create edges via terminators
  for (auto &MBB : Blocks) {
    MyBasicBlock *Src = MBB.get();
    Instruction *TI = Src->getTerminator();

    if (auto *BI = dyn_cast<BranchInst>(TI)) {
      for (unsigned i = 0; i < BI->getNumSuccessors(); ++i) {
        BasicBlock *SuccBB = BI->getSuccessor(i);
        Edges.emplace_back(Src, BBMap[SuccBB]);
      }
    } else if (auto *SI = dyn_cast<SwitchInst>(TI)) {
      Edges.emplace_back(Src, BBMap[SI->getDefaultDest()]);
      for (auto Case : SI->cases())
        Edges.emplace_back(Src, BBMap[Case.getCaseSuccessor()]);
    } else {
      // Handle other terminator types if needed
      errs() << "Unhandled terminator type in block " << Src->getName() 
             << ": " << TI->getOpcodeName() << "\n";
    }
  }

  // 3. Attach Successors and Predecessors
  for (const auto &E : Edges) {
    E.first->Successors.push_back(E.second);
    E.second->Predecessors.push_back(E.first);
  }

  // 4. Run Analysis
  calculateDominators();
}

void CFGraph::dfsPostOrder(MyBasicBlock* node, std::set<MyBasicBlock*>& visited, 
                           std::vector<MyBasicBlock*>& postOrder) {
  visited.insert(node);
  for (auto* succ : node->Successors) {
    if (visited.find(succ) == visited.end()) {
      dfsPostOrder(succ, visited, postOrder);
    }
  }
  postOrder.push_back(node);
}

void CFGraph::calculateDominators() {
  if (Blocks.empty()) 
    return;

  // A. Compute Reverse Post-Order (RPO)
  std::vector<MyBasicBlock*> rpo;
  std::set<MyBasicBlock*> visited;
  dfsPostOrder(EntryBlock, visited, rpo);
  std::reverse(rpo.begin(), rpo.end()); 

  // B. Initialize Sets
  // Dom(n_0) = {n_0}
  EntryBlock->Dominators.clear();
  EntryBlock->Dominators.insert(EntryBlock);

  // Dom(n) = {All Nodes} for n != n_0
  std::set<MyBasicBlock*> allNodes;
  for(const auto& b : Blocks) 
    allNodes.insert(b.get());

  for (auto& node : Blocks) {
    if (node.get() != EntryBlock) {
      node->Dominators = allNodes;
    }
  }

  // C. Iterative Solver
  bool changed = true;
  while (changed) {
    changed = false;
    
    // Iterate in Reverse Post Order
    for (MyBasicBlock* node : rpo) {
      if (node == EntryBlock) 
        continue;

      // Calculate Intersection of Predecessors
      // Dom(n) = {n} U (Intersect(Dom(p)) for all p in preds)
      std::set<MyBasicBlock*> newDom;
      bool firstPred = true;

      for (MyBasicBlock* pred : node->Predecessors) {
        // Skip predecessors that haven't been processed/reached yet 
        if (pred->Dominators.empty()) 
          continue; 

        if (firstPred) {
          newDom = pred->Dominators;
          firstPred = false;
        } else {
          // Intersection
          std::set<MyBasicBlock*> intersection;
          std::set_intersection(newDom.begin(), newDom.end(),
                                pred->Dominators.begin(), pred->Dominators.end(),
                                std::inserter(intersection, intersection.begin()));
          newDom = intersection;
        }
      }

      newDom.insert(node); // Union with {n}

      if (newDom != node->Dominators) {
        node->Dominators = newDom;
        changed = true;
      }
    }
  }

  // D. Calculate Immediate Dominators (IDom)
  for (auto& nodePtr : Blocks) {
    MyBasicBlock* node = nodePtr.get();
    if (node == EntryBlock) 
      continue;

    MyBasicBlock* closestDom = nullptr;
    size_t maxDomSize = 0;

    for (MyBasicBlock* dom : node->Dominators) {
      if (dom == node) 
        continue; // Strict dominators only

      // If dom has more dominators than current closest, it is "closer" to us
      if (dom->Dominators.size() > maxDomSize) {
        maxDomSize = dom->Dominators.size();
        closestDom = dom;
      }
    }
    node->IDom = closestDom;
  }
}

void CFGraph::emitDot(const std::string &FilePath) const {
  std::error_code EC;
  raw_fd_ostream File(FilePath, EC);

  if (EC) {
    errs() << "Error opening DOT file: " << EC.message() << "\n";
    return;
  }

  // Helper function to escape strings for DOT format
  auto escapeDot = [](const std::string &S) -> std::string {
    std::string Result;
    for (char C : S) {
      if (C == '\\') {
        Result += "\\\\";
      } else if (C == '"') {
        Result += "\\\"";
      } else if (C == '\n') {
        Result += "\\l";
      } else {
        Result += C;
      }
    }
    return Result;
  };

  File << "digraph CFG {\n";
  File << "  node [shape=box, fontname=\"monospace\", style=filled];\n";
  File << "  edge [penwidth=1.5];\n\n";

  constexpr unsigned HEAD = 2;
  constexpr unsigned TAIL = 2;
  constexpr unsigned MAX_INST_WIDTH = 60;  // Maximum characters per instruction line

  // Emit nodes
  for (const auto &Ptr : Blocks) {
    const MyBasicBlock *BB = Ptr.get();
    const auto &Insts = BB->getInstructions();
    unsigned InstCount = Insts.size();

    // Determine node color
    std::string nodeColor = "lightgray";
    std::string fontColor = "black";
    
    if (BB == EntryBlock) {
      nodeColor = "lightblue";
    } else if (auto *TI = BB->getTerminator()) {
      if (isa<BranchInst>(TI)) {
        auto *BI = cast<BranchInst>(TI);
        if (BI->isConditional()) {
          nodeColor = "lightyellow";  // Conditional branches
        }
      } else if (isa<SwitchInst>(TI)) {
        nodeColor = "lightcoral";  // Switch statements
      } else if (isa<ReturnInst>(TI)) {
        nodeColor = "lightgreen";  // Return blocks
      }
    }

    File << "  " << BB->getName() << " [label=\"";
    
    // Header
    File << BB->getName() << "\\l"
         << InstCount << " instructions\\l";

    auto printInst = [&](Instruction *I) {
      std::string S;
      raw_string_ostream OS(S);
      I->print(OS);
      std::string InstStr = OS.str();
      
      // Truncate if too long
      if (InstStr.length() > MAX_INST_WIDTH) {
        InstStr = InstStr.substr(0, MAX_INST_WIDTH) + "...";
      }
      
      File << escapeDot(InstStr) << "\\l";
    };

    if (InstCount <= HEAD + TAIL) {
      // Small block → print all instructions
      for (Instruction *I : Insts)
        printInst(I);
    } else {
      // Print first HEAD instructions
      for (unsigned i = 0; i < HEAD; ++i)
        printInst(Insts[i]);

      // Ellipsis
      File << "...\\l";

      // Print last TAIL instructions
      for (unsigned i = InstCount - TAIL; i < InstCount; ++i)
        printInst(Insts[i]);
    }

    File << "\", fillcolor=\"" << nodeColor 
         << "\", fontcolor=\"" << fontColor << "\"];\n";
  }

  File << "\n";

  // Emit edges with colors
  for (const auto &E : Edges) {
    MyBasicBlock *Src = E.first;
    MyBasicBlock *Dst = E.second;
    
    // Determine edge color based on branch type
    std::string edgeColor = "black";
    std::string edgeLabel = "";
    
    if (auto *TI = Src->getTerminator()) {
      if (auto *BI = dyn_cast<BranchInst>(TI)) {
        if (BI->isConditional()) {
          // Color conditional branches
          if (BI->getSuccessor(0) == Dst->BBRef) {
            edgeColor = "darkgreen";
            edgeLabel = "true";
          } else {
            edgeColor = "red";
            edgeLabel = "false";
          }
        } else {
          edgeColor = "blue";
        }
      } else if (isa<SwitchInst>(TI)) {
        edgeColor = "purple";
      }
    }
    
    File << "  " << Src->getName()
        << " -> " << Dst->getName()
        << " [color=\"" << edgeColor << "\"";
    
    if (!edgeLabel.empty()) {
      File << ", label=\"" << edgeLabel << "\"";
    }
    
    File << "];\n"; 
  }

  File << "}\n";
}

// ============================================================================
// DOMTree Implementation
// ============================================================================

DOMTree::DOMTree(const CFGraph &CFG) {
  // Phase 1: Initialize the map
  // We must add EVERY node as a key. If we only add parents, leaf nodes 
  // (nodes that dominate nothing) won't appear in the DOT graph output.
  for (const auto &BlockPtr : CFG.Blocks) {
    adjList[BlockPtr.get()] = {}; 
  }

  // Phase 2: Build the Edges
  for (const auto &BlockPtr : CFG.Blocks) {
    MyBasicBlock *Child = BlockPtr.get();
    MyBasicBlock *Parent = Child->getImmediateDominator();

    // If Parent exists, add this Child to the Parent's adjacency list
    if (Parent) {
      adjList[Parent].push_back(Child);
    }
  }
}

void DOMTree::emitDot(const std::string &FilePath) const {
  std::error_code EC;
  raw_fd_ostream File(FilePath, EC);

  if (EC) {
    errs() << "Error opening DOT file: " << EC.message() << "\n";
    return;
  }

  File << "digraph DOMTree {\n";
  File << "  rankdir=TB;\n";  // Top to bottom layout
  File << "  node [shape=box, style=\"rounded,filled\", fontname=\"Arial\", fontsize=12];\n";
  File << "  edge [color=\"#2E86AB\", penwidth=2, arrowsize=0.8];\n\n";

  // Find the root node (node with no parent in the tree)
  MyBasicBlock *Root = nullptr;
  for (const auto &Entry : adjList) {
    bool hasParent = false;
    for (const auto &ParentEntry : adjList) {
      for (MyBasicBlock *Child : ParentEntry.second) {
        if (Child == Entry.first) {
          hasParent = true;
          break;
        }
      }
      if (hasParent) break;
    }
    if (!hasParent && !Entry.first->getName().empty()) {
      Root = Entry.first;
      break;
    }
  }

  // Emit all nodes with styling
  for (const auto &Entry : adjList) {
    if (!Entry.first->getName().empty()) {
      MyBasicBlock *Node = Entry.first;
      std::string nodeColor;
      std::string fontColor = "white";
      std::string shape = "box";
      
      // Determine node type and color
      if (Node == Root) {
        // Root node (entry block)
        nodeColor = "#06A77D";  // Teal green
        shape = "box";
      } else if (Entry.second.empty()) {
        // Leaf nodes (nodes with no children)
        nodeColor = "#D62246";  // Red
      } else if (Entry.second.size() == 1) {
        // Single child
        nodeColor = "#4ECDC4";  // Light turquoise
      } else {
        // Multiple children (branch points in dom tree)
        nodeColor = "#F77F00";  // Orange
      }
      
      File << "  \"" << Node->getName() 
           << "\" [fillcolor=\"" << nodeColor 
           << "\", fontcolor=\"" << fontColor 
           << "\", shape=" << shape << "];\n";
    }
  }
  
  File << "\n";

  // Emit edges with gradient styling
  for (const auto &Entry : adjList) {
    MyBasicBlock *Parent = Entry.first;
    for (MyBasicBlock *Child : Entry.second) {
      if (!Parent->getName().empty() && !Child->getName().empty()) {
        File << "  \"" << Parent->getName() 
             << "\" -> \"" << Child->getName() << "\";\n";
      }
    }
  }

  File << "}\n";
}

// ============================================================================
// CFGDomAnalysisPass Implementation
// ============================================================================

PreservedAnalyses CFGDomAnalysisPass::run(
    Function &F,
    AnalysisManager<Function> &AM) {

  errs() << ">>> CFGDomAnalysisPass::run() called for function: " 
         << F.getName() << " <<<\n";
  
  CFGraph CFG(F);

  if (!CFG_DOT_FOLDER.empty()) {
    std::string FileName = (CFG_DOT_FOLDER + "/" + F.getName().str() + ".dot");
    errs() << "Emitting CFG DOT file for function '" << F.getName() 
           << "' at: " << FileName << "\n";
    CFG.emitDot(FileName);

    if(EMIT_DOM_TREE) {
      std::string DomFileName = (CFG_DOT_FOLDER + "/" + F.getName().str() + "_dom.dot");
      errs() << "Emitting DOM Tree DOT file for function '" << F.getName() 
             << "' at: " << DomFileName << "\n";
      DOMTree DomTree(CFG);
      DomTree.emitDot(DomFileName);
    }
  }

  
  return PreservedAnalyses::all();
}


