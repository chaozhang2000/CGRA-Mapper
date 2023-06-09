/*
 * ======================================================================
 * CGRA.cpp
 * ======================================================================
 * CGRA implementation.
 *
 * Author : Cheng Tan
 *   Date : Jan 9, 2023
 */

#include <fstream>
#include "CGRA.h"
#include "json.hpp"

using json = nlohmann::json;

CGRA::CGRA(int t_rows, int t_columns, bool t_diagonalVectorization,
	   bool t_heterogeneity, bool t_parameterizableCGRA,
	   map<string, list<int>*>* t_additionalFunc) {
	
	//初始化行列和FUCount,FUCount就是rows乘columns给nodes分配空间。
  m_rows = t_rows;
  m_columns = t_columns;
  m_FUCount = t_rows * t_columns;
  nodes = new CGRANode**[t_rows];

	//参数化地初始化CGRA
  if (t_parameterizableCGRA) {
		//根据行列来初始化CGRANode
    int node_id = 0;
    map<int, CGRANode*> id2Node;
    for (int i=0; i<t_rows; ++i) {
      nodes[i] = new CGRANode*[t_columns];
      for (int j=0; j<t_columns; ++j) {
							//CGRANode的构造函数很简单，只设置id和行列号。其他在其构造函数中都给了默认值
        nodes[i][j] = new CGRANode(node_id, j, i);
	id2Node[node_id] = nodes[i][j];
	node_id += 1;
      }
    }

		//从paramCGRA.json文件中读取参数，示例并没有采用这种方式。
		//读取参数后对每个CGRANode进行配置，这里主要配置的是将要disable的node全部disable将不启用所有fu的node先disbale所有的fu，再根据是否有accessMEM功能来确定是否开启load和store功能。
		//这里其实默认除了load和store外的所有fu功能都开启。
    ifstream paramCGRA("./paramCGRA.json");
    if (!paramCGRA.good()) {
      cout<<"Parameterizable CGRA design/mapping requires paramCGRA.json"<<endl;
      return;
    }
    json param;
    paramCGRA >> param;
    
    int numOfNodes = t_rows * t_columns;
    for (int nodeID = 0; nodeID < numOfNodes; ++nodeID) {
      bool disabled = param["tiles"][to_string(nodeID)]["disabled"];
      if (disabled) {
        id2Node[nodeID]->disable();
      } else {
        bool supportAllFUs = param["tiles"][to_string(nodeID)]["supportAllFUs"];
	if (!supportAllFUs) {
	  id2Node[nodeID]->disableAllFUs();
	}
	if (param["tiles"][to_string(nodeID)].contains("accessMem")) {
	  if (param["tiles"][to_string(nodeID)]["accessMem"]) {
	    id2Node[nodeID]->enableLoad();
	    id2Node[nodeID]->enableStore();
	  }
	}

	// TODO: need to take care of supportedFUs:
	//
      }
    }

		//从param读取links相关的参数，初始化CGRALink,并对每个srcNode和dstNode注册对应的outlink和inlink，并在对应的link中注册src和dst node
    json paramLinks = param["links"];
    m_LinkCount = paramLinks.size();
    links = new CGRALink*[m_LinkCount];

    for (int linkID = 0; linkID < paramLinks.size(); ++linkID) {
      int srcNodeID = paramLinks[linkID]["srcTile"];
      int dstNodeID = paramLinks[linkID]["dstTile"];

      links[linkID] = new CGRALink(linkID);
      id2Node[srcNodeID]->attachOutLink(links[linkID]);
      id2Node[dstNodeID]->attachInLink(links[linkID]);
      links[linkID]->connect(id2Node[srcNodeID], id2Node[dstNodeID]);
    }

    // need to perform disable() again, as it will disable the related links
    for (int nodeID = 0; nodeID < numOfNodes; ++nodeID) {
      bool disabled = param["tiles"][to_string(nodeID)]["disabled"];
      if (disabled) {
        id2Node[nodeID]->disable();
      }
    }

		//进行非参数化的CGRA配置
  } else {

		//对行列中的CGRANode进行初始化。
    int node_id = 0;
    for (int i=0; i<t_rows; ++i) {
      nodes[i] = new CGRANode*[t_columns];
      for (int j=0; j<t_columns; ++j) {
        nodes[i][j] = new CGRANode(node_id++, j, i);
      }
    }

		//这里LinkCount的数量不再是由paramCGRA中来读取，而是采用下面的方式来进行计算，可以发现，这是一种默认的链接方式。
		//这是一种这样的连接方式:每个node的上下左右都双向连接，除了最外围的一圈只往里面连接
    m_LinkCount = 2 * (t_rows * (t_columns-1) + (t_rows-1) * t_columns);
    links = new CGRALink*[m_LinkCount];

    // Enable the load/store on specific CGRA nodes based on param.json.
		//这里param.json中的格式例如
		//
  	//"additionalFunc"        : {
    //                          "load" : [4],
    //                         "store": [4]
    //                        }
		//其中的[4]表示CGRANode的编号，从第一行开始从左向右编号
    int loadCount = 0;
    int storeCount = 0;
    for (map<string, list<int>*>::iterator iter=t_additionalFunc->begin();
        iter!=t_additionalFunc->end(); ++iter) {
      for (int nodeIndex: *(iter->second)) {
        if (nodeIndex >= m_FUCount) {
          cout<<"\033[0;31mInvalid CGRA node ID "<<nodeIndex<<" for operation "<<iter->first<<"\033[0m"<<endl;
          continue;
        } else {
          int row = nodeIndex / m_columns;
          int col = nodeIndex % m_columns;
          bool canEnable = nodes[row][col]->enableFunctionality(iter->first);
          if (!canEnable) {
            cout<<"\033[0;31mInvalid operation "<<iter->first<<" on CGRA node ID "<<nodeIndex<<"\033[0m"<<endl;
          } else {
            if ((iter->first).compare("store")) {
              storeCount += 1;
            }
            if ((iter->first).compare("load")) {
              loadCount += 1;
            }
          }
        }
      }
    }
		//如果没有指定哪几个node来进行load和store会默认开启第一列的所有node可以load，store也是一样。
    if (storeCount == 0) {
      cout<<"Without customization in param.json, we enable store functionality on the left most column."<<endl;
      for (int r=0; r<t_rows; ++r) {
        nodes[r][0]->enableStore();
      }
    }
    if (loadCount == 0) {
      cout<<"Without customization in param.json, we enable load functionality on the left most column."<<endl;
      for (int r=0; r<t_rows; ++r) {
        nodes[r][0]->enableLoad();
      }
    }
		
    // Some other basic operations that can be indicated in the param.json:
		//如对所有的node开启call，根据是否使能了diagonalVectorization对部分node开启vectorization还是对全部开启,根据是否使能了heterogeneity来决定是否对奇偶数行列开启Complex()。对所有的node开启return。
    // Enable the specialized 'call' functionality.
    for (int r=0; r<t_rows; ++r) {
      for (int c=0; c<t_columns; ++c) {
        nodes[r][c]->enableCall();
      }
    }

    // Enable the vectorization.
    if (t_diagonalVectorization) {
      for (int r=0; r<t_rows; ++r) {
        for (int c=0; c<t_columns; ++c) {
          if((r+c)%2 == 0)
            nodes[r][c]->enableVectorization();
        }
      }
    } else {
      for (int r=0; r<t_rows; ++r) {
        for (int c=0; c<t_columns; ++c) {
          nodes[r][c]->enableVectorization();
        }
      }
    }


    // Enable the heterogeneity.
    if (t_heterogeneity) {
      for (int r=0; r<t_rows; ++r) {
        for (int c=0; c<t_columns; ++c) {
          if(r%2==1 and c%2 == 1)
            nodes[r][c]->enableComplex();
        }
      }
    }

    for (int r=0; r<t_rows; ++r) {
      for (int c=0; c<t_columns; ++c) {
        nodes[r][c]->enableReturn();
      }
    }

    // Connect the CGRA nodes with links.
    int link_id = 0;
    for (int i=0; i<t_rows; ++i) {
      for (int j=0; j<t_columns; ++j) {
        if (i < t_rows - 1) {
          links[link_id] = new CGRALink(link_id);
          nodes[i][j]->attachOutLink(links[link_id]);
          nodes[i+1][j]->attachInLink(links[link_id]);
          links[link_id]->connect(nodes[i][j], nodes[i+1][j]);
          ++link_id;
        }
        if (i > 0) {
          links[link_id] = new CGRALink(link_id);
          nodes[i][j]->attachOutLink(links[link_id]);
          nodes[i-1][j]->attachInLink(links[link_id]);
          links[link_id]->connect(nodes[i][j], nodes[i-1][j]);
          ++link_id;
        }
        if (j < t_columns - 1) {
          links[link_id] = new CGRALink(link_id);
          nodes[i][j]->attachOutLink(links[link_id]);
          nodes[i][j+1]->attachInLink(links[link_id]);
          links[link_id]->connect(nodes[i][j], nodes[i][j+1]);
          ++link_id;
        }
        if (j > 0) {
          links[link_id] = new CGRALink(link_id);
          nodes[i][j]->attachOutLink(links[link_id]);
          nodes[i][j-1]->attachInLink(links[link_id]);
          links[link_id]->connect(nodes[i][j], nodes[i][j-1]);
          ++link_id;
        }
      }
    }

    disableSpecificConnections();
  }
}

void CGRA::disableSpecificConnections() {
//  nodes[0][0]->disable();
//  nodes[0][1]->disable();
//  nodes[0][2]->disable();
//  nodes[0][3]->disable();
//  nodes[0][4]->disable();
//  nodes[1][0]->disable();
//  nodes[1][1]->disable();
//  nodes[1][2]->disable();
//  nodes[1][4]->disable();
//  nodes[1][3]->disable();
//  nodes[2][0]->disable();
//  nodes[2][1]->disable();
//  nodes[2][2]->disable();
//  nodes[2][3]->disable();
//  nodes[2][4]->disable();
//  nodes[3][1]->disable();
//  nodes[3][2]->disable();
//  nodes[3][2]->disable();
//  nodes[3][3]->disable();
//  nodes[3][4]->disable();
//  nodes[4][1]->disable();
//  nodes[4][2]->disable();
//  nodes[4][3]->disable();
//  nodes[4][4]->disable();

}

void CGRA::setRegConstraint(int t_regConstraint) {
  for (int i=0; i<m_rows; ++i)
    for (int j=0; j<m_columns; ++j)
      nodes[i][j]->setRegConstraint(t_regConstraint);
}

void CGRA::setBypassConstraint(int t_bypassConstraint) {
  for (int i=0; i<m_LinkCount; ++i)
    links[i]->setBypassConstraint(t_bypassConstraint);
}

void CGRA::setCtrlMemConstraint(int t_ctrlMemConstraint) {
  for (int i=0; i<m_rows; ++i)
    for (int j=0; j<m_columns; ++j)
      nodes[i][j]->setCtrlMemConstraint(t_ctrlMemConstraint);

  for (int i=0; i<m_LinkCount; ++i)
    links[i]->setCtrlMemConstraint(t_ctrlMemConstraint);
}

int CGRA::getFUCount() {
  return m_FUCount;
}

void CGRA::constructMRRG(int t_II) {
  for (int i=0; i<m_rows; ++i)
    for (int j=0; j<m_columns; ++j)
      nodes[i][j]->constructMRRG(m_FUCount, t_II);
  for (int i=0; i<m_LinkCount; ++i)
    links[i]->constructMRRG(m_FUCount, t_II);
}

CGRALink* CGRA::getLink(CGRANode* t_n1, CGRANode* t_n2) {
   for (int i=0; i<m_LinkCount; ++i) {
     if (links[i]->getSrc()==t_n1 and links[i]->getDst() == t_n2) {
       return links[i];
     }
  }

  // cout << "bad query for CGRA link\n";
  return NULL;
}

int CGRA::getLinkCount() {
  return m_LinkCount;
}
