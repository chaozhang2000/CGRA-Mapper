/**
 * @file DFG.cpp
 * @author Cheng Tan 
 * @brief  the implementation of DFG class
 * @version 0.1
 */
#include <fstream>
#include "DFG.h"

/**
 * How this function is implemented:
 * 1. init some var
 * 2. use construct() function to construct DFG
 * 3. init exec latency and pipelined opt (not used now)
 */
DFG::DFG(Function& t_F, list<Loop*>* t_loops, bool t_targetFunction,
         bool t_precisionAware, 
         map<string, int>* t_execLatency, list<string>* t_pipelinedOpt) {
  m_num = 0;
  m_targetFunction = t_targetFunction;
  m_targetLoops = t_loops;
  m_orderedNodes = NULL;
  m_CDFGFused = false;
  m_cycleNodeLists = new list<list<DFGNode*>*>();
  m_precisionAware = t_precisionAware;

  construct(t_F);
  initExecLatency(t_execLatency);
  initPipelinedOpt(t_pipelinedOpt);

}


//属于目标Function或目标循环的个数为0或某个目标循环中含有该指令

/** Used to determine whether the instruction should be ignored,An ignored instruction will not be reflected in the DFG
 * @param t_inst instruction that need to be judged
 * @return true if the inst should be ignored
 * 
 * what is in this function
 * 1. if the m_targetFunction param is set, all inst in function will not be ignore,this function always return flase
 * 2. else all inst is in target loops will not be ignore and other insts in function will be ignored. Not all inst in the function belong to target loop.For example, instructions for entering and exiting functions
 */
bool DFG::shouldIgnore(Instruction* t_inst) {
  if (m_targetFunction) {
    return false;
  }
  if (m_targetLoops->size() == 0)
    return false;
  for (Loop* current_loop: *m_targetLoops) {
    if (current_loop->contains(t_inst)) {
      return false;
    }
  }
  return true;
}

 /** 
	* what is in his function
	* 1. Clear the data structions which is used to save nodes and edges
	* 2. Traverse all basic block in target function 
	* 3. Traverse all instructions of current basic block and create a dfgNode for each instruction
	* 4. Traverse all DFGNodes and all operands of every instruction,create DFGEdge according to their operands 
	* 5. connect all DFGNode to generate the DFG
	* 6. reorder the DFGNodes according the longest path in DFG
 */
void DFG::construct(Function& t_F) {

	//1. Clear the data structions which is used to save nodes and edges
  m_DFGEdges.clear();
  nodes.clear();
  m_ctrlEdges.clear();

  int nodeID = 0;
  int ctrlEdgeID = 0;
  int dfgEdgeID = 0;

  cout<<"*** constructing DFG of current function: "<<t_F.getName().str()<<"\n";

  // FIXME: eleminate duplicated edges.
	

	//2. Traverse all basic block in target function 
  for (Function::iterator BB=t_F.begin(), BEnd=t_F.end();
      BB!=BEnd; ++BB) {
    BasicBlock *curBB = &*BB;
		errs()<<"----------------------------------\n";
    errs()<<"*** current basic block beginning: "<<*curBB->begin()<<"\n";
		//print all successor basic block of current basic block
    for (BasicBlock* sucBB : successors(curBB)) {
      errs()<<"   ****** successor basic block beginning: "<<*sucBB->begin()<<"\n";
    }

     // Construct DFG nodes.
		 // !!!!!!!!!!!!!!!!!!!!!!330-350 is important
    for (BasicBlock::iterator II=curBB->begin(),
        IEnd=curBB->end(); II!=IEnd; ++II) {
      Instruction* curII = &*II;

      // Ignore this IR if it is out of the scope.
      if (shouldIgnore(curII)) {
        errs()<<*curII<<" *** ignored by pass due to that the BB is out "<<
            "of the scope (target loop)\n";
        continue;
      }
      errs()<<*curII;
      DFGNode* dfgNode;
      if (hasNode(curII)) {
        dfgNode = getNode(curII);
      } else {
        dfgNode = new DFGNode(nodeID++, m_precisionAware, curII, getValueName(curII));
        nodes.push_back(dfgNode);
      }
      errs()<<" (dfgNode ID: "<<dfgNode->getID()<<")\n";
    }
		// !!!!!!!352-396 和控制流的产生有关，目前不关心，不重要。
		//getTerminator()方法是BasicBlock类的一个成员函数，返回指向该基本块终结指令的指针，
		//基本块是LLVM中表示程序控制流的最小单元，是一系列指令的序列，只有一个入口和出口，出口是终结指令，是一种控制流指令，如分支跳转返回等，决定了程序跳转到哪个基本块
    Instruction* terminator = curBB->getTerminator();

		//如果terminator可以忽略就不需要考虑下一个运行的基本块了,successors()可以获取后继的基本块的指针
    if (shouldIgnore(terminator))
      continue;
		//遍历所有的后继基本块，将后继基本块的不可忽视的入口指令也添加到节点，并添加控制箭头，即从当前基本块的出口指令指向到后继基本块入口指令的箭头
    for (BasicBlock* sucBB : successors(curBB)) {
      // TODO: get the live-in nodes rather than front() and connect them
			// 遍历每个后继基本块中的所有指令
      for (BasicBlock::iterator II=sucBB->begin(),
          IEnd=sucBB->end(); II!=IEnd; ++II) {
        Instruction* inst = &*II;
        // Ignore this IR if it is out of the scope.
        if (shouldIgnore(inst))
          continue;
				//只处理后继模块的入口指令，为后继基本块的入口指令创建DFGNode节点
        if (isLiveInInst(sucBB, inst)) {
          errs()<<" check inst: "<<*inst;//<<"\n";

          DFGNode* dfgNode;
          if (hasNode(inst)) {
            dfgNode = getNode(inst);
          } else {
            dfgNode = new DFGNode(nodeID++, m_precisionAware, inst, getValueName(inst));
            nodes.push_back(dfgNode);
          }
      errs()<<" (dfgNode ID: "<<dfgNode->getID()<<")\n";
          errs()<<"!!!!!!! construct ctrl flow: "<<*terminator<<"->"<<*inst<<"\n";
          // Construct contrl flow edges.
          DFGEdge* ctrlEdge;
          if (hasCtrlEdge(getNode(terminator), dfgNode)) {
            ctrlEdge = getCtrlEdge(getNode(terminator), dfgNode);
          }
          else {
            ctrlEdge = new DFGEdge(ctrlEdgeID++, getNode(terminator), dfgNode, true);
            m_ctrlEdges.push_back(ctrlEdge);
          }

        }
      }
    }
  }

 

  // Construct data flow edges.
	// second for loop
  for (DFGNode* node: nodes) {
//    nodes.push_back(Node(curII, getValueName(curII)));
    Instruction* curII = node->getInst();
    assert(node == getNode(curII));
    switch (curII->getOpcode()) {
      // The load/store instruction is special
      case llvm::Instruction::Load: {
        LoadInst* linst = dyn_cast<LoadInst>(curII);
        Value* loadValPtr = linst->getPointerOperand();

        // Parameter of the loop or the basic block, invisible in DFG.
        if (!hasNode(loadValPtr))
          break;
        DFGEdge* dfgEdge;
        if (hasDFGEdge(getNode(loadValPtr), node))
          dfgEdge = getDFGEdge(getNode(loadValPtr), node);
        else {
          dfgEdge = new DFGEdge(dfgEdgeID++, getNode(loadValPtr), node);
          m_DFGEdges.push_back(dfgEdge);
        }
        break;
      }
      case llvm::Instruction::Store: {
        StoreInst* sinst = dyn_cast<StoreInst>(curII);
        Value* storeValPtr = sinst->getPointerOperand();
        Value* storeVal = sinst->getValueOperand();
        DFGEdge* dfgEdge1;
        DFGEdge* dfgEdge2;

        // TODO: need to figure out storeVal and storeValPtr
        if (hasNode(storeVal)) {
          if (hasDFGEdge(getNode(storeVal), node))
            dfgEdge1 = getDFGEdge(getNode(storeVal), node);
          else {
            dfgEdge1 = new DFGEdge(dfgEdgeID++, getNode(storeVal), node);
            m_DFGEdges.push_back(dfgEdge1);
          }
        }
        if (hasNode(storeValPtr)) {
          if (hasDFGEdge(getNode(storeValPtr), node))
            dfgEdge2 = getDFGEdge(getNode(storeValPtr), node);
          else {
            dfgEdge2 = new DFGEdge(dfgEdgeID++, getNode(storeValPtr), node);
            m_DFGEdges.push_back(dfgEdge2);
          }
        }
        break;
      }
      default: {
        for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
          Instruction* tempInst = dyn_cast<Instruction>(*op);
						//errs()<<*curII<<"\n";		
          if (tempInst and !shouldIgnore(tempInst)) {
						//errs()<<*tempInst<<"\n";		
            DFGEdge* dfgEdge;
            if (hasNode(tempInst)) {
              if (hasDFGEdge(getNode(tempInst), node))
                dfgEdge = getDFGEdge(getNode(tempInst), node);
              else {
                dfgEdge = new DFGEdge(dfgEdgeID++, getNode(tempInst), node);
                m_DFGEdges.push_back(dfgEdge);
              }
            }
          } else {
            // Original Branch node will take three
            // predecessors (i.e., condi, true, false).
            if(!node->isBranch())
              node->addConst();
          } 
        }
        break;
      }
    }
  }
  connectDFGNodes();

  calculateCycles();

  reorderInLongest();
  
}

bool DFG::isMinimumAndHasNotBeenVisited(set<DFGNode*>* t_visited, map<DFGNode*, int>* t_map, DFGNode* t_node) {
	//first，if this node has been set level it should be ignore
  if (t_visited->find(t_node) != t_visited->end()) {
    return false;
  }
	//second, if this node hasn't been set level,we judge if it can be set level now
	//we traverse every node, if we can find a node which hasn't be visited and  has fewer prenodes than the DFGNode we focus on,we should ifgnore our target node. because it is not the node with the min prenode now.
  for (DFGNode* e_node: nodes) {
    if (e_node != t_node and t_visited->find(e_node) == t_visited->end() and (*t_map)[e_node] < (*t_map)[t_node]) {
      return false;
    }
  }
  return true;
}

/**
 * what is in this function:
 * 1. use reorderDFS() to get the longest Path in DFG
 * 2. give every DFGNode a level, first handle the DFGNodes in longestPath 
 * 3. chose the node hasn't been handled and with the fewest previous nodes and give it a level.until everynode is handled
 * 4. clear the list nodes,and push DFGNodes in order from small to large
 */
void DFG::reorderInLongest() {
  list<DFGNode*>* longestPath = new list<DFGNode*>();
  list<DFGNode*>* currentPath = new list<DFGNode*>();
  set<DFGNode*>* visited = new set<DFGNode*>();
  map<DFGNode*, int> indegree;//record every DFGNode have how many preNodes that have not been given a level
  for (DFGNode* node: nodes) {
    indegree[node] = node->getPredNodes()->size();
    currentPath->clear();
    visited->clear();
		//get the longestPath
    reorderDFS(visited, longestPath, currentPath, node);
  }

  visited->clear();
  int level = 0;
  for (DFGNode* node: *longestPath) {
		//errs()<<node->getID() <<*(node->getInst())<<"\n";
    node->setLevel(level);
    visited->insert(node);
    //cout<<"check longest path node: "<<node->getID()<<endl;
    for (DFGNode* succNode: *(node->getSuccNodes())) {
      indegree[succNode] -= 1;
    }
    level += 1;
  }
  int maxLevel = level;

  while (visited->size() < nodes.size()) {
    for (DFGNode* node: nodes) {
      // if (visited->find(node) == visited->end() and indegree[node] <= 0) {
      if (isMinimumAndHasNotBeenVisited(visited, &indegree, node)) {
        level = 0;
        for (DFGNode* preNode: *(node->getPredNodes())) {
          if (level < preNode->getLevel() + 1) {
            level = preNode->getLevel() + 1;
          }
        }
        node->setLevel(level);
        visited->insert(node);
        for (DFGNode* succNode: *(node->getSuccNodes())) {
          indegree[succNode] -= 1;
        }
      }
    }
  }

  list<DFGNode*> tempNodes;
  for (int l=0; l<maxLevel+1; ++l) {
    for (DFGNode* node: nodes) {
      if (node->getLevel() == l) {
        tempNodes.push_back(node);
      }
    }
  }

  nodes.clear();
  cout<<"[reorder DFG along with the longest path]\n";
  for (DFGNode* node: tempNodes) {
    nodes.push_back(node);
    errs()<<"("<<node->getID()<<") "<<*(node->getInst())<<", level: "<<node->getLevel()<<"\n";
  }
  delete longestPath;
  delete currentPath;
  delete visited;

}

//DFS(Depth-First Search) 深度优先搜索算法是一种用于遍历或搜索图的非线性数据结构的算法。它从起始顶点开始，沿着一条路径尽可能深入图中之前的每个未访问的顶点，直到达到最深的顶点为止。然后，回溯到上一个节点，继续探索其他分支直到所有节点都被访问到。

void DFG::reorderDFS(set<DFGNode*>* t_visited, list<DFGNode*>* t_targetPath,
                     list<DFGNode*>* t_curPath, DFGNode* targetDFGNode) {

  t_visited->insert(targetDFGNode);
  t_curPath->push_back(targetDFGNode);

  // Update target longest path if current one is longer.
  if (t_curPath->size() > t_targetPath->size()) {
    t_targetPath->clear();
    for (DFGNode* node: *t_curPath) {
      t_targetPath->push_back(node);
    }
  }

  for (DFGNode* succNode: *(targetDFGNode->getSuccNodes())) {
    if (t_visited->find(succNode) == t_visited->end()) { // not visited yet
      reorderDFS(t_visited, t_targetPath, t_curPath, succNode);
      t_visited->erase(succNode);
      t_curPath->pop_back();
    }
  }

}


void DFG::initExecLatency(map<string, int>* t_execLatency) {
  set<string> targetOpt;
  for (map<string, int>::iterator iter=t_execLatency->begin();
      iter!=t_execLatency->end(); ++iter) {
    targetOpt.insert(iter->first);
  }
  for (DFGNode* node: nodes) {
    if (t_execLatency->find(node->getOpcodeName()) != t_execLatency->end()) {
      string opcodeName = node->getOpcodeName();
      node->setExecLatency((*t_execLatency)[opcodeName]);
      targetOpt.erase(opcodeName);
    }
  }
  if (!targetOpt.empty()) {
    cout<<"\033[0;31mPlease check the operations targeting multi-cycle execution in <param.json>:\"\033[0m";
    for (set<string>::iterator it = targetOpt.begin(); it != targetOpt.end(); ++it) {
      cout<<" "<<*it<<" "; // Note the "*" here
    }
    cout<<"\033[0;31m\".\033[0m"<<endl;
  }
}

void DFG::initPipelinedOpt(list<string>* t_pipelinedOpt) {
  set<string> targetOpt;
  for (string opt: *t_pipelinedOpt) {
    targetOpt.insert(opt);
  }
  for (DFGNode* node: nodes) {
    list<string>::iterator it;
    it = find(t_pipelinedOpt->begin(), t_pipelinedOpt->end(), node->getOpcodeName());
    if(it != t_pipelinedOpt->end()) {
      string opcodeName = node->getOpcodeName();
      node->setPipelinable();
      targetOpt.erase(opcodeName);
    }
  }
  if (!targetOpt.empty()) {
    cout<<"\033[0;31mPlease check the pipelinable operations in <param.json>:\"\033[0m";
    for (set<string>::iterator it = targetOpt.begin(); it != targetOpt.end(); ++it) {
      cout<<" "<<*it<<" "; // Note the "*" here
    }
    cout<<"\033[0;31m\".\033[0m"<<endl;
  }
}
/**
 * @param t_bb :input basic block
 * @param t_inst :input inst
 * @return 
 *
 * what is in this function:
 * if the t_inst is the front inst of the t_bb return true
 * else 
 *
 */
bool DFG::isLiveInInst(BasicBlock* t_bb, Instruction* t_inst) {
  if (t_inst == &(t_bb->front())) {
    errs()<<"ctrl to: "<<*t_inst<<"; front: "<<(t_bb->front())<<"; ";
    return true;
  }
  for (Instruction::op_iterator op = t_inst->op_begin(), opEnd = t_inst->op_end(); op != opEnd; ++op) {
    Instruction* tempInst = dyn_cast<Instruction>(*op);
    if (tempInst and !containsInst(t_bb, tempInst)) {
      errs()<<"ctrl to: "<<*t_inst<<"; containsInst(t_bb, tempInst): "<<containsInst(t_bb, tempInst)<<"; ";
      return true;
    }
  }

  // The first (lower ID) IR with only in-block dependency is also treated as live-in.
  for (Instruction::op_iterator op = t_inst->op_begin(), opEnd = t_inst->op_end(); op != opEnd; ++op) {
    Instruction* tempInst = dyn_cast<Instruction>(*op);
    if (tempInst and getInstID(t_bb, t_inst) > getInstID(t_bb, tempInst)) {
      return false;
    }
  }

  errs()<<"ctrl to: "<<*t_inst<<"; ";
  return true;
}

/**Judge if the inst is in the basicblock
 * @param t_bb :the input basic basicblock 
 * @param t_inst :the input inst
 * @return : true if t_inst is in t_bb
 */
bool DFG::containsInst(BasicBlock* t_bb, Instruction* t_inst) {

  for (BasicBlock::iterator II=t_bb->begin(),
       IEnd=t_bb->end(); II!=IEnd; ++II) {
    Instruction* inst = &*II;
    if ((inst) == (t_inst)) {
      return true;
    }
  }
  return false;
}

int DFG::getInstID(BasicBlock* t_bb, Instruction* t_inst) {

  int id = 0;
  for (BasicBlock::iterator II=t_bb->begin(),
       IEnd=t_bb->end(); II!=IEnd; ++II) {
    Instruction* inst = &*II;
    if ((inst) == (t_inst)) {
      return id;
    }
    id += 1;
  }
  // This never gonna happen.
  assert(false);
  return -1;
}

 /** 
	* this function is used to connect DFGNodes to generate DFG
	*
	* what is in this function:
 	* 1. cut Edges of every DFGNode 
 	* 2. Traverse all DFGEdges ,setOutEdge for the src DFGNode and setInEdeg for the dst DFGNode
 	*/
void DFG::connectDFGNodes() {
  for (DFGNode* node: nodes)
    node->cutEdges();

  // Incorporate ctrl flow into data flow.
	// this is about ctrl flow,do not care about it now
  if (!m_CDFGFused) {
    for (DFGEdge* edge: m_ctrlEdges) {
      m_DFGEdges.push_back(edge);
    }
    m_CDFGFused = true;
  }

  for (DFGEdge* edge: m_DFGEdges) {
    DFGNode* left = edge->getSrc();
    DFGNode* right = edge->getDst();
    left->setOutEdge(edge);
    right->setInEdge(edge);
  }
}

void DFG::generateJSON() {
  ofstream jsonFile;
  jsonFile.open("dfg.json");
  jsonFile<<"[\n";
  int node_index = 0;
  int node_size = nodes.size();
  for (DFGNode* node: nodes) {
    jsonFile<<"  {\n";
    jsonFile<<"    \"fu\"         : \""<<node->getFuType()<<"\",\n";
    jsonFile<<"    \"id\"         : "<<node->getID()<<",\n";
    jsonFile<<"    \"org_opt\"    : \""<<node->getOpcodeName()<<"\",\n";
    jsonFile<<"    \"JSON_opt\"   : \""<<node->getJSONOpt()<<"\",\n";
    jsonFile<<"    \"in_const\"   : [";
    int const_size = node->getNumConst();
    for (int const_index=0; const_index < const_size; ++const_index) {
      jsonFile<<const_index;
      if (const_index < const_size - 1)
        jsonFile<<",";
    }
    jsonFile<<"],\n";
    jsonFile<<"    \"pre\"         : [";
    int in_size = node->getPredNodes()->size();
    int in_index = 0;
    for (DFGNode* predNode: *(node->getPredNodes())) {
      jsonFile<<predNode->getID();
      in_index += 1;
      if (in_index < in_size)
        jsonFile<<",";
    }
    jsonFile<<"],\n";
    jsonFile<<"    \"succ\"       : [[";
    int out_size = node->getSuccNodes()->size();
    int out_index = 0;
    for (DFGNode* succNode: *(node->getSuccNodes())) {
      jsonFile<<succNode->getID();
      out_index += 1;
      if (out_index < out_size)
        jsonFile<<",";
    }
    jsonFile<<"]]\n";
    node_index += 1;
    if (node_index < node_size)
      jsonFile<<"  },\n";
    else
      jsonFile<<"  }\n";
  }
  jsonFile<<"]\n";
}

void DFG::generateDot(Function &t_F, bool t_isTrimmedDemo) {

  error_code error;
//  sys::fs::OpenFlags F_Excl;
  string func_name = t_F.getName().str();
  string file_name = func_name + ".dot";
  StringRef fileName(file_name);
  raw_fd_ostream file(fileName, error, sys::fs::OF_None);

  file << "digraph \"DFG for'"  << t_F.getName() << "\' function\" {\n";

  //Dump DFG nodes.
  for (DFGNode* node: nodes) {
//    if (dyn_cast<Instruction>((*node)->getInst())) {
    if (t_isTrimmedDemo) {
      file << "\tNode" << node->getID() << node->getOpcodeName() << "[shape=record, label=\"" << "(" << node->getID() << ") " << node->getOpcodeName() << "\"];\n";
    } else {
      file << "\tNode" << node->getInst() << "[shape=record, label=\"" <<
          changeIns2Str(node->getInst()) << "\"];\n";
    }
  }
  /*
    if(dyn_cast<Instruction>(node->first))
      file << "\tNode" << node->first << "[shape=record, label=\"" << *(node->first) << "\"];\n";
      file << "\tNode" << (*node)->getInst() << "[shape=record, label=\"" << ((*node)->getID()) << "\"];\n";
    else {
      file << "\tNode" << (*node)->getInst() << "[shape=record, label=\"" << (*node)->getStringRef() << "\"];\n";
    }
            file << "\tNode" << node->first << "[shape=record, label=\"" << node->second << "\"];\n";
  */


  // Dump control flow.
  file << "edge [color=blue]" << "\n";
  for (DFGEdge* edge: m_ctrlEdges) {
    // Distinguish data and control flows. Don't show the ctrl flows that are optimzied out from the data flow optimization.
    if (find(m_DFGEdges.begin(), m_DFGEdges.end(), edge) != m_DFGEdges.end()) {
      if (t_isTrimmedDemo) {
        file << "\tNode" << edge->getSrc()->getID() << edge->getSrc()->getOpcodeName() << " -> Node" << edge->getDst()->getID() << edge->getDst()->getOpcodeName() << "\n";
      } else {
        file << "\tNode" << edge->getSrc()->getInst() << " -> Node" << edge->getDst()->getInst() << "\n";
      }
    }
  }

  // Dump data flow.
  file << "edge [color=red]" << "\n";
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
    if (find(m_ctrlEdges.begin(), m_ctrlEdges.end(), edge) == m_ctrlEdges.end()) {
      if (t_isTrimmedDemo) {
        file << "\tNode" << edge->getSrc()->getID() << edge->getSrc()->getOpcodeName() << " -> Node" << edge->getDst()->getID() << edge->getDst()->getOpcodeName() << "\n";
      } else {
        file << "\tNode" << edge->getSrc()->getInst() << " -> Node" << edge->getDst()->getInst() << "\n";
      }
    }
  }
//  cout << "Write data flow done.\n";
  file << "}\n";
  file.close();

}

void DFG::DFS_on_DFG(DFGNode* t_head, DFGNode* t_current,
    list<DFGNode*>* t_visitedNodes, list<DFGEdge*>* t_erasedEdges,
    list<DFGEdge*>* t_currentCycle, list<list<DFGEdge*>*>* t_cycles) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (find(t_erasedEdges->begin(), t_erasedEdges->end(), edge) != t_erasedEdges->end())continue;
    // check whether the IR is equal
    if (edge->getSrc() == t_current) {
      // skip the visited nodes/edges:
      if (find(t_currentCycle->begin(), t_currentCycle->end(), edge) != t_currentCycle->end())continue;
      t_currentCycle->push_back(edge);//push_back 是在列表的末尾添加一个元素。

      if (edge->getDst() == t_head) {
        cout << "==================================\n";
        errs() << "[detected one cycle] head: "<<*(t_head->getInst())<<"\n";
        list<DFGEdge*>* temp_cycle = new list<DFGEdge*>();
        for (DFGEdge* currentEdge: *t_currentCycle) {
          temp_cycle->push_back(currentEdge);
          // break the cycle to avoid future repeated detection
          errs() << "cycle edge: {" << *(currentEdge)->getSrc()->getInst() << "  } -> {"<< *(currentEdge)->getDst()->getInst() << "  } ("<<currentEdge->getSrc()->getID()<<" -> "<<currentEdge->getDst()->getID()<<")\n";
        }
        t_erasedEdges->push_back(edge);
        t_cycles->push_back(temp_cycle);
        t_currentCycle->remove(edge);//remove方法用于从列表中删除指定值的所有元素。
      } else {
        if (find(t_visitedNodes->begin(), t_visitedNodes->end(), edge->getDst()) == t_visitedNodes->end()) {
          t_visitedNodes->push_back(edge->getDst());
          // Only continue when the path size is less than the node count.
          if (t_currentCycle->size() <= nodes.size()) {
            DFS_on_DFG(t_head, edge->getDst(), t_visitedNodes, t_erasedEdges, t_currentCycle, t_cycles);
          }
        } else {
          t_currentCycle->remove(edge);
        }
      }
    }
  }
  if (t_currentCycle->size()!=0) {
    t_currentCycle->pop_back();//pop_back是在列表的末尾删除一个元素。
  }
}

list<list<DFGEdge*>*>* DFG::calculateCycles() {
  list<list<DFGEdge*>*>* cycleLists = new list<list<DFGEdge*>*>();
  list<DFGEdge*>* currentCycle = new list<DFGEdge*>();
  list<DFGNode*>* visitedNodes = new list<DFGNode*>();
  list<DFGEdge*>* erasedEdges = new list<DFGEdge*>();
  cycleLists->clear();
  for (DFGNode* node: nodes) {
    currentCycle->clear();
    visitedNodes->clear();
    visitedNodes->push_back(node);
    DFS_on_DFG(node, node, visitedNodes, erasedEdges, currentCycle, cycleLists);
  }
  int cycleID = 0;
  m_cycleNodeLists->clear();
  for (list<DFGEdge*>* cycle: *cycleLists) {
    list<DFGNode*>* nodeCycle = new list<DFGNode*>();
    for (DFGEdge* edge: *cycle) {
      edge->getDst()->setCritical();
      edge->getDst()->addCycleID(cycleID);
      nodeCycle->push_back(edge->getDst());
    }
    m_cycleNodeLists->push_back(nodeCycle);
    cycleID += 1;
  }
  return cycleLists;
}

list<list<DFGNode*>*>* DFG::getCycleLists() {
  return m_cycleNodeLists;
}

void DFG::showOpcodeDistribution() {

  map<string, int> opcodeMap;
  for (DFGNode* node: nodes) {
    opcodeMap[node->getOpcodeName()] += 1;
  }
  for (map<string, int>::iterator opcodeItr=opcodeMap.begin();
      opcodeItr!=opcodeMap.end(); ++opcodeItr) {
    cout << (*opcodeItr).first << " : " << (*opcodeItr).second << "\n";
  }
  int simdNodeCount = 0;
  for (DFGNode* node: nodes) {
    if (node->isVectorized()) {
      simdNodeCount++;
    }
  }    
  cout << "DFG node count: "<<nodes.size()<<"; DFG edge count: "<<m_DFGEdges.size()<<"; SIMD node count: "<<simdNodeCount<<"\n";
}

int DFG::getID(DFGNode* t_node) {
  int index = 0;
  return t_node->getID();
}

DFGNode* DFG::getNode(Value* t_value) {
  for (DFGNode* node: nodes) {
    if (node->getInst() == t_value) {
      return node;
    }
  }
  assert("ERROR cannot find the corresponding DFG node.");
  return NULL;
}

/** judge if the nodes for this inst have been created in nodes
 * @param t_vlaue: the inst need to judge
 * @return true:have existes in nodes false:...
*/
bool DFG::hasNode(Value* t_value) {
  for (DFGNode* node: nodes) {
    if (node->getInst() == t_value) {
      return true;
    }
  }
  return false;
}

DFGEdge* DFG::getCtrlEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_ctrlEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return edge;
    }
  }
  assert("ERROR cannot find the corresponding Ctrl edge.");
  return NULL;
}

bool DFG::hasCtrlEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_ctrlEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return true;
    }
  }
  return false;
}


DFGEdge* DFG::getDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return edge;
    }

  }
  assert("ERROR cannot find the corresponding DFG edge.");
  return NULL;
}

bool DFG::hasDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return true;
    }
  }
  return false;
}

string DFG::changeIns2Str(Instruction* t_ins) {
  string temp_str;
  raw_string_ostream os(temp_str);
  t_ins->print(os);
  return os.str();
}

//get value's name or inst's content
StringRef DFG::getValueName(Value* t_value) {
  string temp_result = "val";
  if (t_value->getName().empty()) {
    temp_result += to_string(m_num);
    m_num++;
  }
  else {
    temp_result = t_value->getName().str();
  }
  StringRef result(temp_result);
//  cout << "" << result;
  return result;
}

int DFG::getNodeCount() {
  return nodes.size();
}
