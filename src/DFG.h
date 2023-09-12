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

    //edges of data flow
    list<DFGEdge*> m_DFGEdges; //DFG箭头
    list<DFGEdge*> m_ctrlEdges; //控制箭头

    string changeIns2Str(Instruction* ins);
    //get value's name or inst's content
    StringRef getValueName(Value* v);
    void DFS_on_DFG(DFGNode*, DFGNode*, list<DFGNode*>*, list<DFGEdge*>*,
        list<DFGEdge*>*, list<list<DFGEdge*>*>*);
    DFGNode* getNode(Value*);
    bool hasNode(Value*);
    DFGEdge* getDFGEdge(DFGNode*, DFGNode*);
    void deleteDFGEdge(DFGNode*, DFGNode*);
    void replaceDFGEdge(DFGNode*, DFGNode*, DFGNode*, DFGNode*);
    bool hasDFGEdge(DFGNode*, DFGNode*);
    DFGEdge* getCtrlEdge(DFGNode*, DFGNode*);
    bool hasCtrlEdge(DFGNode*, DFGNode*);
    bool shouldIgnore(Instruction*);
    void tuneForBranch();
    void tuneForBitcast();
    void tuneForLoad();
    void tuneForPattern();
    void combineCmpBranch();
    void combineMulAdd();
    void combinePhiAdd();
    void combine(string, string);
    void trimForStandalone();
    void detectMemDataDependency();
    void eliminateOpcode(string);
    bool searchDFS(DFGNode*, DFGNode*, list<DFGNode*>*);
    void connectDFGNodes();
    bool isLiveInInst(BasicBlock*, Instruction*);
    bool containsInst(BasicBlock*, Instruction*);
    int getInstID(BasicBlock*, Instruction*);
    // Reorder the DFG nodes (initial CPU execution ordering) in
    // ASAP (as soon as possible) or ALAP (as last as possible)
    // for mapping.
    void reorderInASAP();
    void reorderInALAP();
    void reorderInLongest();
    void reorderDFS(set<DFGNode*>*, list<DFGNode*>*,
                    list<DFGNode*>*, DFGNode*);
    void initExecLatency(map<string, int>*);
    void initPipelinedOpt(list<string>*);
    bool isMinimumAndHasNotBeenVisited(set<DFGNode*>*, map<DFGNode*, int>*, DFGNode*);

  public:
		/**The constructor function of class DFG
		 * @param t_F the function processed by functionpass
		 * @param the list of target loops in the t_F,which is produced by the getTargetLoops function in mapperPass.cpp.But now it is always empty because our test functions do not contain loops now.
		 * @param t_targetFunction if this param is true,generate the DFG for all inst in function,if this param is false generate the DFG only for the target loop in the function.
		 * @param t_precisionAware TODO
		 * @param t_heterogeneity TODO 
		 */
		DFG(Function& t_F, list<Loop*>* t_loops, bool t_targetFunction,
         bool t_precisionAware, bool t_heterogeneity,
         map<string, int>* t_execLatency, list<string>* t_pipelinedOpt);

		/**TODO
		 */
    list<list<DFGNode*>*>* m_cycleNodeLists;//时钟节点列表
		/**TODO
		 */
    list<DFGNode*> nodes;//DFG 节点列表

    list<DFGNode*>* getBFSOrderedNodes();
    list<DFGNode*>* getDFSOrderedNodes();
		/**Extract DFG from specific function 
 		* @param t_F the function pointer which the mapperPass is processing
 		*/
    void construct(Function&);
    int getNodeCount();

    void setupCycles();
    list<list<DFGEdge*>*>* calculateCycles();
    list<list<DFGNode*>*>* getCycleLists();
    int getID(DFGNode*);
    bool isLoad(DFGNode*);
    bool isStore(DFGNode*);
    void showOpcodeDistribution();
    void generateDot(Function&, bool);
    void generateJSON();
};
