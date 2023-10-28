/*
 * ======================================================================
 * CGRANode.h
 * ======================================================================
 * CGRA tile implementation header file.
 *
 * Author : Cheng Tan
 *   Date : July 16, 2019
 */

#ifndef CGRANode_H
#define CGRANode_H

#include "CGRALink.h"
#include "DFGNode.h"
#include <iostream>
//#include <llvm/IR/Function.h>
//#include <llvm/IR/Value.h>
//#include <llvm/Support/raw_ostream.h>
//#include <llvm/Support/FileSystem.h>
#include <list>
#include <map>
#include <string>

using namespace std;
using namespace llvm;

class CGRALink;
class DFGNode;

class CGRANode {

  private:
//    typedef std::pair<Value*, StringRef> DFG_Node;
    int m_id;
    int m_x;
    int m_y;

		/**the value to record the number of regs in a CGRANode
		 */
    int m_registerCount;

		/**the list of regs in CGRANode(PE)
		 */
    list<float> m_registers;

		/**the value to record the size of ctrlMem in a CGRANode
		 */
    int m_ctrlMemSize;

    int m_currentCtrlMemItems;
    float* m_ctrlMem;

		/**the list to record input CGRALinks of this CGRANode
		 */
    list<CGRALink*> m_inLinks;
		/**the list to record output CGRALinks of this CGRANode
		 */
    list<CGRALink*> m_outLinks;

    list<CGRALink*>* m_occupiableInLinks;
    list<CGRALink*>* m_occupiableOutLinks;
    list<CGRANode*>* m_neighbors;

    // functional unit occupied with cycle going on
		
		/**the value to save the maximum number of clock cycles, the best value is II.This value also comes into play when building the MRRG.But we don't know the value of II before finishing Mapping,so we chose a big enough value. 
		 */
    int m_cycleBoundary;

    int* m_fuOccupied;
    DFGNode** m_dfgNodes;
    map<CGRALink*,bool*> m_xbarOccupied;

		/**true main disable the CGRANode
		 */
    bool m_disabled;

    bool m_canReturn;
    bool m_canStore;
    bool m_canLoad;
    bool m_canCall;
    bool m_canAdd;
    bool m_canMul;
    bool m_canShift;
    bool m_canPhi;
    bool m_canSel;
    bool m_canCmp;
    bool m_canMAC;
    bool m_canLogic;
    bool m_canBr;

		/** This value record during a certain clock cycle, a register is occupied by data from a certain port
		 * m_regs_duration[cycle+d][i] = t_port_id
		 */
    int** m_regs_duration;

		/**TODO:this value has appeared in CGRANode::constructMRRG() but it's meaning is not clear yet.
		 */
    int** m_regs_timing;

		/**this value record the dfgNodes which has been Occupy and their Occupy status.
		 * the pair<DFGNode*,int> record the DFGNode which is Occupyed and it's Occupy status (SINGLE OCCUPY,START_PIPE_OCCUPY and so on)
		 * the list is used to record the Occupyed DFGNode in the same clock cycle.
		 * the vector is used to record the different clock cycle's Occupyed DFGNodes.
		 */
    vector<list<pair<DFGNode*, int>>*> m_dfgNodesWithOccupyStatus;

  public:
		/**The constructor function of class CGRANode
		 * this function init CGRANode's ID,x and y according the params,other var is init by default value.
		 * the function add,mul,shift and so on is turned on by default,but the load and store is turned off default
		 * @param t_id : the id of the CGRANode
		 * @param t_x : the x of the CGRANode
		 * @param t_y : the y of the CGRANode 
		 */
		CGRANode(int t_id, int t_x, int t_y);

		/**The function to set the number of regs in a CGRANode.
		 * set the m_registerCount the value of t_registerConstraint
		 * the number of regs in a CGRANode comes from the param "regConstraint" in param.json
		 * @param t_registerConstraint : the number of regs
		 */
		void setRegConstraint(int t_registerConstraint);

		/**The function to set the size of ctrlMem in a CGRANode.
		 * set the m_ctrlMemSize the value of t_ctrlMemConstraint
		 * the size of ctrlMem in a CGRANode comes from the param "ctrlMemConstraint" in param.json
		 * @param t_ctrlMemConstraint : the size of ctrlMem
		 */
		void setCtrlMemConstraint(int t_ctrlMemConstraint);

    void setID(int);
    void setLocation(int, int);
    int getID();

		/**The function to enable Function of the CGRANode according to the string of func name
		 * the store,load,return,call,complex is supported now
		 * @param t_func : the function want to enable
		 * @return true mains enable successfully
		 */
		bool enableFunctionality(string t_func);

    void enableReturn();
    void enableStore();
    void enableLoad();
    void enableCall();
    void enableAdd();
    void enableMul();
    void enableShift();
    void enablePhi();
    void enableSel();
    void enableCmp();
    void enableMAC();
    void enableLogic();
    void enableBr();

		/**add the CGRALink to the list of this CGRANode's InCGRALinks
		 * @param t_link : the pointer of the in CGRALink
		 */
		void attachInLink(CGRALink* t_link);

		/**add the CGRALink to the list of this CGRANode's outCGRALinks
		 * @param t_link : the pointer of the out CGRALink
		 */
		void attachOutLink(CGRALink* t_link);

    list<CGRALink*>* getInLinks();
    list<CGRALink*>* getOutLinks();
    CGRALink* getInLink(CGRANode*);
    CGRALink* getOutLink(CGRANode*);
    list<CGRANode*>* getNeighbors();

		/**The function to construct MRRG in CGRANode Class
		 * @param t_CGRANodeCount : the number of FU in CGRA
		 * @param t_II : the Value of II
		 */
		void constructMRRG(int t_CGRANodeCount, int t_II);
    bool canSupport(DFGNode*);
    bool isOccupied(int, int);
    // bool canOccupy(int, int);
    bool canOccupy(DFGNode*, int, int);

		/**The function to record the DFGNode mapped to this CGRANode.
		 * 1. give value to m_dfgNodesWithOccupyStatus according to the t_cycle and t_opt.
		 * 2. call t_opt's setMapped method.
		 * @param t_opt : the DFGNode which mapped to this CGRANode
		 * @param t_cycle : the clock cycle when map the DFGnode(t_opt) to this CGRANode
		 * @param t_II : the value of II
		 * @param t_isStaticElasticCGRA : now is always false
		 */
		void setDFGNode(DFGNode* t_opt, int t_cycle, int t_II, bool t_isStaticElasticCGRA);
    void configXbar(CGRALink*, int, int);
    void addRegisterValue(float);
    list<CGRALink*>* getOccupiableInLinks(int, int);
    list<CGRALink*>* getOccupiableOutLinks(int, int);
    int getAvailableRegisterCount();
    int getMinIdleCycle(DFGNode*, int, int);
    int getCurrentCtrlMemItems();
    int getX();
    int getY();
    bool canReturn();
    bool canStore();
    bool canLoad();
    bool canCall();
    bool canAdd();
    bool canMul();
    bool canShift();
    bool canPhi();
    bool canSel();
    bool canCmp();
    bool canMAC();
    bool canLogic();
    bool canBr();
		/**The function get MappedDFGNode in this CGRANode by searching in m_dfgNodesWithOccupyStatus 
		 * @param t_cycle : the value of cycle 
		 * @return : the pointer of the mappedDFGNode in this CGRANode at t_cycle. if not find, return NULL
		 */
    DFGNode* getMappedDFGNode(int t_cycle);

    bool containMappedDFGNode(DFGNode*, int);

		/**The function used to allocate Reg for CGRALink(t_link). 
		 * get reg_id from t_link using t_link->getDirectionID() then call the allocateReg(int,int,int,int) function. 
		 * allocateReg(int,int,int,int)function, Traverse each reg in this CGRANode, to find a reg which can be occupied according to the m_regs_duration,then occupied it by setting the m_regs_duration.
		 * @param t_link : 
		 * @param t_cycle : 
		 * @param t_duration : 
		 * @param t_II : 
		 */
		void allocateReg(CGRALink* t_link, int t_cycle, int t_duration, int t_II);
		void allocateReg(int t_port_id, int t_cycle, int t_duration, int t_II);

    int* getRegsAllocation(int);

		/**disable the CGRANode
		 * set the m_disabled true, main this CGRANode is disabled
		 * disable all in CGRALinks and out CGRALinks
		 */
    void disable();

		/**disable all function of the CGRANode
		 * set m_canReturn,m_canStore,m_canLoad and so on the value flase
		 */
    void disableAllFUs();
};

#endif
