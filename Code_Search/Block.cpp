#include <string.h> 
#include "Block.h"

Block::Block():mID(0), mNodeNum(0), mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL), mPNodePos(0) {};

Block::Block(int ID, int PnodePos, int NodeNum, int *NodeList) :mID(ID), mNodeNum(NodeNum), mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL), mPNodePos(PnodePos) {
	mNodeList = new std::vector<int>(NodeList, NodeList + NodeNum); //nodenum > 0
}

Block::Block(const Block& cpy):mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL) {
	mID = cpy.mID;
	mNodeNum = cpy.mNodeNum;
	mPNodePos = cpy.mPNodePos;

	mNodeList = new std::vector<int>(*(cpy.mNodeList));
} 

void Block::release() {
	if (InSDist != NULL) {
		for (int i = 0; i < mNodeNum; i++) {
			delete[] InSDist[i];
		}
		delete[] InSDist;
		InSDist = NULL;
	}

	if (NodeToPortal != NULL) {
		for (int i = 0; i < mNodeNum; i++) {
			delete[] NodeToPortal[i];
		}
		delete[] NodeToPortal;
		NodeToPortal = NULL;
	}

	if (mNodeList != NULL) {
		delete mNodeList;
		mNodeList = NULL;
	}
		
}

Block::~Block() {
	release();
}

void Block::ConstructAdjMatrix(int matrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE]) {

	InSDist = new int *[mNodeNum];
	for (int i = 0; i < mNodeNum; i++) {
		InSDist[i] = new int[mNodeNum];
		memcpy(InSDist[i], matrix[i], sizeof(int)*mNodeNum);
	}
}

//½¨nodeµ½portal×î¶Ìindex
void Block::ConstructNodeToPortal(NodeDist matrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE / 2]) {

	int PortalCount = mNodeNum - mPNodePos;
	if (PortalCount) { 
		NodeToPortal = new NodeDist *[mNodeNum];
		for (int node = 0; node < mNodeNum; node++) {
			NodeToPortal[node] = new NodeDist[PortalCount];
			memcpy(NodeToPortal[node], matrix[node], sizeof(NodeDist)*PortalCount);
		}
	}
	else
		NodeToPortal = NULL;
}

