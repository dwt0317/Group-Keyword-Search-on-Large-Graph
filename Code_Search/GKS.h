#ifndef _GKS_H
#define _GKS_H

#include <vector>
#include <hash_map>
#include <map>
#include "ExtraInfo.h"
#include "VisitInfo.h"
#include "BlockPool.h"
using namespace std;

//Group keyword search º¯Êý¶¨Òå
extern int **portalMatrix;
extern int portalNum;
extern int* portalMtrxIndex;
extern NodeDist **portalDistIndex;

void GKS_Query(int StartNode, std::vector<KeyWord> KSet, std::pair<int, int> Cuslimit, BlockPool& blockPool);

void SearchFirstDist(int, std::vector<std::vector<int>> *, bool *, std::vector<KeyWord>&, int);

void SearchFirstDist_2(int, vector<vector<int>> *, bool *, vector<KeyWord>&, int, vector<int>*);

void SearchSecondDist(int, int, int, VisitInfo *, vector<KeyWord>&, bool *, vector<int>**);

void GetBlockToSearch(vector<vector<int>>* ilist, vector<int> *inblockToSearch);

void searchPortalMatrix_2(int, int, int, int, vector<vector<int>> *, bool *, hash_map<int, int>*, vector<KeyWord>&, vector<int>*);


void searchPortalMatrix(int startNode, int pnode, int spDist, int Farthest, vector<vector<int>> *invertList, bool *startVisited, hash_map<int, int>* sRmtpDistMap,
	vector<KeyWord>& KSet);

void doStartBlock(int,  vector<vector<int>> *, bool *, vector<KeyWord>& KSet, int);

void doStartBlock_2(int , vector<vector<int>> *, bool *, vector<KeyWord>& , int ,  vector<int>*);

void AssginTask(vector<vector<int>> *, int , vector<KeyWord>&);

void OutPutResultToFile(char filename[], int Rate);

#endif