/*
 * ======================================================================
 * Mapper.h
 * ======================================================================
 * Mapper implementation header file.
 *
 * Author : Cheng Tan
 *   Date : July 16, 2019
 */

#include "DFG.h"
#include "CGRA.h"

class Mapper {
  private:
    int m_maxMappingCycle;
		/** This value record the mapping result(which DFGNode is map to which CGRANode).
		 */
    map<DFGNode*, CGRANode*> m_mapping;

		/** This value record the mapping result(which DFGNode is map at which clock cycle,the int means clock cycle).
		 */
    map<DFGNode*, int> m_mappingTiming;

    map<CGRANode*, int>* dijkstra_search(CGRA*, DFG*, int, DFGNode*,
                                         DFGNode*, CGRANode*);
    int getMaxMappingCycle();
    bool tryToRoute(CGRA*, DFG*, int, DFGNode*, CGRANode*,
                    DFGNode*, CGRANode*, int, bool, bool);
    list<DFGNode*>* getMappedDFGNodes(DFG*, CGRANode*);
    map<int, CGRANode*>* getReorderPath(map<CGRANode*, int>*);
    bool DFSMap(CGRA*, DFG*, int, list<DFGNode*>*, list<map<CGRANode*, int>*>*, bool);

		/** This function reorder the paths according the cost
		 * TODO:just return the path with lowest cost may be a better choice
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_dfg : the pointer to the DFG
		 * @param t_II : the value of II
		 * @param t_dfgNode : the dfgNode need to be mapped
		 * @param t_paths: the posible paths from the fu which process previous DFGNode to the fu process t_dfgNode.
		 * @return : the reordered paths
		 */
		list<map<CGRANode*, int>*>* getOrderedPotentialPaths(CGRA* t_cgra,
    		DFG* t_dfg, int t_II, DFGNode* t_dfgNode, list<map<CGRANode*, int>*>* t_paths);

  public:
		/**The constructor function of class Mapper
		 * use the C++ default construct function
		 */
    Mapper(){}

		/**get the ResMII 
		 * ResMII is equal to the number of DFGNode divided by the number of FU in CGRA.
		 * This is the smallest possible value of II.
		 * @param t_dfg : the pointer to the DFG
		 * @param t_cgar : the pointer to the CGRA
		 * @return : the ResMII int type
		 */
		int getResMII(DFG* t_dfg, CGRA* t_cgra);

		/**get the RecMII 
		 * RecMII is equal to the max length of clcye(环) in DFG.
		 * this value is equal to 0, because our DFG doesn't have any cycles.this function may also be deleted later.
		 * @param t_dfg : the pointer to the DFG
		 * @return : the RecMII int type
		 */
		int getRecMII(DFG* t_dfg);

		/**The function to construct MRRG
		 * @param t_dfg : the pointer to the DFG
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_II : the value of II
		 */
		void constructMRRG(DFG* t_dfg, CGRA* t_cgra, int t_II);

		/**The function to do Mapping via heuristics
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_dfg : the pointer to the DFG
		 * @param t_II : the smallest possible value of II(now ResMII)
		 * @param t_isStaticElasticCGRA : TODO
		 * @return : TODO
		 */
		int heuristicMap(CGRA* t_cgra, DFG* t_dfg, int t_II, bool t_isStaticElasticCGRA);

    int exhaustiveMap(CGRA*, DFG*, int, bool);

		/**This function try to map t_dfgNode to t_fu, if successed, return the path, else return NULL
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_dfg : the pointer to the DFG
		 * @param t_II : the value of II
		 * @param t_dfgNode : DFGNode that hopes to be mapped to a CGRANode(t_fu).
		 * @param t_fu : The CGRANode that DFGNode(t_dfgNode) hopes to be mapped to.
		 * @return : path map<CGRANode*,int>,the "int" is clock cycle. the path record the path from the fu which process previous DFGNode to the fu process current DFGNode. if can't map t_dfgNode to t_fu, return NULL.
		 */
		map<CGRANode*, int>* calculateCost(CGRA* t_cgra, DFG* t_dfg, int t_II, DFGNode* t_dfgNode, CGRANode* t_fu);

		/** This function choose the best path from paths. use getOrderedPotentialPaths function to reorder the paths according the cost,then choose the path with lowest cost.
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_dfg : the pointer to the DFG
		 * @param t_II : the value of II
		 * @param t_dfgNode : the dfgNode need to be mapped
		 * @param t_paths: the posible paths from the fu which process previous DFGNode to the fu process t_dfgNode.
		 * @return : the best path
		 */
		map<CGRANode*, int>* getPathWithMinCostAndConstraints(CGRA* t_cgra,
    	DFG* t_dfg, int t_II, DFGNode* t_dfgNode, list<map<CGRANode*, int>*>* t_paths);

		/** TODO:
		 * @param t_caga : the pointer to the CGRA 
		 * @param t_dfg : the pointer to the DFG
		 * @param t_II : the value of II
		 * @param t_dfgNode : the dfgNode need to be mapped
		 * @param t_paths: the posible paths from the fu which process previous DFGNode to the fu process t_dfgNode.
		 * @param t_isStaticElasticCGRA: true mean is StaticElasticCGRA, now is always false.
		 * @return : the best path
		 */
		bool schedule(CGRA* t_cgra, DFG* t_dfg, int t_II, DFGNode* t_dfgNode, map<CGRANode*, int>* t_path, bool t_isStaticElasticCGRA);
    void showSchedule(CGRA*, DFG*, int, bool, bool);
    void generateJSON(CGRA*, DFG*, int, bool);
};
