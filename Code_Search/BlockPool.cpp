#include "BlockPool.h"
#include<time.h>
#include<iostream>
#include<omp.h>

using namespace std;
extern double time1, time2, time3, time4, time5, time6,time7;

std::vector<std::vector<int>> *BlockID = NULL; //each nodes' blockID
vector<Block*> *BlockArray; //store blocks
int **portalMatrix;
int *portalMtrxIndex;  //global id -> matrix id
NodeDist **portalDistIndex;
int portalNum = 0;
extern int threadNum;

//这几个不能弄成局部变量，一个是会造成stack overflow，再一个反复创建空间极其耗时
//也不能作为类的变量，估计也是太大
int nodelist[MAX_SWAP_SPACE]; //temp store reading nodelist
int adjmatrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE]; //temp store reading intrablock
NodeDist nodeToPortal[MAX_SWAP_SPACE][MAX_SWAP_SPACE / 2]; //temp store reading nodetoportal



BlockPool::BlockPool()  {

}

BlockPool::BlockPool(const BlockPool& cp) {

}


void BlockPool::AddBlock(Block* pblock) {
	BlockArray->push_back(pblock);
}


//读取block信息
//多线程同时读取同一块block的几率并不大，问题不在于文件
//在不同文件之间一直跳来跳去会导致read head来回跑，产生开销
Block* BlockPool::ReadBlocktoPool(int BlockID) {
	//construct intput name
	char filename[20] = "Index\\Block_";
	char charBlockID[10] = { '\0' };

	sprintf(charBlockID, "%d", BlockID);

	int len = strlen(charBlockID);

	for (int cat = 0; cat < len; cat++) {
		filename[cat + 12] = charBlockID[cat];
	}

	filename[len + 12] = '\0';
	//

	FILE *InFile = fopen(filename, "r");

	if (InFile == NULL)
		return NULL;

	//read
	int id, mpnodepos, totalnode;
	fscanf(InFile, "%d %d %d\n", &id, &mpnodepos, &totalnode);

	//read
	for (int node = 0; node < totalnode; node++) {
		fscanf(InFile, "%d ", nodelist + node);
	}

	Block *block = new Block(id, mpnodepos, totalnode, nodelist); //construct block

	//read
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
	
	//AddBlock(block); //add block to blockPool
	(*BlockArray)[BlockID] = block;
	fclose(InFile); //alert you to shutdown infile !!!!!!!

	return block;
}

//读取索引
bool BlockPool::ReadBlockIDtoMem(char filename[], int GraphNodeNum) {
	FILE *InFile = fopen(filename, "r");

	if (InFile == NULL)
		return false;

	BlockID = new std::vector<std::vector<int>>(GraphNodeNum, std::vector<int>());

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

	//BlockArray = new vector<Block*>();

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
	//cout << "dd";
	//cin >> blockNums;

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

		portalDistIndex[i][0].NodeID = availableSize;//第一个储存单元存放可用的portal有多少个
		for (int j = 0; j < availableSize; j++){
			fscanf(read, "%d %d\n", &portalDistIndex[i][j+1].NodeID, &portalDistIndex[i][j+1].Dist);
		}
	}
	return true;
}

bool BlockPool::GetPortalMatrix(string filename){
	FILE *read = fopen(filename.c_str(), "r");
	bool input = true;

	//在C++中，利用花括号给数组初始化，如果花括号中给的初始值的个数不够，则会自动将未给出初始值的元素赋0
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
	//while (!feof(read))
	//{
	//	fscanf(read, "%d %d %d\n", info, info + 1, info + 2);
	//	portalMatrix[info[0]][info[1]] = info[2];
	//}

	fclose(read);
	return true;
}



void BlockPool::intPortalMatrix(){
	//portalMatrix = new int *[portalNum];
	//for (int i = 0; i < portalNum; i++) {
	//	portalMatrix[i] = new int[portalNum];
	//	memset(portalMatrix[i], 0, sizeof(int)*portalNum);
	//}
	portalMtrxIndex = new int[MAX_NODE_NUM];
	memset(portalMtrxIndex, -1, sizeof(int)*MAX_NODE_NUM);
}

BlockPool::~BlockPool() {
	delete BlockArray;

	delete[]portalMtrxIndex;

	delete BlockID;

}


//curBlockNum突然变化，其他代码越界？ shide


//结构体不能作为threadprvate的变量
//#pragma omp threadprivate(adjmatrix)