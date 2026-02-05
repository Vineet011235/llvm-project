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

// Track if we've initialized the verification report file
static bool VerificationReportInitialized = false;

// ============================================================================
// MyBasicBlock Implementation
// ============================================================================

MyBasicBlock::MyBasicBlock(BasicBlock &BB)
    : BBRef(&BB),
      Name("BB_" + std::to_string(getNextID())),
      TerminatorInst(BB.getTerminator()),
      IDom(nullptr) {

  for (Instruction &I : BB)
    BlockInstructions.push_back(&I);
}

// ============================================================================
// CFGraph Implementation
// ============================================================================

CFGraph::CFGraph(Function &F) : ParentFunction(&F) {  
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

  return PreservedAnalyses::all();
}


