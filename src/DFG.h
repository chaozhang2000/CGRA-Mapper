/**
 * @file DFG.h
 * @author Cheng Tan 
 * @brief  the defination of DFG class
 * @version 0.1
 */

#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Use.h>
#include <llvm/Analysis/CFG.h>
#include <llvm/Analysis/LoopInfo.h>
#include <list>
#include <set>
#include <map>
#include <iostream>

#include "DFGNode.h"
#include "DFGEdge.h"

using namespace llvm;
using namespace std;
///the class of DFG
class DFG {
  private:
    int m_num;
    bool m_CDFGFused;
    bool m_targetFunction;
    bool m_precisionAware;
    list<DFGNode*>* m_orderedNodes;//排序后的DFGNode
    list<Loop*>* m_targetLoops; //目标循环

		/**List to save the pointer of DFGEdges in DFG
		 */
    list<DFGEdge*> m_DFGEdges; 
															 
    list<DFGEdge*> m_ctrlEdges;

    string changeIns2Str(Instruction* ins);
    //get value's name or inst's content
    StringRef getValueName(Value* v);

		/**
		 * this function is used to get the path from t_current to t_head in dfg, if the t_head == t_current,it will get the cycles in dfg. but now, the branch is not support,so cycles is not exsisted in dfg,this function is not necessory
		 * @param t_head : 
		 * @param t_current : 
		 * @param t_visitedNodes : 
		 * @param t_erasedEdges : 
		 * @param t_currentCycle :
		 * @param t_cycles : 
		 */
		void DFS_on_DFG(DFGNode* t_head, DFGNode* t_current,
    list<DFGNode*>* t_visitedNodes, list<DFGEdge*>* t_erasedEdges,
    list<DFGEdge*>* t_currentCycle, list<list<DFGEdge*>*>* t_cycles); 

    DFGNode* getNode(Value*);
    bool hasNode(Value*);

		/**Get the pointer of DFGEdge from t_src to t_dst DFGNode.The DFGEdge must be confirmed to have been created.You can use hasDFGEdge() to check this.
		 * @param t_src the pointer to the src DFGNode
		 * @param t_dst the pointer to the dst DFGNode
		 * @return DFGEdge* the pointer to DFGEdge we want to find.
		 */
    DFGEdge* getDFGEdge(DFGNode* t_src, DFGNode* t_dst);

		/**Check if the DFGEdge from t_src to t_dst DFGNode has be created
		 * @param t_src the pointer to the src DFGNode
		 * @param t_dst the pointer to the dst DFGNode
		 * @return true main the DFGEdge from t_src to t_dst is in m_DFGEdges,has been created in the past
		 */
    bool hasDFGEdge(DFGNode* t_src, DFGNode* t_dst);
    DFGEdge* getCtrlEdge(DFGNode*, DFGNode*);
    bool hasCtrlEdge(DFGNode*, DFGNode*);
    bool shouldIgnore(Instruction*);
    void combinePhiAdd();
    void connectDFGNodes();
    bool isLiveInInst(BasicBlock*, Instruction*);
    bool containsInst(BasicBlock*, Instruction*);
    int getInstID(BasicBlock*, Instruction*);

		/** this function Reorder the DFG nodes based on the longest path
		 * the public var nodes(list<DFGNode*> nodes)will be clean and rebuild in this function
		 */
    void reorderInLongest();

		/** find the longest path in dfg from targetDFGNode
		 * this function use DFS (Depth-First Search)
		 * DFS(Depth-First Search) 深度优先搜索算法是一种用于遍历或搜索图的非线性数据结构的算法。
		 * 它从起始顶点开始，沿着一条路径尽可能深入图中之前的每个未访问的顶点，直到达到最深的顶点为止。
		 * 然后，回溯到上一个节点，继续探索其他分支直到所有节点都被访问到。
		 * @param t_targetPath the list to save DFGNodes in the longest path
		 * @param targetDFGNode if this param is true,generate the DFG for all inst in function,if this param is false generate the DFG only for the target loop in the function.
		 */
		void reorderDFS(set<DFGNode*>* t_visited, list<DFGNode*>* t_targetPath,
                     list<DFGNode*>* t_curPath, DFGNode* targetDFGNode);

    void initExecLatency(map<string, int>*);
    void initPipelinedOpt(list<string>*);

		/** judge if the target DFGNode(t_node) is unvisited and has the fewest previous DFGNodes among the nodes that haven't been visited(give a level value).
		 * @param t_visited : a set of DFGNode which have been visited
		 * @param t_map : a map which records how many unvisited previous node each node has
		 * @param t_node : the target DFGNode.
		 * @return bool : if the target node is unvisited and with the fewest unvisited previous nodes,return true.else return false. 
		 */
		bool isMinimumAndHasNotBeenVisited(set<DFGNode*>* t_visited, map<DFGNode*, int>* t_map, DFGNode* t_node);

  public:
		/**The constructor function of class DFG
		 * @param t_F the function processed by functionpass
		 * @param the list of target loops in the t_F,which is produced by the getTargetLoops function in mapperPass.cpp.But now it is always empty because our test functions do not contain loops now.
		 * @param t_targetFunction if this param is true,generate the DFG for all inst in function,if this param is false generate the DFG only for the target loop in the function.
		 * @param t_precisionAware TODO
		 */
		DFG(Function& t_F, list<Loop*>* t_loops, bool t_targetFunction,
         bool t_precisionAware,
         map<string, int>* t_execLatency, list<string>* t_pipelinedOpt);

		/** the list to save cycles(环) in DFG,
		 * but the kernel dose not have cycles now,so this list is useless now.
		 */
    list<list<DFGNode*>*>* m_cycleNodeLists;
																						
		/**List to save the pointer of DFGNodes in DFG
		 */
    list<DFGNode*> nodes;

		/**Extract DFG from specific function 
 		* @param t_F the function pointer which the mapperPass is processing
 		*/
    void construct(Function&);
    int getNodeCount();

    void setupCycles();


		/** the function to get the cycles(环) in DFG, the cycles will be save in m_cycleNodeLists.
		 * but the kernel dose not have cycles now,so this function is not necessory now.
		 */
    list<list<DFGEdge*>*>* calculateCycles();

    list<list<DFGNode*>*>* getCycleLists();
    int getID(DFGNode*);
    bool isLoad(DFGNode*);
    bool isStore(DFGNode*);

		/** Print the information of DFG.
		 * Print the name and number of occurrences of each operation that appears in the data flow graph
		 * Print the number of DFGNodes and DFGLinks in the DFG
		 */
    void showOpcodeDistribution();

		/** Generate the Dot file according the DFG
		 * we can latter use dot tool to generate the png file of DFG.
 		 * @param t_F : the function pointer which the mapperPass is processing
 		 * @param t_isTrimmedDemo : use to control the generation of different dot(png) files. If this value is true , the name of the DFGNode in the dot(png) is the opcode of the inst,but if this value is false ,the name will be the complete instruction. this value is given from the "isTrimmedDemo" param in param.json.
		 */
		void generateDot(Function &t_F, bool t_isTrimmedDemo);

		/** Generate the JSON file according the DFG
		 */
    void generateJSON();
};
