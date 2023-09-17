/**
 * @file mapperPass.cpp
 * @author Cheng Tan and Chao Zhang
 * @brief the top file of mapper
 * @version 0.1
 */ 

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopIterator.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <set>
#include "json.hpp"
#include "Mapper.h"

using namespace llvm;
using namespace std;
using json = nlohmann::json;

void addDefaultKernels(map<string, list<int>*>*);

namespace {

  struct mapperPass : public FunctionPass {

  public:
    static char ID;
    Mapper* mapper;
    mapperPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addPreserved<LoopInfoWrapperPass>();
      AU.setPreservesAll();
    }

		/**
		 * Mapper enter at this function
		 */
    bool runOnFunction(Function &t_F) override {

      // Initializes input parameters.
      int rows                      = 4;
      int columns                   = 4;
      bool targetEntireFunction     = false;
      bool targetNested             = true;
      bool doCGRAMapping            = true;
      bool isStaticElasticCGRA      = false;
      bool isTrimmedDemo            = true;
      int ctrlMemConstraint         = 200;
      int bypassConstraint          = 4;
      int regConstraint             = 8;
      bool precisionAware           = false;
      bool diagonalVectorization    = false;
      bool heuristicMapping         = true;
      bool parameterizableCGRA      = false;
      map<string, int>* execLatency = new map<string, int>();	//lantancy of operations
      list<string>* pipelinedOpt    = new list<string>();	//operations support pipeline
      map<string, list<int>*>* additionalFunc = new map<string, list<int>*>(); //TODO:

      // Set the target function and loop.
      map<string, list<int>*>* functionWithLoop = new map<string, list<int>*>();//record the functionwithLoop .map<name of function,pointer of the loop_number's list>
      addDefaultKernels(functionWithLoop);//not important

      // Read the parameter JSON file.
      ifstream i("./param.json");
      if (!i.good()) {

        cout<< "=============================================================\n";
        cout<<"\033[0;31mPlease provide a valid <param.json> in the current directory."<<endl;
        cout<<"A set of default parameters is leveraged.\033[0m"<<endl;
        cout<< "=============================================================\n";
      } else {
        json param;
        i >> param;
 
	// Check param exist or not. the name of param bellow must be include in param.json ,this will be checked in bellow try block
	set<string> paramKeys;
	paramKeys.insert("row");
	paramKeys.insert("column");
	paramKeys.insert("targetFunction");
	paramKeys.insert("kernel");
	paramKeys.insert("targetNested");
	paramKeys.insert("isTrimmedDemo");
	paramKeys.insert("doCGRAMapping");
	paramKeys.insert("isStaticElasticCGRA");
	paramKeys.insert("ctrlMemConstraint");
	paramKeys.insert("bypassConstraint");
	paramKeys.insert("regConstraint");
	paramKeys.insert("precisionAware");
	paramKeys.insert("diagonalVectorization");
	paramKeys.insert("heuristicMapping");
	paramKeys.insert("parameterizableCGRA");

	try
        {
          // try to access a nonexisting key
          for (auto itr : paramKeys)
          {
            param.at(itr);
          }
        }
        catch (json::out_of_range& e)
        {
          cout<<"Please include related parameter in param.json: "<<e.what()<<endl;
	  exit(0);
        }
	//finished the param.json checking

        // Configuration for customizable CGRA.(assign date from json to variables)
				//1.assign value to functionWithLoop with kernel and targetLoopsID read from json file
        (*functionWithLoop)[param["kernel"]] = new list<int>();
        (*functionWithLoop)[param["kernel"]]->push_back(0);
				//2. assign value to normal variable
        rows                  = param["row"];
        columns               = param["column"];
        targetEntireFunction  = param["targetFunction"];
        targetNested          = param["targetNested"];
        doCGRAMapping         = param["doCGRAMapping"];
        isStaticElasticCGRA   = param["isStaticElasticCGRA"];
        isTrimmedDemo         = param["isTrimmedDemo"];
        ctrlMemConstraint     = param["ctrlMemConstraint"];
        bypassConstraint      = param["bypassConstraint"];
        regConstraint         = param["regConstraint"];
        precisionAware        = param["precisionAware"];
        diagonalVectorization = param["diagonalVectorization"];
        heuristicMapping      = param["heuristicMapping"];
        parameterizableCGRA   = param["parameterizableCGRA"];
				//3. assign value to execLatency , piplinedOpt,additionalFunc
        for (auto& opt : param["optLatency"].items()) {
          cout<<opt.key()<<" : "<<opt.value()<<endl;
          (*execLatency)[opt.key()] = opt.value();
        }
        json pipeOpt = param["optPipelined"];
        for (int i=0; i<pipeOpt.size(); ++i) {
          pipelinedOpt->push_back(pipeOpt[i]);
        }
        for (auto& opt : param["additionalFunc"].items()) {
          (*additionalFunc)[opt.key()] = new list<int>();
          cout<<opt.key()<<" : "<<opt.value()<<": ";
          for (int i=0; i<opt.value().size(); ++i) {
            (*additionalFunc)[opt.key()]->push_back(opt.value()[i]);
            cout<<opt.value()[i]<<" ";
          }
          cout<<endl;
        }
      }
			//finished read data from param.json

      // Check existance. if the name of kernel read from param.json is in input .bc 
      if (functionWithLoop->find(t_F.getName().str()) == functionWithLoop->end()) {
        cout<<"[function \'"<<t_F.getName().str()<<"\' is not in our target list]\n";
        return false;
      }
      cout << "==================================\n";
      cout<<"[function \'"<<t_F.getName().str()<<"\' is one of our targets]\n";

      list<Loop*>* targetLoops = getTargetLoops(t_F, functionWithLoop, targetNested);
      // TODO: will make a list of patterns/tiles to illustrate how the
      //       heterogeneity is
      DFG* dfg = new DFG(t_F, targetLoops, targetEntireFunction, precisionAware,
                         execLatency, pipelinedOpt);
      CGRA* cgra = new CGRA(rows, columns, diagonalVectorization,
		            parameterizableCGRA, additionalFunc);
      cgra->setRegConstraint(regConstraint);
      cgra->setCtrlMemConstraint(ctrlMemConstraint);
      cgra->setBypassConstraint(bypassConstraint);
      mapper = new Mapper();

      // Show the count of different opcodes (IRs).
      cout << "==================================\n";
      cout << "[show opcode count]\n";
      dfg->showOpcodeDistribution();

      // Generate the DFG dot file.
      cout << "==================================\n";
      cout << "[generate dot for DFG]\n";
      dfg->generateDot(t_F, isTrimmedDemo);

      // Generate the DFG dot file.
      cout << "==================================\n";
      cout << "[generate JSON for DFG]\n";
      dfg->generateJSON();

      // Initialize the II.
      int ResMII = mapper->getResMII(dfg, cgra);
      cout << "==================================\n";
      cout << "[ResMII: " << ResMII << "]\n";
      int RecMII = mapper->getRecMII(dfg);
      cout << "==================================\n";
      cout << "[RecMII: " << RecMII << "]\n";
      int II = ResMII;
      if(II < RecMII)
        II = RecMII;

      if (!doCGRAMapping) {
        cout << "==================================\n";
        return false;
      }
      // Heuristic algorithm (hill climbing) to get a valid mapping within
      // a acceptable II.
      bool success = false;
      if (!isStaticElasticCGRA) {
        cout << "==================================\n";
        if (heuristicMapping) {
          cout << "[heuristic]\n";
          II = mapper->heuristicMap(cgra, dfg, II, isStaticElasticCGRA);
        } else {
          cout << "[exhaustive]\n";
          II = mapper->exhaustiveMap(cgra, dfg, II, isStaticElasticCGRA);
        }
      }

      // Partially exhaustive search to try to map the DFG onto
      // the static elastic CGRA.

      if (isStaticElasticCGRA and !success) {
        cout << "==================================\n";
        cout << "[exhaustive]\n";
        II = mapper->exhaustiveMap(cgra, dfg, II, isStaticElasticCGRA);
      }

      // Show the mapping and routing results with JSON output.
      if (II == -1)
        cout << "[fail]\n";
      else {
        mapper->showSchedule(cgra, dfg, II, isStaticElasticCGRA, parameterizableCGRA);
        cout << "==================================\n";
        cout << "[Mapping Success]\n";
        cout << "==================================\n";
        mapper->generateJSON(cgra, dfg, II, isStaticElasticCGRA);
        cout << "[Output Json]\n";
      }
      cout << "=================================="<<endl;

      return false;
    }
		//end of runOnFunction

/**
 * get the targetLoop,it will be the outmost loop,or innermost loop
 * @param t_F function information of bc file
 * @param t_functionWithLoop information of target function and loops
 * @param t_targetNested if this param is True,when face the nested loop,the target loop is outmost loop,else the target loop is the innermost loop
 * @return return the list of targetLoops,now it will just be one loop.and will pass to the construction fuction of DFG,as a scope of the inst in IR.
 */
    list<Loop*>* getTargetLoops(Function& t_F, map<string, list<int>*>* t_functionWithLoop, bool t_targetNested) {
      list<Loop*>* targetLoops = new list<Loop*>();
      // Since the ordering of the target loop id could be random, I use O(n^2) to search the target loop.
			// in this while loop ,target loopID of target function is poped from t_functionWithLoop's loopID list
      if((*t_functionWithLoop).at(t_F.getName().str())->size() > 0) {
        LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        Loop* current_loop = NULL;
        for(LoopInfo::iterator loopItr=LI.begin();
            loopItr!= LI.end(); ++loopItr) {
          // targetLoops->push_back(*loopItr);
          current_loop = *loopItr;
          if (current_loop->getParentLoop()==nullptr) {
//debug begin print all the header of loops .need targetNested param is set false
    			BasicBlock *curBB = current_loop->getHeader();
      for (BasicBlock::iterator II=curBB->begin(),
          IEnd=curBB->end(); II!=IEnd; ++II) {
        Instruction* inst = &*II;
          errs()<<" inst in header: "<<*inst<<"\n";
			}
         errs()<<"\n";
//debug end
            // Targets innermost loop if the param targetNested is not set.
            if (!t_targetNested) {
              while (!current_loop->getSubLoops().empty()) {
                errs()<<"[explore] nested loop ... subloop size: "<<current_loop->getSubLoops().size()<<"\n";
                // TODO: might change '0' to a reasonable index
                current_loop = current_loop->getSubLoops()[0];
//debug begin print all the header of loops .need targetNested param is set false
    			BasicBlock *curBB = current_loop->getHeader();
      for (BasicBlock::iterator II=curBB->begin(),
          IEnd=curBB->end(); II!=IEnd; ++II) {
        Instruction* inst = &*II;
          errs()<<" inst in header: "<<*inst<<"\n";
			}
         errs()<<"\n";
//debug end
              }
            }
            targetLoops->push_back(current_loop);
            errs()<<"*** reach target loop <<\n";
            //break;
          }
        }
        if (targetLoops->size() == 0) {
          errs()<<"... no loop detected in the target kernel ...\n";
        }
      }
      errs()<<"... done detected loops.size(): "<<targetLoops->size()<<"\n";
      return targetLoops;
    }//end of getTargetLoops
  };//end of FucntionPass class
}//end of namespace

char mapperPass::ID = 0;
static RegisterPass<mapperPass> X("mapperPass", "DFG Pass Analyse", false, false);

/**
 * Add the kernel names of some popular applications.Assume each kernel contains single loop.
 * @param t_functionWithLoop the pointer to data structure used to record the name of functions that contains loops and the labels of loops
 * 
 * the Name of kernel function need be add to functionWithLoop first.We can find kernel's name in kernel.ll,Actually we just need to add one kernel's name,but we need to test different kernels,so we add them here ahead of time. A better implementation is to pass the name of the kernel as a parameter.TODO
 */
void addDefaultKernels(map<string, list<int>*>* t_functionWithLoop) {

  (*t_functionWithLoop)["_Z12ARENA_kerneliii"] = new list<int>();
  (*t_functionWithLoop)["_Z12ARENA_kerneliii"]->push_back(0);
  (*t_functionWithLoop)["_Z4spmviiPiS_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z4spmviiPiS_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z4spmvPiii"] = new list<int>();
  (*t_functionWithLoop)["_Z4spmvPiii"]->push_back(0);
  (*t_functionWithLoop)["adpcm_coder"] = new list<int>();
  (*t_functionWithLoop)["adpcm_coder"]->push_back(0);
  (*t_functionWithLoop)["adpcm_decoder"] = new list<int>();
  (*t_functionWithLoop)["adpcm_decoder"]->push_back(0);
  (*t_functionWithLoop)["kernel_gemm"] = new list<int>();
  (*t_functionWithLoop)["kernel_gemm"]->push_back(0);
  (*t_functionWithLoop)["kernel"] = new list<int>();
  (*t_functionWithLoop)["kernel"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelPfS_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPfS_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z6kerneliPPiS_S_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kerneliPPiS_S_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelPPii"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPii"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBType"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBType"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBTypePi"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBTypePi"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBTypeP4Vect"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBTypeP4Vect"]->push_back(0);
  (*t_functionWithLoop)["fir"] = new list<int>();
  (*t_functionWithLoop)["fir"]->push_back(0);
  (*t_functionWithLoop)["spmv"] = new list<int>();
  (*t_functionWithLoop)["spmv"]->push_back(0);
  // (*functionWithLoop)["fir"].push_back(1);
  (*t_functionWithLoop)["latnrm"] = new list<int>();
  (*t_functionWithLoop)["latnrm"]->push_back(1);
  (*t_functionWithLoop)["fft"] = new list<int>();
  (*t_functionWithLoop)["fft"]->push_back(0);
  (*t_functionWithLoop)["BF_encrypt"] = new list<int>();
  (*t_functionWithLoop)["BF_encrypt"]->push_back(0);
  (*t_functionWithLoop)["susan_smoothing"] = new list<int>();
  (*t_functionWithLoop)["susan_smoothing"]->push_back(0);

  (*t_functionWithLoop)["_Z9LUPSolve0PPdPiS_iS_"] = new list<int>();
  (*t_functionWithLoop)["_Z9LUPSolve0PPdPiS_iS_"]->push_back(0);

  // For LU:
  // init
  (*t_functionWithLoop)["_Z6kernelPPdidPi"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdidPi"]->push_back(0);

  // solver0 & solver1
  (*t_functionWithLoop)["_Z6kernelPPdPiS_iS_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPiS_iS_"]->push_back(0);

  // determinant
  (*t_functionWithLoop)["_Z6kernelPPdPii"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPii"]->push_back(0);

  // invert
  (*t_functionWithLoop)["_Z6kernelPPdPiiS0_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPiiS0_"]->push_back(0);

  // nested
  // (*t_functionWithLoop)["_Z6kernelPfS_S_"] = new list<int>();
  // (*t_functionWithLoop)["_Z6kernelPfS_S_"]->push_back(0);
}
