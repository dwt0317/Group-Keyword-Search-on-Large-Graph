#include "BlockPool.h"
#include<time.h>
#include<iostream>
#include<omp.h>

using namespace std;

vector<vector<int>> *BlockID = NULL; //each nodes' blockID
vector<Block*> *BlockArray; //store blocks
int **portalMatrix;
int *portalMtrxIndex;  //global id -> matrix id
NodeDist **portalDistIndex;
int portalNum = 0;
extern int threadNum;

int nodelist[MAX_SWAP_SPACE]; //temp reading nodelist
int adjmatrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE]; //temp reading intrablock
NodeDist nodeToPortal[MAX_SWAP_SPACE][MAX_SWAP_SPACE / 2]; //temp reading nodetoportal

BlockPool::BlockPool()  {

}

BlockPool::BlockPool(const BlockPool& cp) {

}

void BlockPool::AddBlock(Block* pblock) {
	BlockArray->push_back(pblock);
}

//读取block信息
Block* BlockPool::ReadBlocktoPool(int BlockID) {
	char filename[20] = "Index\\Block_";
	char charBlockID[10] = { '\0' };

	sprintf(charBlockID, "%d", BlockID);
	int len = strlen(charBlockID);
	for (int cat = 0; cat < len; cat++) {
		filename[cat + 12] = charBlockID[cat];
	}
	filename[len + 12] = '\0';
	FILE *InFile = fopen(filename, "r");
	if (InFile == NULL)
		return NULL;

	int id, mpnodepos, totalnode;
	fscanf(InFile, "%d %d %d\n", &id, &mpnodepos, &totalnode);

	for (int node = 0; node < totalnode; node++) {
		fscanf(InFile, "%d ", nodelist + node);
	}

	Block *block = new Block(id, mpnodepos, totalnode, nodelist); //construct block
	for (int i = 0; i < totalnode; i++) {
		for (int j = 0; j < totalnode; j++) {
			fscanf(InFile, "%d ", &adjmatrix[i][j]);
		}
	}
	block->ConstructAdjMatrix(adjmatrix);
	//read
	int PortalCount = totalnode - mpnodepos;
	for (int i = 0; i < totalnode; i++) {
		for (int j = 0; j < PortalCount; j++) {
			fscanf(InFile, "%d %d ", &nodeToPortal[i][j].NodeID, &nodeToPortal[i][j].Dist);
		}
	}
	block->ConstructNodeToPortal(nodeToPortal);
	
	(*BlockArray)[BlockID] = block;
	fclose(InFile); 

	return block;
}

//读取索引
bool BlockPool::ReadBlockIDtoMem(char filename[], int GraphNodeNum) {
	FILE *InFile = fopen(filename, "r");

	if (InFile == NULL)
		return false;

	BlockID = new vector<vector<int>>(GraphNodeNum, vector<int>());
	for (int node = 0; node < GraphNodeNum; node++) {
		int len;
		fscanf(InFile, "%d ", &len);
		for (int i = 0; i < len; i++) {
			int temp;
			fscanf(InFile, "%d ", &temp);
			(*BlockID)[node].push_back(temp);
		}
	}

	fclose(InFile);

	InFile = fopen("Index\\Block_Num", "r");

	if (InFile == NULL){
		cout << "Error in reading block numbers!" << endl;
		return false;
	}
	int blockNums = 0;
	fscanf(InFile, "%d ", &blockNums);
	BlockArray = new vector<Block*>(blockNums, NULL);
	for (int i = 0; i < blockNums; i++){
		ReadBlocktoPool(i);
	}
	return true;
}

//读取portalDistIndex,记录每个portal可到的portal
bool BlockPool::GetPortalDistIndex(string filename){
	FILE *read = fopen(filename.c_str(), "r");
	if (!read)
		return false;
	fscanf(read, "%d\n", &portalNum);
	portalDistIndex = new NodeDist*[portalNum];
	for (int i = 0; i < portalNum; i++){
		int availableSize = 0;
		fscanf(read, "%d\n", &availableSize);
		portalDistIndex[i] = new NodeDist[availableSize+1];

		portalDistIndex[i][0].NodeID = availableSize;
		for (int j = 0; j < availableSize; j++){
			fscanf(read, "%d %d\n", &portalDistIndex[i][j+1].NodeID, &portalDistIndex[i][j+1].Dist);
		}
	}
	return true;
}

bool BlockPool::GetPortalMatrix(string filename){
	FILE *read = fopen(filename.c_str(), "r");
	bool input = true;
	int info[3] = { 0 };
	if (!read)
		return false;
	int count = 0;
	//读取邻接矩阵
	fscanf(read, "%d\n", &portalNum);
	intPortalMatrix();
	for (int i = 0; i < MAX_NODE_NUM; i++){
		fscanf(read, "%d\n", &portalMtrxIndex[i]);
	}
	fclose(read);
	return true;
}



void BlockPool::intPortalMatrix(){
	portalMtrxIndex = new int[MAX_NODE_NUM];
	memset(portalMtrxIndex, -1, sizeof(int)*MAX_NODE_NUM);
}

BlockPool::~BlockPool() {
	delete BlockArray;
	delete[]portalMtrxIndex;
	delete BlockID;

}
