/*
 * ======================================================================
 * DFGEdge.h
 * ======================================================================
 * DFG edge implementation header file.
 *
 * Author : Cheng Tan
 *   Date : July 19, 2019
 */

#ifndef DFGEdge_H
#define DFGEdge_H

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include "DFGNode.h"

using namespace llvm;

class DFGNode;

class DFGEdge
{
  private:
		/**The var to record DFGEdge ID 
		 */
    int m_id;
		/**The var to record src DFGNode,the edge points from src DFGNode to dst DFGNode
		 */
    DFGNode *m_src;
		/**The var to record dst DFGNode,the edge points from src DFGNode to dst DFGNode
		 */
    DFGNode *m_dst;
    bool m_isCtrlEdge;

  public:

		/**The constructor function of class DFGEdge
		 * assign value to m_id,m_src and m_dst to create a Edge from src DFGNode to dst DFGNode
		 * @param t_id the id of the DFGEdge created
		 * @param t_src pointer to the src DFGNode 
		 * @param t_dst pointer to the dst DFGNode
		 */
    DFGEdge(int t_id, DFGNode* t_src, DFGNode* t_dst);
    DFGEdge(int t_id, DFGNode* t_src, DFGNode* t_dst, bool t_isCtrlEdge);
    void setID(int);
    int getID();
    DFGNode* getSrc();
    DFGNode* getDst();
    void connect(DFGNode*, DFGNode*);
    DFGNode* getConnectedNode(DFGNode*);
    bool isCtrlEdge();
};

#endif
