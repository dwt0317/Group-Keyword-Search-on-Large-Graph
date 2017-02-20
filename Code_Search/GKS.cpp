#include <algorithm> //for sort invertList
#include <map> //for source to portal node
#include <windows.h> //for count time
#include<iostream>
#include <omp.h>
#include "GKS.h"
#include "VisitInfo.h"
#include "macro.h"
#include "Global.h"
#include <time.h>
#include <hash_map>


using namespace std;

extern int threadNum;
extern vector<Block*> *BlockArray; //store blocks
extern hash_map<int, int>** sRmtpDistMapArray;

int timecount3 = 0;
//opt result and rating
vector<int> **resultArray;

//temp result and rating
int TempRating = 0;
int OptRating = 0;

vector<int> **desireArray;

int rateCount = 0;
int rateTotal = 0;

#pragma omp threadprivate(TempRating, StartDist,timecount3) 

bool RatingComp(int A, int B) {return NodeRating[A] > NodeRating[B];}

//初始化搜索
void GKS_Query(int StartNode, vector<KeyWord> KSet, pair<int, int> Cuslimit, BlockPool& blockPool) {

	OptRating = 0;

	for (int time = 0; time < 1; ++time) {
		clock_t start1 = clock();
		vector<vector<int>> *invertList = new vector<vector<int>>(KSet.size(), vector<int>());
		resultArray = new vector<int>*[threadNum];
		desireArray = new vector<int>*[threadNum];
		for (int i = 0; i < threadNum; i++){
			resultArray[i] = new vector<int>();
			desireArray[i] = new vector<int>();
		}

		int count = 0;
		bool only = true;

		//起始点keyword是否符合`
		for (vector<KeyWord>::iterator itr = KSet.begin(); only && itr != KSet.end(); itr++, count++) {
			if (NodeKW[StartNode] ==*itr) { //relevant with KSet
				(*invertList)[count].push_back(StartNode);
				only = false;
			}
		}

		bool *visited = new bool[MAX_NODE_NUM];
		memset(visited, false, sizeof(bool)*MAX_NODE_NUM);
		visited[StartNode] = true;
		SearchFirstDist(StartNode, invertList, visited, KSet, Cuslimit.first);
		delete [] visited;

		bool cover = true;
		for (vector<vector<int>>::iterator itrK = invertList->begin(); cover && itrK != invertList->end(); itrK++) {
			if (itrK->empty()) {
				cover = false;
			}
		}

		if (cover) {
			//sort invertList with rate
			for (vector<vector<int>>::iterator itr = invertList->begin(); itr != invertList->end(); itr++) {
				sort(itr->begin(), itr->end(), RatingComp);
			}
		    AssginTask(invertList, Cuslimit.second, KSet);
		}
		for (int i = 0; i < threadNum; i++){
			vector<int>().swap(*(resultArray[i]));
			vector<int>().swap(*(desireArray[i]));
		}
		delete []resultArray;
		delete []desireArray;
		vector<vector<int>>().swap(*invertList);
	}
}

//寻找满足第一距离约束的点集合
void SearchFirstDist(int StartNode, vector<vector<int>> *invertList, bool *visited, vector<KeyWord>& KSet, int Farthest) {

	int IntraID = 0;  	//find intra block's nodeid

	//for each startnode's blockid
	int blockcount = (*BlockID)[StartNode].size();

	//deal with start block info,startNode所在的block直接搜索

	doStartBlock(StartNode, invertList, visited, KSet, Farthest);


	//这样没错，先查找start block,对于start block中的点来说有些点可能不通过portal也能到达
	//hash_map<int, int>* sRmtpDistMap = new hash_map<int, int>();
	hash_map<int, int>* sRmtpDistMap = sRmtpDistMapArray[0];
	if (blockcount != 1){
		//searchPortalMatrix(StartNode, StartNode, 0, Farthest, invertlistArray, visited, visitedArray, sRmtpDistArrayS, KSet, blockPool);
		searchPortalMatrix(StartNode, StartNode, 0, Farthest, invertList, visited, sRmtpDistMap, KSet);
	}
	else{
		for (int BCount = 0; BCount < blockcount; BCount++) {
			//find its intra block index，获取block id
			int blockID = (*BlockID)[StartNode][BCount];
			//Block* curBlock = blockPool.findBlock(blockID);
			Block* curBlock = (*BlockArray)[blockID];

			vector<int> *BlockNodes = curBlock->mNodeList;
			int end = BlockNodes->size();
			for (int i = 0; i < end; i++) {
				if (StartNode == (*BlockNodes)[i])
					IntraID = i;
			}

			//遍历区域中每一个portal node
			int pNodePos = curBlock->mPNodePos;
			int endPortal = end - pNodePos;
			//clock_t start6 = clock();
			//#pragma omp parallel for
			for (int portal = 0; portal < endPortal; portal++) {
				int spdist = curBlock->NodeToPortal[IntraID][portal].Dist;
				if (spdist) { //use portal node relative position in IntraIndex				
					//通过relative id 找 global id, mNodeList的下标为内部id,值为global id
					int pnode = curBlock->NodeToPortal[IntraID][portal].NodeID;//relative id   NodeToPortal是排过序的，和portalList中的顺序不一样
					pnode = (*BlockNodes)[pnode]; //global id
					int threadId = omp_get_thread_num();
					if (spdist < Farthest) {
						//searchPortalMatrix(StartNode, pnode, spdist, Farthest, invertlistArray, visited, visitedArray, sRmtpDistArrayS, KSet, blockpoolArray[threadId]);
						searchPortalMatrix(StartNode, pnode, spdist, Farthest, invertList, visited, sRmtpDistMap, KSet);
					}
					//这个else break直接少了1/3的计算量，nodeToPortal是sort过的
					else
						break;
				}//if (pdist)
			}//foreach portal
		}
	}
	sRmtpDistMapArray[0]->clear();
	//delete sRmtpDistMap;

}

//搜索portal node矩阵
void searchPortalMatrix(int startNode, int pnode, int spDist, int Farthest, vector<vector<int>> *invertList, bool *startVisited, hash_map<int, int>* sRmtpDistMap,
	vector<KeyWord>& KSet){

	hash_map<int, int>::iterator h_it;
	int portalMtrxId = 0, portalGlobalId = 0;
	int sRmtpDist = 0; //start -> remote portal distance
	portalMtrxId = portalMtrxIndex[pnode];     //startPortal的matrix id

	int threadId = omp_get_thread_num();

	for (int i = 1; i < portalDistIndex[portalMtrxId][0].NodeID; i++){
		bool feasiblePortal = true;
		int portalDist = portalDistIndex[portalMtrxId][i].Dist;
		portalGlobalId = portalDistIndex[portalMtrxId][i].NodeID;

		if ((portalDist + spDist <= Farthest && portalDist != 0) || pnode == portalGlobalId)  //portalIndex
		{
			sRmtpDist = portalDist + spDist;
			h_it = sRmtpDistMap->find(portalGlobalId);
			if (h_it != sRmtpDistMap->end()){
				int lastDist = h_it->second;
				if (sRmtpDist >= lastDist){
					feasiblePortal = false;
				}
				else{
					h_it->second = sRmtpDist;
				}
			}
			else
				sRmtpDistMap->insert(pair<int, int>(portalGlobalId, sRmtpDist));

			if (!feasiblePortal)
				continue;

			int blockcount2 = (*BlockID)[portalGlobalId].size();
			for (int BCount = 0; BCount < blockcount2; BCount++) {
				int blockID = (*BlockID)[portalGlobalId][BCount];
				int startbcount = (*BlockID)[startNode].size();
			
				Block* curBlock = (*BlockArray)[blockID];
				vector<int> *BlockNodes = (curBlock->mNodeList);
				int end = BlockNodes->size();
				int IntraID = 0;
				for (int i = 0; i < end; i++) {
					if (portalGlobalId == (*BlockNodes)[i]){
						IntraID = i;
						break;
					}					
				}
				//search intra block index
				int **IntraIndex = curBlock->InSDist;
				for (int node = 0; node < end; node++) {  
					int RealID = (*BlockNodes)[node];
					if (!startVisited[RealID]){  
						if (IntraIndex[IntraID][node] && IntraIndex[IntraID][node] + sRmtpDist <= Farthest && !startVisited[RealID]){
							int count = 0; bool only = true;
							for (vector<KeyWord>::iterator itr = KSet.begin(); only && itr != KSet.end(); itr++, count++) {
								if (NodeKW[RealID] ==*itr) {
									(*invertList)[count].push_back(RealID);
									only = false;
								}
							}
							startVisited[RealID] = true;
						}
					}
				}

			}

		}
		else if (portalDist + spDist > Farthest)
			break;
	}
}

//搜索起始block
void doStartBlock(int StartNode, vector<vector<int>> *invertList, bool *startVisit, vector<KeyWord>& KSet, int Farthest){
	int blockcount2 = (*BlockID)[StartNode].size();

	for (int BCount = 0; BCount < blockcount2; BCount++) {
		//find its intra block index，获取block id
		int blockID = (*BlockID)[StartNode][BCount];
		//Block* curBlock = blockPool.findBlock(blockID);
		Block* curBlock = (*BlockArray)[blockID];
		vector<int> *BlockNodes = (curBlock->mNodeList);
		int end = BlockNodes->size();
		int IntraID = 0;
		for (int i = 0; i < end; i++) {
			if (StartNode == (*BlockNodes)[i])
				IntraID = i;
		}
		//search intra block index
		int **IntraIndex = curBlock->InSDist;
		for (int node = 0; node < end; node++) {
			int RealID = (*BlockNodes)[node];
			if (IntraIndex[IntraID][node] && IntraIndex[IntraID][node] <= Farthest && !startVisit[RealID]) {

				int count = 0; bool only = true;
				for (vector<KeyWord>::iterator itr = KSet.begin(); only && itr != KSet.end(); itr++, count++) {
					if (NodeKW[RealID] ==*itr) {
						(*invertList)[count].push_back(RealID);
						only = false;
						startVisit[RealID] = true;
					}
				}
				
			}
		}
	}
}

//按照预估的要搜索的block数量为每个线程尽量平均分配任务
void AssginTask(vector<vector<int>> *invertList, int PairDist, vector<KeyWord>& KSet){
	int taskArraySize = threadNum;
	vector<vector<int>>** task = new vector<vector<int>> *[threadNum];
	VisitInfo** VisitInfoArray = new VisitInfo *[threadNum];
	for (int i = 0; i < threadNum; i++){
		task[i] = new vector<vector<int>>(KSet.size(), vector<int>());
	}

	int taskNum = (*invertList)[0].size();
	int taskSize = ceil(taskNum / threadNum);

	if (taskNum <= threadNum){
		for (int i = 0; i < taskNum; i++){
			(*(task[i]))[0].push_back((*invertList)[0][i]);
		}
		taskArraySize = taskNum;
	}
	else{
		for (int i = 0; i < taskNum; ){
			for (int j = 0; j < threadNum; j++){
				if (i < taskNum){
					(*(task[j]))[0].push_back((*invertList)[0][i]);
					i++;
				}
				else
					break;
			}
		}
	}

	int m = 1;
	for (vector<vector<int>>::iterator outIt = invertList->begin() + 1; outIt != invertList->end(); outIt++){
		for (vector<int>::iterator inIt = outIt->begin(); inIt != outIt->end(); inIt++){
			for (int i = 0; i < taskArraySize; i++){
				(*(task[i]))[m].push_back(*inIt);
			}
		}
		m++;
	}

	vector<int>** blockToSearch = new vector<int>* [KSet.size()];
	for (int i = 0; i < KSet.size(); i++){
		blockToSearch[i] = new vector<int>();
	}
	int keyPos = 1;
	int startLevel = 0;
	for (int i = 1; i < KSet.size(); i++){
		vector<vector<int>>* newInvert = new vector<vector<int>>(KSet.size());
		copy(invertList->begin() + startLevel+1, invertList->end(), newInvert->begin());
		GetBlockToSearch(newInvert, blockToSearch[startLevel]);
		startLevel++;
		vector<vector<int>>().swap(*newInvert);
	}
	bool **visitedArray = new bool*[threadNum];
	#pragma omp parallel for schedule(static,1)

	for (int i = 0; i < taskArraySize; i++){

		visitedArray[i] = new bool[MAX_NODE_NUM];
		memset(visitedArray[i], false, sizeof(bool)*MAX_NODE_NUM);
		VisitInfoArray[i] = new VisitInfo(KSet.size(), task[i]);
		SearchSecondDist(1, KSet.size(), PairDist, VisitInfoArray[i], KSet, visitedArray[i], blockToSearch);
		delete[] visitedArray[i];
	}

	for (int i = 0; i < KSet.size(); i++){
		vector<int>().swap(*blockToSearch[i]);
	}
	for (int i = 0; i < threadNum; i++){
		vector<vector<int>>().swap(*(task[i]));
	}
	delete blockToSearch;
	delete[]task;
	delete[]VisitInfoArray;

}

//寻找满足第二距离约束的点集合
void SearchSecondDist(int curlevel, int lastlevel, int PairDist, VisitInfo *CliqueList, vector<KeyWord>& KSet, bool *visited, vector<int>** blockToSearch) {

	int threadId = omp_get_thread_num();
	if (curlevel != lastlevel) {
		int CurStartLevel = CliqueList->StartLevel[curlevel];

		//get relative candidate list,比如需要在第三个关键词集合中找，不一定是全部的点都再访问一遍，可能只访问f[1,3]或f[2,3]
		vector<int> *CandList = CliqueList->f[CurStartLevel][curlevel];

		int RatingUpper = 0;

		for (int kpos = curlevel + 1; kpos <= lastlevel; kpos++) {
			RatingUpper += NodeRating[(*CliqueList->f[curlevel - 1][kpos])[0]];
		}
		int RatingLower = 0;
#pragma omp critical
		{
			RatingLower = OptRating - RatingUpper;
		}
		bool RatingPruning = false;

		for (vector<int>::iterator itrClist = CandList->begin(); !RatingPruning && itrClist != CandList->end(); itrClist++) {
			int dnode = *itrClist;

			int curnodeid = *itrClist;

			if (TempRating + NodeRating[*itrClist] > RatingLower) {

				vector<vector<int>> *NewCliqueList = new vector<vector<int>>(lastlevel - curlevel, vector<int>());

				visited[*itrClist] = true;

				SearchFirstDist_2(*itrClist, NewCliqueList, visited, vector<KeyWord>(KSet.begin() + curlevel, KSet.end()), PairDist, blockToSearch[curlevel-1]);
				memset(visited, false, sizeof(bool)*MAX_NODE_NUM);

				bool ContinueSearch = true;
				//通过newCliquelist可以初始化此关键字之后的f向量，1关键字找出来的newlist可以初始化所有f(1,*),2关键字则是f(2,*)
				for (int IntersectCL = curlevel + 1, IntersectNewCL = 0; ContinueSearch && IntersectCL <= lastlevel; IntersectCL++, IntersectNewCL++) {

					vector<int> temp;

					for (vector<int>::iterator CL = CliqueList->f[curlevel - 1][IntersectCL]->begin();
						CL != CliqueList->f[curlevel - 1][IntersectCL]->end(); ++CL) {

						bool stop = false;

						for (vector<int>::iterator NewCL = (*NewCliqueList)[IntersectNewCL].begin();   //看new中是否存在*CL
							!stop && NewCL != (*NewCliqueList)[IntersectNewCL].end(); NewCL++) {

							if (*CL == *NewCL) {
								temp.push_back(*CL);
								stop = true;
							}
						}
					}

					if (!temp.empty()) {   //temp中存new与f的交集
						*(CliqueList->f[curlevel][IntersectCL]) = temp;
						//set start level,有重合的点说明之后的那个f[curlevel][IntersectCL]可以作为IntersectCL-1关键词的查询备选点
						CliqueList->StartLevel[IntersectCL] = curlevel;  //标记每个level关键字查询时的初始备选点集合，就像最开始的invertlist,
					}
					else {
						ContinueSearch = false;   //是否搜索candist中的下一个node, 即是否有结果
						break; 
					}
				}

				vector<vector<int>>().swap(*NewCliqueList);
				delete NewCliqueList;

				if (ContinueSearch) {
					int ddnode = *itrClist;
					TempRating += NodeRating[*itrClist];
					desireArray[threadId]->push_back(*itrClist);
					SearchSecondDist(curlevel + 1, lastlevel, PairDist, CliqueList, KSet, visited, blockToSearch);
					TempRating -= NodeRating[*itrClist];

					desireArray[threadId]->pop_back();
					for (int level = curlevel + 1; level <= lastlevel; level++) {
						if (CliqueList->StartLevel[level] >= curlevel) //如果f(2,3)有值，但现在2关键字换了，所以能用的只能是f(1,3)
							--(CliqueList->StartLevel[level]);
					}
				}
			}
		}
	}
	else { //last keyword
		vector<int> *LastCandList = CliqueList->f[lastlevel - 1][lastlevel];
		int Rate = TempRating + NodeRating[(*LastCandList)[0]]; //按得分排序的，只拿第一个 
#pragma omp critical 
		{
			if (Rate > OptRating) {
				OptRating = Rate;
				*resultArray[threadId] = *desireArray[threadId];
				resultArray[threadId]->push_back((*LastCandList)[0]);
			}
		}
	}
}


//在SearchSecondDist内循环中使用的特定函数
void SearchFirstDist_2(int StartNode, vector<vector<int>> *invertList, bool *visited, vector<KeyWord>& KSet, int Farthest, vector<int>* blockToSearch) {

	int IntraID = 0;  	//find intra block's nodeid
	int threadId = omp_get_thread_num();
	int blockcount = (*BlockID)[StartNode].size();

	//deal with start block info,startNode所在的block直接搜索

	doStartBlock_2(StartNode, invertList, visited, KSet, Farthest,blockToSearch);

	hash_map<int, int>* sRmtpDistMap = sRmtpDistMapArray[threadId];

	if (blockcount != 1){
		searchPortalMatrix_2(StartNode, StartNode, 0, Farthest, invertList, visited, sRmtpDistMap, KSet, blockToSearch);
	}
	else{
		for (int BCount = 0; BCount < blockcount; BCount++) {
			int blockID = (*BlockID)[StartNode][BCount];
			Block* curBlock = (*BlockArray)[blockID];

			vector<int> *BlockNodes = curBlock->mNodeList;
			int end = BlockNodes->size();
			for (int i = 0; i < end; i++) {
				if (StartNode == (*BlockNodes)[i])
					IntraID = i;
			}

			//遍历区域中每一个portal node
			int pNodePos = curBlock->mPNodePos;
			int endPortal = end - pNodePos;

			for (int portal = 0; portal < endPortal; portal++) {
				int spdist = curBlock->NodeToPortal[IntraID][portal].Dist;
				if (spdist) { 			
					//通过relative id 找 global id, mNodeList的下标为内部id,值为global id
					int pnode = curBlock->NodeToPortal[IntraID][portal].NodeID;//relative id   NodeToPortal是排过序的
					pnode = (*BlockNodes)[pnode]; //global id
					if (spdist < Farthest) {
						searchPortalMatrix_2(StartNode, pnode, spdist, Farthest, invertList, visited, sRmtpDistMap, KSet, blockToSearch);
					}
					else
						break;
				}//if (pdist)
			}//foreach portal
		}
	}
	sRmtpDistMapArray[threadId]->clear();
}

//在SearchSecondDist内循环中使用的特定函数
void doStartBlock_2(int StartNode, vector<vector<int>> *invertList, bool *startVisit, vector<KeyWord>& KSet, int Farthest, vector<int>* blockToSearch){
	int blockcount2 = (*BlockID)[StartNode].size();
	for (int BCount = 0; BCount < blockcount2; BCount++) {
		//find its intra block index，获取block id
		int blockID = (*BlockID)[StartNode][BCount];
		bool feasibleBlock = false;
		Block* curBlock = (*BlockArray)[blockID];

		vector<int> *BlockNodes = (curBlock->mNodeList);
		int end = BlockNodes->size();
		int IntraID = 0;
		for (int i = 0; i < end; i++) {
			if (StartNode == (*BlockNodes)[i])
				IntraID = i;
		}
		//search intra block index
		int **IntraIndex = curBlock->InSDist;
		for (int node = 0; node < end; node++) {
			int RealID = (*BlockNodes)[node];
			int ddist = IntraIndex[IntraID][node];
			bool dbool = startVisit[RealID];
			int lllll = 0;
			if (IntraIndex[IntraID][node] && IntraIndex[IntraID][node] <= Farthest && !startVisit[RealID]) {

				int count = 0; bool only = true;
				for (vector<KeyWord>::iterator itr = KSet.begin(); only && itr != KSet.end(); itr++, count++) {
					if (NodeKW[RealID] ==*itr) {
						(*invertList)[count].push_back(RealID);
						only = false;
						startVisit[RealID] = true;
					}
				}			
			}
		}
	}
}

//在SearchSecondDist内循环中使用的特定函数
void searchPortalMatrix_2(int startNode, int pnode, int spDist, int Farthest, vector<vector<int>> *invertList, bool *visited, hash_map<int, int>* sRmtpDistMap, vector<KeyWord>& KSet,
	vector<int>* blockToSearch){
	hash_map<int, int>::iterator h_it;
	int portalMtrxId = 0, portalGlobalId = 0;
	int sRmtpDist = 0; //start -> remote portal distance
	portalMtrxId = portalMtrxIndex[pnode];     //startPortal的matrix id
	int blockNum = blockToSearch->size();
	int threadId = omp_get_thread_num();
	
	for (int i = 1; i < portalDistIndex[portalMtrxId][0].NodeID; i++){
		int portalDist = portalDistIndex[portalMtrxId][i].Dist;
		portalGlobalId = portalDistIndex[portalMtrxId][i].NodeID;
		bool feasiblePortal = true;
		if ((portalDist + spDist <= Farthest && portalDist != 0) || pnode == portalGlobalId)  //portalIndex
		{
			sRmtpDist = portalDist + spDist;
			h_it = sRmtpDistMap->find(portalGlobalId);
			if (h_it != sRmtpDistMap->end()){
				int lastDist = h_it->second;
				if (sRmtpDist >= lastDist){
					feasiblePortal = false;
				}
				else{
					h_it->second = sRmtpDist;
				}
			}
			else
				sRmtpDistMap->insert(pair<int, int>(portalGlobalId, sRmtpDist));
			if (!feasiblePortal)
				continue;

			for (int i = 0; i < blockNum; i++){
				int blockId = (*blockToSearch)[i];

				int pblockCount = (*BlockID)[portalGlobalId].size();
				for (int j = 0; j < pblockCount; j++){
					int pblockId = (*BlockID)[portalGlobalId][j];
					int startbcount = (*BlockID)[startNode].size();
					if (pblockId != blockId)   //判断是否是要找的block
						continue;

					bool feasibleBlock = false;

					Block* curBlock = (*BlockArray)[pblockId];

					vector<int> *BlockNodes = (curBlock->mNodeList);
					int end = BlockNodes->size();
					int IntraID = 0;
					for (int i = 0; i < end; i++) {
						if (portalGlobalId == (*BlockNodes)[i]){
							IntraID = i;
							break;
						}							
					}
					//search intra block index
					int **IntraIndex = curBlock->InSDist;

					for (int node = 0; node < end; node++) {  
						int RealID = (*BlockNodes)[node];
						if (visited[RealID])
							continue;
						if (IntraIndex[IntraID][node] && IntraIndex[IntraID][node] + sRmtpDist <= Farthest ){
							int count = 0; bool only = true;
							for (vector<KeyWord>::iterator itr = KSet.begin(); only && itr != KSet.end(); itr++, count++) {
								if (NodeKW[RealID] ==*itr) {
									int ddist = IntraIndex[IntraID][node];
									(*invertList)[count].push_back(RealID);
									only = false;
								}
							}
							visited[RealID] = true;
						}
					}			
				}
			}

		}
		else if (portalDist + spDist > Farthest)
			break;
	}

}

//初步获取要搜索的block的数量
void GetBlockToSearch(vector<vector<int>>* ilist, vector<int> *inblockToSearch){
	for (vector<vector<int>>::iterator outIt = ilist->begin(); outIt != ilist->end(); outIt++){
		for (vector<int>::iterator it = outIt->begin(); it != outIt->end(); it++){
			int blockCount = (*BlockID)[*it].size();
			for (int i = 0; i < blockCount; i++){
				bool exist = false;
				int blockId = (*BlockID)[*it][i];
				for (vector<int>::iterator it = inblockToSearch->begin(); it != inblockToSearch->end(); it++){
					if (*it == blockId){
						exist = true;
						break;
					}
				}
				if (!exist){
					inblockToSearch->push_back(blockId);
				}
			}
		}
	}
}

//输出结果
void OutPutResultToFile(char filename[], int Rate) {
	FILE *file = fopen(filename, "a+");

	if (file == NULL)
		return;

	fprintf(file, "%d: %d\n",MAX_BLOCK_NODE, Rate);

	fclose(file);
}

