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
    void constructMRRG(int);
    int getRows() { return m_rows; }
    int getColumns() { return m_columns; }
    CGRALink* getLink(CGRANode*, CGRANode*);
    void setBypassConstraint(int);
    void setCtrlMemConstraint(int);
    void setRegConstraint(int);
};
