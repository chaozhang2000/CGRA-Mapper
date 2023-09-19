/*
 * ======================================================================
 * CGRALink.h
 * ======================================================================
 * CGRA link implementation header file.
 *
 * Author : Cheng Tan
 *   Date : July 16, 2019
 */

#ifndef CGRALink_H
#define CGRALink_H

//#include <llvm/Support/raw_ostream.h>
//#include <llvm/Support/FileSystem.h>

#include "CGRANode.h"
#include "DFGNode.h"

//using namespace llvm;
using namespace std;

class CGRANode;

class CGRALink
{
  private:
    int m_id;

		/**record the pointer of src CGRANode
		 */
    CGRANode *m_src;

		/**record the pointer of dst CGRANode
		 */
    CGRANode *m_dst;
		
    int m_II;
    int m_ctrlMemSize;
    int m_bypassConstraint;
    int m_currentCtrlMemItems;

    int m_cycleBoundary;
    bool m_disabled;
    bool* m_occupied;
    bool* m_bypassed;
    bool* m_generatedOut;
    bool* m_arrived;
    DFGNode** m_dfgNodes;
    bool satisfyBypassConstraint(int, int);

  public:
		/**The constructor function of class CGRALink
		 * this function init CGRANode's ID according the params,other var is init by default value.
		 * @param t_linkId : the id of the CGRALink
		 */
		CGRALink(int t_linkId);

    void setID(int);
    int getID();
    CGRANode*  getSrc();
    CGRANode*  getDst();

		/**Connect the CGRALink to src and dst CGRANodes,by set m_src=t_src and m_dst=t_dst
		 * @param t_src : the pointer to the src CGRAnode
		 * @param t_dst : the pointer to the dst CGRAnode
		 */
		void connect(CGRANode* t_src, CGRANode* t_dst);

    CGRANode* getConnectedNode(CGRANode*);

    void constructMRRG(int, int);
    bool canOccupy(int, int);
    bool isOccupied(int);
    bool isOccupied(int, int, bool);
    bool canOccupy(DFGNode*, CGRANode*, int, int);
    void occupy(DFGNode*, int, int, int, bool, bool, bool);
    bool isBypass(int);
    string getDirection(CGRANode*);
    int getDirectionID(CGRANode*);
    bool isReused(int);
    DFGNode* getMappedDFGNode(int);
    void setCtrlMemConstraint(int);
		/** TODO: there may be a misunderstanding of this function. and why this function is only defined in CGRALink is not clear. 
		 * set the number of CGRAlinks(bypass channel) that each CGRAnode interacts with surrounding nodes.
		 * set m_bypassConstraint the value of t_bypassConstraint,this value comes from "bypassConstraint" param in param.json.
		 * @param t_bypassConstraint : the number of the bypass channel.
		 */
		void setBypassConstraint(int t_bypassConstraint);
    int getBypassConstraint();
    void disable();
};

#endif
