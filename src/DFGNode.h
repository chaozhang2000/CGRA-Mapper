/**
 * @file DFGNode.h
 * @author Cheng Tan 
 * @brief  the defination of DFGNode class
 * @version 0.1
 */
#ifndef DFGNode_H
#define DFGNode_H

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <list>
#include <stdio.h>
#include <iostream>

#include "DFGEdge.h"

using namespace llvm;
using namespace std;

class DFGEdge;

class DFGNode {
  private:
    // Original id that is ordered in the original execution order (i.e.,
    // CPU/IR execution sequential ordering).
    int m_id;
    bool m_precisionAware;
    Instruction* m_inst;
    Value* m_value;
    StringRef m_stringRef;
    string m_opcodeName;

		/**the list to save the pointers of input DFGEdges
		 */
    list<DFGEdge*> m_inEdges;

		/**the list to save the pointers of output DFGEdges
		 */
    list<DFGEdge*> m_outEdges;

		/**the list to save the pointers of successor DFGNodes of this DFGNode
		 */
    list<DFGNode*>* m_succNodes;

		/**the list to save the pointers of previous DFGNodes of this DFGNode
		 */
    list<DFGNode*>* m_predNodes;

    list<DFGNode*>* m_patternNodes;
    list<int>* m_cycleID;

		/**this variable is used to record if this DFGNode have been mapped
		 */
    bool m_isMapped;

		/**this variable is used to record how many constants there are in the operands of the current DFG node
		 */
    int m_numConst;

    string m_optType;
    string m_fuType;
    bool m_isPatternRoot;
    bool m_critical;

		/** the var to save the value of DFGNode.
		 */
    int m_level;

    int m_execLatency;
    bool m_pipelinable;
    // "m_predicated" indicates whether the execution of the node depends on
    // predication or not (i.e., the predecessor probably is a "branch"). 
    bool m_isPredicatee;
    list<DFGNode*>* m_predicatees;
    bool m_isPredicater;
    DFGNode* m_patternRoot;
    void setPatternRoot(DFGNode*);

  public:
		/**The constructor function of class DFGNode
		 * @param t_id :the id that give the DFGNode
		 * @param t_precisionAware :TODO
		 * @param t_inst :The instruction corresponding to this DFGNode
		 * @param t_stringRef :TODO
		 */
		DFGNode(int t_id, bool t_precisionAware, Instruction* t_inst,
                 StringRef t_stringRef);
    int getID();
    void setID(int);

		/** Set the level of the DFGNode. set the m_level to the value of t_level.
		 * @param t_level: the level we want to set for this DFGNode
		 */
    void setLevel(int t_level);

    int getLevel();
    bool isMapped();

		/** This function set m_isMapped true
		 */
    void setMapped();

		/** clear the m_isMapped var of this DFGNode
		 */
    void clearMapped();

    bool isLoad();
    bool isStore();
    bool isReturn();
    bool isCall();
    bool isBranch();
    bool isPhi();
    bool isAdd();
    bool isMul();
    bool isCmp();
    bool isBitcast();
    bool isGetptr();
    bool isOpt(string);
    bool isVectorized();
    void setCombine();
    void addPatternPartner(DFGNode*);
    Instruction* getInst();
    StringRef getStringRef();
    string getOpcodeName();


		/**
		 * get the previous DFGnodes of this DFGNode
		 * it will get information from m_succNodes.if m_preNodes is empty,the information will be get from m_inEdges
		 * @return list<DFGNode*>* : a list of previous DFGnodes
		 */
    list<DFGNode*>* getPredNodes();

		/**
		 * get the successor DFGnodes of this DFGNode
		 * it will get information from m_succNodes.if m_succNodes is empty, the information will be get from m_outEdges
		 * @return list<DFGNode*>* : a list of successor DFGnodes
		 */
    list<DFGNode*>* getSuccNodes();

    bool isSuccessorOf(DFGNode*);
    bool isPredecessorOf(DFGNode*);
    bool isOneOfThem(list<DFGNode*>*);


		/** add t_dfgEdge to m_inEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_inEdges
		 */
    void setInEdge(DFGEdge* t_dfgEdge);
		/** add t_dfgEdge to m_outEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_outEdges
		 */
    void setOutEdge(DFGEdge* t_dfgEdge);

		/**delete the DFGEdge associated with the DFGNode
		 * clear the m_inEdges,m_outEdges,m_predNodes and m_succNodes
		 */
    void cutEdges();

    string getJSONOpt();
    string getFuType();

		/** m_numConst + 1
		 */
    void addConst();
		void removeConst();
    int getNumConst();
    void initType();
    bool isPatternRoot();
    DFGNode* getPatternRoot();
    list<DFGNode*>* getPatternNodes();
    void setCritical();
    void addCycleID(int);
    bool isCritical();
    int getCycleID();
    list<int>* getCycleIDs();
    void addPredicatee(DFGNode*);
    list<DFGNode*>* getPredicatees();
    void setPredicatee();
    bool isPredicatee();
    bool isPredicater();
    bool shareSameCycle(DFGNode*);
    void setExecLatency(int);
    bool isMultiCycleExec();
    int getExecLatency();
    void setPipelinable();
    bool isPipelinable();
    bool shareFU(DFGNode*);
};
#endif
