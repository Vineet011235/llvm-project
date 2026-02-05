#include "llvm/Analysis/CFGDomAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>
#include <vector>
#include <cstdlib>

using namespace llvm;

// Command line options
static cl::opt<std::string> RESULT_DIR(
    "result-dir",
    cl::desc("Directory to store all analysis results (CFG, DOM tree, verification report) [MUST EXIST]"),
    cl::value_desc("directory"),
    cl::init("")
);

static cl::opt<bool> SHOW_CFG(
    "show-cfg",
    cl::desc("Whether to emit the CFG DOT file"),
    cl::init(false)
);

static cl::opt<bool> SHOW_DOM_TREE(
    "show-dom-tree",
    cl::desc("Whether to emit the dominator tree DOT file"),
    cl::init(false)
);

static cl::opt<bool> SHOW_REPORT(
    "show-report",
    cl::desc("Whether to generate the dominator tree verification report"),
    cl::init(false)
);

static cl::opt<bool> SHOW_DOMINATORS(
    "show-dominators",
    cl::desc("Whether to generate a .txt file with pre and post dominators for each block"),
    cl::init(false)
);

// Track if we've initialized the verification report file
static bool VerificationReportInitialized = false;

// ============================================================================
// MyBasicBlock Implementation
// ============================================================================

MyBasicBlock::MyBasicBlock(BasicBlock &BB)
    : BBRef(&BB),
      Name("BB_" + std::to_string(getNextID())),
      TerminatorInst(BB.getTerminator()),
      IDom(nullptr),
      IPostDom(nullptr) {

  for (Instruction &I : BB)
    BlockInstructions.push_back(&I);
}

// Default constructor for virtual exit node
MyBasicBlock::MyBasicBlock()
    : BBRef(nullptr),
      Name("VirtualExit"),
      TerminatorInst(nullptr),
      IDom(nullptr),
      IPostDom(nullptr) {
  // Empty - virtual node has no instructions
}

// ============================================================================
// CFGraph Implementation
// ============================================================================

CFGraph::CFGraph(Function &F) : VirtualExit(nullptr), ParentFunction(&F) {  
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
    } else if(isa<ReturnInst>(TI)) {
      // No successors for return instructions [Handled implicitly by having no edges]
    } else {
      // Handle other terminator types if needed
      errs() << "Unhandled terminator type in block " << Src->getName() 
             << ": " << TI->getOpcodeName() << "\n";
    }
  }

  // 3. Identify reachable blocks from entry
  std::set<MyBasicBlock*> reachable;
  std::function<void(MyBasicBlock*, std::set<MyBasicBlock*>&)> markReachable;
  markReachable = [&](MyBasicBlock* node, std::set<MyBasicBlock*>& visited) {
    if (visited.find(node) != visited.end()) return;
    visited.insert(node);
    
    // Check successors via terminator
    Instruction *TI = node->getTerminator();
    if (auto *BI = dyn_cast<BranchInst>(TI)) {
      for (unsigned i = 0; i < BI->getNumSuccessors(); ++i) {
        BasicBlock *SuccBB = BI->getSuccessor(i);
        if (BBMap.find(SuccBB) != BBMap.end()) {
          markReachable(BBMap[SuccBB], visited);
        }
      }
    } else if (auto *SI = dyn_cast<SwitchInst>(TI)) {
      if (BBMap.find(SI->getDefaultDest()) != BBMap.end()) {
        markReachable(BBMap[SI->getDefaultDest()], visited);
      }
      for (auto Case : SI->cases()) {
        if (BBMap.find(Case.getCaseSuccessor()) != BBMap.end()) {
          markReachable(BBMap[Case.getCaseSuccessor()], visited);
        }
      }
    }
  };
  
  markReachable(EntryBlock, reachable);

  // 4. Attach Successors and Predecessors only for edges between reachable blocks
  for (const auto &E : Edges) {
    // Only add edge if both source and destination are reachable
    if (reachable.find(E.first) != reachable.end() && 
        reachable.find(E.second) != reachable.end()) {
      E.first->Successors.push_back(E.second);
      E.second->Predecessors.push_back(E.first);
    }
  }

  // 5. Run Analysis
  calculateDominators();
  
  // 6. Create Virtual Exit Node for Post-Dominator Analysis
  // Critical for handling multiple exit blocks correctly
  // Note: VirtualExit uses default constructor - doesn't correspond to a real BasicBlock
  VirtualExit = new MyBasicBlock();
  
  // Find all exit blocks (blocks with no successors or return/unreachable instructions)
  std::vector<MyBasicBlock*> exitBlocks;
  for (const auto &BlockPtr : Blocks) {
    MyBasicBlock *BB = BlockPtr.get();
    // Only consider reachable blocks
    if (reachable.find(BB) == reachable.end()) continue;
    
    // A block is an exit if:
    // 1. It has no successors, OR
    // 2. It has a return/unreachable terminator
    Instruction *TI = BB->getTerminator();
    if (BB->Successors.empty() || 
        isa<ReturnInst>(TI) || 
        isa<UnreachableInst>(TI)) {
      exitBlocks.push_back(BB);
    }
  }
  
  // If no exit blocks found, use all blocks with no successors
  if (exitBlocks.empty()) {
    for (const auto &BlockPtr : Blocks) {
      MyBasicBlock *BB = BlockPtr.get();
      if (reachable.find(BB) != reachable.end() && BB->Successors.empty()) {
        exitBlocks.push_back(BB);
      }
    }
  }
  
  // Connect all exit blocks to virtual exit
  for (MyBasicBlock *exitBB : exitBlocks) {
    exitBB->Successors.push_back(VirtualExit);
    VirtualExit->Predecessors.push_back(exitBB);
  }
  
  // 7. Calculate Post-Dominators
  calculatePostDominators();
}

// Destructor to clean up virtual exit node
CFGraph::~CFGraph() {
  if (VirtualExit) {
    delete VirtualExit;
    VirtualExit = nullptr;
  }
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

  // Dom(n) = {All Reachable Nodes} for reachable n != n_0
  // Unreachable nodes get empty dominator sets
  std::set<MyBasicBlock*> allReachableNodes;
  for(const auto& node : rpo) 
    allReachableNodes.insert(node);

  for (auto& node : Blocks) {
    if (node.get() != EntryBlock) {
      // Only initialize dominators for reachable nodes
      if (visited.find(node.get()) != visited.end()) {
        node->Dominators = allReachableNodes;
      } else {
        // Unreachable nodes have empty dominator sets
        node->Dominators.clear();
      }
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

    // Skip unreachable nodes - they have no immediate dominator
    if (node->Dominators.empty()) {
      node->IDom = nullptr;
      continue;
    }

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

// Helper for reverse DFS (traversing backwards from exit using predecessors)
void CFGraph::dfsPostOrderReverse(MyBasicBlock* node, std::set<MyBasicBlock*>& visited,
                                   std::vector<MyBasicBlock*>& postOrder) {
  visited.insert(node);
  for (auto* pred : node->Predecessors) {
    if (visited.find(pred) == visited.end()) {
      dfsPostOrderReverse(pred, visited, postOrder);
    }
  }
  postOrder.push_back(node);
}

void CFGraph::calculatePostDominators() {
  if (!VirtualExit) 
    return;

  // A. Compute reachable nodes from VirtualExit (backwards)
  // This identifies which nodes can reach an exit
  std::vector<MyBasicBlock*> postOrder;
  std::set<MyBasicBlock*> reachableFromExit;
  dfsPostOrderReverse(VirtualExit, reachableFromExit, postOrder);
  
  // Reverse to get "forward" order in reversed CFG
  std::reverse(postOrder.begin(), postOrder.end());

  // B. Initialize Sets
  // PostDom(exit) = {exit}
  VirtualExit->PostDominators.clear();
  VirtualExit->PostDominators.insert(VirtualExit);

  // PostDom(n) = {All Exit-Reachable Nodes} for n != exit
  // Nodes not reachable from exit get empty post-dominator sets
  std::set<MyBasicBlock*> allExitReachableNodes = reachableFromExit;

  // Initialize regular blocks
  for (auto& node : Blocks) {
    if (reachableFromExit.find(node.get()) != reachableFromExit.end()) {
      // Exit-reachable nodes initialized to all exit-reachable nodes
      node->PostDominators = allExitReachableNodes;
    } else {
      // Unreachable-from-exit nodes (e.g., infinite loops) get themselves only
      node->PostDominators.clear();
      node->PostDominators.insert(node.get());
    }
  }

  // C. Iterative Solver - CRITICAL: Use Successors, not Predecessors!
  bool changed = true;
  int iterations = 0;
  const int MAX_ITERATIONS = 1000; // Safety limit
  
  while (changed && iterations < MAX_ITERATIONS) {
    changed = false;
    iterations++;
    
    // Iterate in our computed order
    for (MyBasicBlock* node : postOrder) {
      if (node == VirtualExit) 
        continue;

      // Skip nodes not reachable from exit
      if (reachableFromExit.find(node) == reachableFromExit.end()) {
        continue;
      }

      // Calculate Intersection of Successors
      // PostDom(n) = {n} U (Intersect(PostDom(s)) for all s in successors)
      std::set<MyBasicBlock*> newPostDom;
      bool firstSucc = true;

      for (MyBasicBlock* succ : node->Successors) {
        // Skip successors that haven't been processed yet or aren't reachable
        if (succ->PostDominators.empty()) 
          continue;

        if (firstSucc) {
          newPostDom = succ->PostDominators;
          firstSucc = false;
        } else {
          // Intersection
          std::set<MyBasicBlock*> intersection;
          std::set_intersection(newPostDom.begin(), newPostDom.end(),
                                succ->PostDominators.begin(), succ->PostDominators.end(),
                                std::inserter(intersection, intersection.begin()));
          newPostDom = intersection;
        }
      }

      // Handle nodes with no successors or where no successor has post-doms yet
      if (firstSucc && !node->Successors.empty()) {
        // Successors exist but none have valid post-dominators yet
        continue;
      }

      newPostDom.insert(node); // Union with {n}

      if (newPostDom != node->PostDominators) {
        node->PostDominators = newPostDom;
        changed = true;
      }
    }
  }

  // D. Calculate Immediate Post-Dominators (IPostDom)
  for (auto& nodePtr : Blocks) {
    MyBasicBlock* node = nodePtr.get();

    // Skip nodes not reachable from exit
    if (reachableFromExit.find(node) == reachableFromExit.end()) {
      node->IPostDom = nullptr;
      continue;
    }

    // Find immediate post-dominator
    MyBasicBlock* closestPostDom = nullptr;
    size_t maxPostDomSize = 0;

    for (MyBasicBlock* pdom : node->PostDominators) {
      if (pdom == node) 
        continue; // Strict post-dominators only

      // If pdom has more post-dominators, it is "closer" to node
      if (pdom->PostDominators.size() > maxPostDomSize) {
        maxPostDomSize = pdom->PostDominators.size();
        closestPostDom = pdom;
      }
    }
    node->IPostDom = closestPostDom;
  }
  
  // Special handling for VirtualExit - it has no post-dominator
  VirtualExit->IPostDom = nullptr;
}

void CFGraph::writeDominatorsToFile(const std::string &FilePath) const {
  std::error_code EC;
  raw_fd_ostream File(FilePath, EC);

  if (EC) {
    errs() << "Error opening dominators file: " << EC.message() << "\n";
    return;
  }

  File << "====================================================================\n";
  File << "Dominator and Post-Dominator Analysis\n";
  File << "====================================================================\n";
  File << "Function: " << ParentFunction->getName() << "\n";
  File << "Total Blocks: " << Blocks.size() << "\n";
  File << "Entry Block: " << (EntryBlock ? EntryBlock->getName() : "None") << "\n";
  File << "Virtual Exit: " << (VirtualExit ? VirtualExit->getName() : "None") << "\n";
  File << "====================================================================\n\n";

  // Iterate through all blocks in a stable order
  for (const auto &BlockPtr : Blocks) {
    MyBasicBlock *BB = BlockPtr.get();
    
    File << "--------------------------------------------------------------------\n";
    File << "Block: " << BB->getName() << "\n";
    File << "--------------------------------------------------------------------\n";
    
    // Show if this is the entry block
    if (BB == EntryBlock) {
      File << "[ENTRY BLOCK]\n";
    }
    
    // Show terminator type
    if (Instruction *TI = BB->getTerminator()) {
      File << "Terminator: " << TI->getOpcodeName() << "\n";
    } else {
      File << "Terminator: None\n";
    }
    
    File << "\n";
    
    // Predecessors
    File << "Predecessors (" << BB->Predecessors.size() << "):\n";
    if (BB->Predecessors.empty()) {
      File << "  (none)\n";
    } else {
      for (MyBasicBlock *Pred : BB->Predecessors) {
        File << "  - " << Pred->getName() << "\n";
      }
    }
    File << "\n";
    
    // Successors
    File << "Successors (" << BB->Successors.size() << "):\n";
    if (BB->Successors.empty()) {
      File << "  (none)\n";
    } else {
      for (MyBasicBlock *Succ : BB->Successors) {
        File << "  - " << Succ->getName() << "\n";
      }
    }
    File << "\n";
    
    // DOMINATORS (Pre-Dominators)
    File << "DOMINATORS (Pre-Dominators) - Set (" << BB->Dominators.size() << "):\n";
    if (BB->Dominators.empty()) {
      File << "  (none - unreachable from entry)\n";
    } else {
      for (MyBasicBlock *Dom : BB->Dominators) {
        File << "  - " << Dom->getName();
        if (Dom == BB) {
          File << " [self]";
        }
        File << "\n";
      }
    }
    File << "\n";
    
    // Immediate Dominator
    File << "Immediate Dominator (IDom):\n";
    if (BB->IDom) {
      File << "  " << BB->IDom->getName() << "\n";
    } else if (BB == EntryBlock) {
      File << "  (none - this is entry block)\n";
    } else if (BB->Dominators.empty()) {
      File << "  (none - unreachable block)\n";
    } else {
      File << "  (none)\n";
    }
    File << "\n";
    
    // POST-DOMINATORS
    File << "POST-DOMINATORS - Set (" << BB->PostDominators.size() << "):\n";
    if (BB->PostDominators.empty()) {
      File << "  (none - unreachable from exit)\n";
    } else if (BB->PostDominators.size() == 1 && BB->PostDominators.count(BB)) {
      File << "  - " << BB->getName() << " [self only - possibly in infinite loop]\n";
    } else {
      for (MyBasicBlock *PostDom : BB->PostDominators) {
        File << "  - " << PostDom->getName();
        if (PostDom == BB) {
          File << " [self]";
        }
        if (PostDom == VirtualExit) {
          File << " [virtual-exit]";
        }
        File << "\n";
      }
    }
    File << "\n";
    
    // Immediate Post-Dominator
    File << "Immediate Post-Dominator (IPostDom):\n";
    if (BB->IPostDom) {
      File << "  " << BB->IPostDom->getName();
      if (BB->IPostDom == VirtualExit) {
        File << " [virtual-exit]";
      }
      File << "\n";
    } else if (BB->PostDominators.empty()) {
      File << "  (none - unreachable from exit)\n";
    } else if (BB->PostDominators.size() == 1 && BB->PostDominators.count(BB)) {
      File << "  (none - node only post-dominates itself)\n";
    } else {
      File << "  (none)\n";
    }
    File << "\n";
  }
  
  // Add analysis for Virtual Exit if it exists
  if (VirtualExit) {
    File << "--------------------------------------------------------------------\n";
    File << "Block: " << VirtualExit->getName() << " [SYNTHETIC]\n";
    File << "--------------------------------------------------------------------\n";
    File << "[VIRTUAL EXIT NODE - Used for Post-Dominator Analysis]\n";
    File << "Terminator: None (synthetic)\n\n";
    
    File << "Predecessors (" << VirtualExit->Predecessors.size() << "):\n";
    if (VirtualExit->Predecessors.empty()) {
      File << "  (none)\n";
    } else {
      File << "  [These are the actual exit blocks of the function]\n";
      for (MyBasicBlock *Pred : VirtualExit->Predecessors) {
        File << "  - " << Pred->getName() << "\n";
      }
    }
    File << "\n";
    
    File << "Successors: (none - this is the exit)\n\n";
    
    File << "POST-DOMINATORS - Set (" << VirtualExit->PostDominators.size() << "):\n";
    for (MyBasicBlock *PostDom : VirtualExit->PostDominators) {
      File << "  - " << PostDom->getName() << " [self]\n";
    }
    File << "\n";
    
    File << "Immediate Post-Dominator: (none - this is the exit node)\n\n";
  }
  
  File << "====================================================================\n";
  File << "End of Dominator Analysis\n";
  File << "====================================================================\n";
  
  File.flush();
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

DOMTree::DOMTree(const CFGraph &CFG) : CFG(CFG) {
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

  // Find the root node (The entry block that dominates itself)
  MyBasicBlock *Root = CFG.EntryBlock;
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
    MyBasicBlock *Node = Entry.first;
    if (!Node || Node->getName().empty()) {
      errs() << "Warning: Skipping null or unnamed node in DOMTree\n";
      continue;
    }
    
    const std::string &nodeName = Node->getName();
    
    // Validate that the name only contains printable ASCII characters
    bool validName = true;
    for (char c : nodeName) {
      if (!std::isprint(static_cast<unsigned char>(c))) {
        errs() << "Warning: Node has non-printable character in name, skipping\n";
        validName = false;
        break;
      }
    }
    if (!validName) continue;
    
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
    
    File << "  \"" << nodeName 
         << "\" [fillcolor=\"" << nodeColor 
         << "\", fontcolor=\"" << fontColor 
         << "\", shape=" << shape << "];\n";
  }
  
  File << "\n";

  // Emit edges with gradient styling
  for (const auto &Entry : adjList) {
    MyBasicBlock *Parent = Entry.first;
    if (!Parent || Parent->getName().empty()) continue;
    
    const std::string &parentName = Parent->getName();
    
    // Validate parent name
    bool validParentName = true;
    for (char c : parentName) {
      if (!std::isprint(static_cast<unsigned char>(c))) {
        validParentName = false;
        break;
      }
    }
    if (!validParentName) continue;
    
    for (MyBasicBlock *Child : Entry.second) {
      if (!Child || Child->getName().empty()) continue;
      
      const std::string &childName = Child->getName();
      
      // Validate child name
      bool validChildName = true;
      for (char c : childName) {
        if (!std::isprint(static_cast<unsigned char>(c))) {
          validChildName = false;
          break;
        }
      }
      if (!validChildName) continue;
      
      File << "  \"" << parentName 
           << "\" -> \"" << childName << "\";\n";
    }
  }

  File << "}\n";
}

bool DOMTree::verifyWithLLVM(const std::string &OutputDir) const {
  // Get the function from the first block's BasicBlock reference
  if (CFG.Blocks.empty()) {
    errs() << "[VERIFY] No blocks in CFG\n";
    return true;
  }

  Function *F = CFG.Blocks[0]->getBasicBlock()->getParent();
  if (!F) {
    errs() << "[VERIFY] ERROR: Cannot get Function from BasicBlock\n";
    return false;
  }

  // Create README file in the parent directory of OutputDir
  std::string ReadmePath = OutputDir;
  if (ReadmePath.back() == '/') {
    ReadmePath.pop_back();
  }
  // Go up one directory if we're in the 'dot' subdirectory
  size_t lastSlash = ReadmePath.find_last_of('/');
  if (lastSlash != std::string::npos && 
      ReadmePath.substr(lastSlash + 1) == "dot") {
    ReadmePath = ReadmePath.substr(0, lastSlash);
  }
  ReadmePath += "/VERIFICATION_REPORT.md";

  // On first verification call, clear/create the file with header
  if (!VerificationReportInitialized) {
    std::error_code EC;
    raw_fd_ostream InitFile(ReadmePath, EC);
    if (!EC) {
      InitFile << "# Dominator Tree Verification Report\n\n";
      InitFile << "*Verification results for all analyzed functions*\n\n";
      InitFile << "---\n\n";
      InitFile.flush();
      VerificationReportInitialized = true;
    }
  }
  
  std::error_code EC;
  // Open in append mode to accumulate results from multiple functions
  raw_fd_ostream File(ReadmePath, EC, sys::fs::OF_Append);

  if (EC) {
    errs() << "Error opening README file: " << EC.message() << "\n";
    return false;
  }

  // Compute LLVM's dominator tree
  DominatorTree LLVMDT(*F);
  
  bool allCorrect = true;
  unsigned numBlocks = 0;
  unsigned numMatches = 0;
  unsigned numMismatches = 0;
  std::vector<std::string> mismatchDetails;

  // Verify each block and collect results
  for (const auto &BlockPtr : CFG.Blocks) {
    MyBasicBlock *MyBB = BlockPtr.get();
    BasicBlock *BB = MyBB->getBasicBlock();
    numBlocks++;

    // Get immediate dominator from our implementation
    MyBasicBlock *MyIDom = MyBB->getImmediateDominator();
    
    // Get immediate dominator from LLVM
    DomTreeNode *LLVMNode = LLVMDT.getNode(BB);
    BasicBlock *LLVMIDomBB = nullptr;
    
    if (LLVMNode && LLVMNode->getIDom()) {
      LLVMIDomBB = LLVMNode->getIDom()->getBlock();
    }

    // Convert LLVM's immediate dominator BasicBlock to MyBasicBlock
    MyBasicBlock *LLVMIDom = nullptr;
    if (LLVMIDomBB) {
      auto it = CFG.BBMap.find(LLVMIDomBB);
      if (it != CFG.BBMap.end()) {
        LLVMIDom = it->second;
      }
    }

    // Compare results
    bool match = (MyIDom == LLVMIDom);
    
    if (match) {
      numMatches++;
    } else {
      numMismatches++;
      allCorrect = false;
      
      std::string detail = "**" + MyBB->getName() + "**\n";
      detail += "  - Custom IDom: ";
      if (MyIDom) {
        detail += MyIDom->getName();
      } else {
        detail += "(none)";
      }
      detail += "\n  - LLVM IDom: ";
      if (LLVMIDom) {
        detail += LLVMIDom->getName();
      } else {
        detail += "(none)";
      }
      mismatchDetails.push_back(detail);
    }
  }

  // Write README in Markdown format
  File << "## Function: `" << F->getName() << "`\n\n";
  
  File << "### Function Information\n\n";
  File << "- **Total Basic Blocks**: " << numBlocks << "\n\n";
  
  File << "### Verification Results\n\n";
  
  if (allCorrect) {
    File << "**Status**: ✅ PASSED\n\n";
    File << "All immediate dominators computed by the custom implementation ";
    File << "match LLVM's DominatorTree analysis.\n\n";
  } else {
    File << "**Status**: ❌ FAILED\n\n";
    File << "Found discrepancies between custom implementation and LLVM's DominatorTree.\n\n";
  }
  
  File << "### Summary Statistics\n\n";
  File << "| Metric | Count |\n";
  File << "|--------|-------|\n";
  File << "| Total Blocks | " << numBlocks << " |\n";
  File << "| Matches | " << numMatches << " |\n";
  File << "| Mismatches | " << numMismatches << " |\n";
  File << "| Success Rate | " << (numBlocks > 0 ? (numMatches * 100 / numBlocks) : 0) << "% |\n\n";
  
  if (!mismatchDetails.empty()) {
    File << "### Detailed Mismatches\n\n";
    for (const auto &detail : mismatchDetails) {
      File << detail << "\n\n";
    }
  }
  
  File << "---\n\n";

  File.flush();
  
  // Print minimal terminal output - just pass/fail status
  if (allCorrect) {
    errs() << "✓ Function '" << F->getName() << "': PASSED (" << numBlocks << " blocks verified)\n";
  } else {
    errs() << "✗ Function '" << F->getName() << "': FAILED (" << numMismatches << "/" << numBlocks 
           << " mismatches - see " << ReadmePath << ")\n";
  }
  
  return allCorrect;
}

// ============================================================================
// CFGDomAnalysisPass Implementation
// ============================================================================

PreservedAnalyses CFGDomAnalysisPass::run(
    Function &F,
    AnalysisManager<Function> &AM) {

  // Always construct CFG and DOMTree regardless of flags
  CFGraph CFG(F);
  DOMTree DomTree(CFG);

  // Check if result directory is specified
  if (RESULT_DIR.empty()) {
    errs() << "No output directory specified. Use -result-dir=<dir> to enable output generation\n";
    return PreservedAnalyses::all();
  }

  // Emit CFG DOT file if requested
  if (SHOW_CFG) {
    std::string FileName = (RESULT_DIR + "/" + F.getName().str() + ".dot");
    CFG.emitDot(FileName);
  }

  // Emit DOM tree DOT file if requested
  if (SHOW_DOM_TREE) {
    std::string DomFileName = (RESULT_DIR + "/" + F.getName().str() + "_dom.dot");
    DomTree.emitDot(DomFileName);
  }

  // Generate verification report if requested
  if (SHOW_REPORT) {
    bool result = DomTree.verifyWithLLVM(RESULT_DIR);
    if (!result) {
      errs() << "Dominator Tree verification failed for function: " << F.getName() << "\n";
    }
  }

  // Generate dominators text file if requested
  if (SHOW_DOMINATORS) {
    std::string DomFileName = (RESULT_DIR + "/" + F.getName().str() + "_dominators.txt");
    CFG.writeDominatorsToFile(DomFileName);
    errs() << "Dominator analysis written to: " << DomFileName << "\n";
  }

  return PreservedAnalyses::all();
}


