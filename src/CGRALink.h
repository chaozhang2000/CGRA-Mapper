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

		/** The same as the m_cycleBoundary in CGRANode class
		 */
    int m_cycleBoundary;

    bool m_disabled;

		/**this value record this CGRALink is occupy at a certain clock cycle.
     *m_occupied[cycle] = true;
		 */
    bool* m_occupied;

		/**this value record this CGRALink is bypass at a certain clock cycle.
		 *m_bypassed[cycle] = true;
		 */
    bool* m_bypassed;

		/**this value record this CGRALink is generatedOut at a certain clock cycle.
		 * TODO: but I don't know what is generatedOut now.
		 * m_generatedOut[cycle] = true;
		 */
    bool* m_generatedOut;

		/**this value record this CGRALink is arrived at a certain clock cycle.
		 * TODO: but I don't know what is arrived now.
		 * m_arrived[cycle] = true;
		 */
    bool* m_arrived;

		/**this value record this CGRALink is occupy by which DFGNode at a certain clock cycle.
     *m_dfgNodes[cycle] = t_srcDFGNode;
		 */
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

		/**The function to construct MRRG in CGRALink Class
		 * @param t_CGRANodeCount : the number of FU in CGRA
		 * @param t_II : the Value of II
		 */
		void constructMRRG(int t_CGRANodeCount, int t_II);

    bool canOccupy(int, int);
    bool isOccupied(int);
    bool isOccupied(int, int, bool);
    bool canOccupy(DFGNode*, CGRANode*, int, int);

		/** occupy the CGRALink,add value to m_dfgNodes,m_arrived,m_generatedOut,m_bypassed,m_occupyed.according to the params of function.
		 * call m_dst's allocateReg() method
		 * @param t_srcDFGNode : the srcDFGNode which will occupy this CGRALink
		 * @param t_cycle : the clock cycle now
		 * @param t_duration : 
		 * @param t_II : the value of II
		 * @param t_isBypass : if this CGRALink is bypass in this clock cycle
		 * @param t_isGeneratedOut : if this CGRALink is GeneratedOut in this clock cycle
		 * @param t_isStaticElasticCGRA : this is always false now
		 */
		void occupy(DFGNode* t_srcDFGNode, int t_cycle, int duration, int t_II, bool t_isBypass, bool t_isGeneratedOut, bool t_isStaticElasticCGRA);

    bool isBypass(int);
    string getDirection(CGRANode*);

		/** this function return the DirectionID,according to the X Y of m_src and m_dst.
		 * TODO: This function only applies to situations where each node is only connected to four nodes, the north,south,west and east
		 * @param t_cgraNode : the pointor to the CGRANode. this should be equal to m_src or m_dst
		 * @return : the DirectionID of this CGRALink,0123 corresponds to North South West East respectively, return -1 if error.
		 */
		int getDirectionID(CGRANode* t_cgraNode);

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
