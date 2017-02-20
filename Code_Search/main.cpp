#include "BlockPool.h" //for BlockPool
#include "ExtraInfo.h" //for NodeKW and NodeRating
#include "GKS.h"
#include <time.h>
#include <omp.h>
#include <iostream>

using namespace std;

extern vector<vector<int>> *BlockID;

int threadNum;
extern int rateTotal = 0;
hash_map<int, int>** sRmtpDistMapArray;

int main(void) {
	threadNum = 1;
	omp_set_num_threads(threadNum);
	clock_t start, end;
	double TotalTime = 0;

	//for output
	double total = 0;
	FILE* StaFile = NULL;
	int MaxDist = 0, MaxPairDist = 0;
	vector<KeyWord> Look_For;
	int keyword;

	int choice = 0;
	bool running = true;
	pair<int, int> Range;

	BlockPool* blockPool = new BlockPool();
	sRmtpDistMapArray = new hash_map<int, int>*[threadNum];
	for (int i = 0; i < threadNum; i++){
		sRmtpDistMapArray[i] = new hash_map<int, int>();
	}

	blockPool->ReadBlockIDtoMem("BlockIndex\\BlockID", MAX_NODE_NUM);
	blockPool->GetPortalMatrix("Result\\PortalMtrxIndex.txt");
	blockPool->GetPortalDistIndex("Result\\PortalDistIndex.txt");

	//generateKeyword(1000000,11,"Data\\keyword");
	//generateRating(1000000, 10, "Data\\rate");

	getData("Data\\keyword", MAX_NODE_NUM, NodeKW);
	getData("Data\\rate", MAX_NODE_NUM, NodeRating);


	while (running) {
		printf("1: Test Max Distance\n2: Test Max Pairwise Distance\n3: Test Keyword Number\n0: exit\n");
		scanf("%d", &choice);

		if (choice) {
			printf("input lowerID and upperID [lowerID, upperID)\n");
			scanf("%d %d", &Range.first, &Range.second);
			if (choice != 3) {
				printf("input Keyword end with 0\n");
				Look_For.clear();
				while (true) {
					scanf("%d", &keyword);
					if ((KeyWord)keyword) {
						Look_For.push_back((KeyWord)keyword);
					} 
					else
						break;
				}
			}

			switch (choice) {
			case 1:
				printf("input Max Pairwise Distance\n");
				scanf("%d", &MaxPairDist);
				printf("Start GKS query\n\n");
				StaFile = fopen("Result\\Pairwise\\TestResult.txt", "a+");
				fprintf(StaFile, "thread num: %d\n", threadNum);
				fprintf(StaFile, "node num: %d\n", Range.second);
				for (int MaxDist2 = 5000; MaxDist2 < 35000; MaxDist2 += 5000) {
					start = clock();
					for (int QueryNode = Range.first; QueryNode < Range.second; QueryNode += 20) {				
						GKS_Query(QueryNode, Look_For, pair<int, int>(MaxDist2, MaxPairDist), *blockPool);
					}
					end = clock();
					TotalTime = static_cast<double>(end - start);
					total = TotalTime / CLOCKS_PER_SEC * 1000;
					fprintf(StaFile, "%d : %lf ms\n", MaxDist2, total);
				}
				fprintf(StaFile, "\n");
				fclose(StaFile);
				OutPutResultToFile("Result\\pairRate.txt", rateTotal);
				break;
			case 2:
				printf("input Max Distance\n");
				scanf("%d", &MaxDist);
				StaFile = fopen("Result\\Max\\TestResult.txt", "a+");
				printf("Start GKS query\n\n");
				fprintf(StaFile, "thread num: %d\n", threadNum);
				fprintf(StaFile, "node num: %d\n", Range.second);
				for (int MaxPairDist2 = 1000; MaxPairDist2 < 6000; MaxPairDist2 += 1000) {
					start = clock();
					for (int QueryNode = Range.first; QueryNode < Range.second; QueryNode += 20) {
						GKS_Query(QueryNode, Look_For, pair<int, int>(MaxDist, MaxPairDist2), *blockPool);
					}
					end = clock();
					TotalTime = static_cast<double>(end - start);
					total = TotalTime / CLOCKS_PER_SEC * 1000;
					fprintf(StaFile, "%d : %lf ms\n", MaxPairDist2, total);
				}
				fprintf(StaFile, "\n");
				fclose(StaFile);
				break;

			case 3:
				
				printf("input Max Distance\n");
				scanf("%d", &MaxDist);

				printf("input Max Pairwise Distance\n");
				scanf("%d", &MaxPairDist);

				printf("Start GKS query\n\n");

				KeyWord allKeyword[] = {Shop, Bar, Hospital, Library, Restaurant, Hotel, Park, Cinema, Gym, Bank};
				StaFile = fopen("Result\\Keyword\\TestResult.txt", "a+");
				fprintf(StaFile, "node num: %d\n", Range.second);
				fprintf(StaFile, "thread num: %d\n", threadNum);
				for (int KeywordNum = 2; KeywordNum < 12; KeywordNum += 2) {
					start = clock();
					vector<KeyWord> KSet(allKeyword, allKeyword + KeywordNum);
					for (int QueryNode = Range.first; QueryNode < Range.second; QueryNode += 20) {	
						GKS_Query(QueryNode, KSet, pair<int, int>(MaxDist, MaxPairDist), *blockPool);
					}
					cout << KeywordNum << endl;
					end = clock();
					TotalTime = static_cast<double>(end - start);
					total = TotalTime / CLOCKS_PER_SEC * 1000;				
					fprintf(StaFile, "%d : %lf ms\n", KeywordNum, total);
				}
				fprintf(StaFile, "\n");
				fclose(StaFile);
				OutPutResultToFile("Result\\rate.txt", rateTotal);
				break;
			}
		}
		else
			running = false;
	}

	return 0;
}