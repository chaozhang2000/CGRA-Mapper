/*
 * ======================================================================
 * CGRA.h
 * ======================================================================
 * CGRA implementation header file.
 *
 * Author : Cheng Tan
 *   Date : July 16, 2019
 */

//#include "llvm/Pass.h"
#include "CGRANode.h"
#include "CGRALink.h"
#include <iostream>
//#include <llvm/Support/raw_ostream.h>

using namespace llvm;

class CGRA {
  private:
		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_FUCount;

		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_LinkCount;

		/** the var to save the rows of CGRAlinks in the CGRA
		 */
    int m_rows;

		/** the var to save the columns of CGRAlinks in the CGRA
		 */
    int m_columns;

  public:
		
		/**The constructor function of class CGRA
		 * @param t_rows : the rows of the CGRA
		 * @param t_columns : the columns of the CGRA
		 * @param t_parameterizableCGRA : Use the information of paramCGRA.json to construct the CGRA.but this function is not completed, so always set this value false in param.json
		 * @param t_additionalFunc : a map to record the function name and the list of CGRANode's ID. every function in this map will be enable in all the CGRANodes with the IDs in the list. the value of this map comes from param.json.we can specify a few nodes to perform load and store operations.If not specified,the first column CGRANodes can perform load and store operations
		 */
		CGRA(int t_rows, int t_columns,
	   	bool t_parameterizableCGRA,
	   	map<string, list<int>*>* t_additionalFunc);

    CGRANode ***nodes;
    CGRALink **links;
    int getFUCount();
    int getLinkCount();
    void getRoutingResource();

		/**The function to construct MRRG
		 * this function traverse all CGRANodes and CGRALinks in CGRA,and call their constructMRRG method
		 * @param t_II : the value of II
		 */
		void constructMRRG(int t_II);

    int getRows() { return m_rows; }
    int getColumns() { return m_columns; }

		/**The function to get CGRALink from CGRANode t_n1 to CGRANode t_n2
		 * this function traverse each CGRALinks in CGRA to check if the src is t_n1 and dst is t_n2.
		 * @param t_n1 : the src CGRANode
		 * @param t_n2 : the dst CGRANode
		 * @return : if the CGRALink is found, return the pointer of this CGRALink,else return NULL.
		 */
		CGRALink* getLink(CGRANode* t_n1, CGRANode* t_n2);

		/**The function to set the number of bypass channel in every CGRALink of the CGRA.
		 * Traverse every CGRALink in the CGRA and call CGRALink.setBypassConstraint for every CGRALink.
		 * TODO why setBypassConstraint is a function of CGRALink but not CGRANode ?
		 * @param t_bypassConstraint : the size of bypass channel
		 */
		void setBypassConstraint(int t_bypassConstraint);

		/**The function to set the size of regs in every CGRANode and CGRALink of the CGRA.
		 * Traverse every CGRANode in the CGRA and call CGRANode.setCtrlMemConstraint CGRALink.setCtrlMemConstraint for every CGRANode and CGRALink.
		 * TODO why CGRALink need setCtrlMemConstraint ?
		 * @param t_ctrlMemConstraint : the size of ctrlMem 
		 */
		void setCtrlMemConstraint(int t_ctrlMemConstraint);

		/**The function to set the number of regs in every CGRANode of the CGRA.
		 * Traverse every CGRANode in the CGRA and call CGRANode.setRegConstraint for every CGRANode.
		 * @param t_registerConstraint : the number of regs
		 */
		void setRegConstraint(int t_regConstraint);
};
