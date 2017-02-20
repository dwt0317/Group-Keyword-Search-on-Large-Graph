#include <algorithm>
#include "Block.h"
extern int modeSwitch;
Block::Block():mID(0), mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL), mPNodePos(0) {};

Block::Block(int ID, int NodeNum, int *NodeList) :mID(ID), mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL), mPNodePos(NodeNum), portalDistIndex(NULL), pnodeNum(0){
	mNodeList = new std::vector<int>(NodeList, NodeList + NodeNum); //nodenum > 0
	pNodeList = new std::vector<int>(0);
}

Block::Block(const Block& cpy) : mNodeList(NULL), InSDist(NULL), NodeToPortal(NULL), portalDistIndex(NULL) {
	mID = cpy.mID;

	mPNodePos = cpy.mPNodePos;
	pNodeList = new std::vector<int>(*(cpy.pNodeList));
	//construct Nodelist array
	mNodeList = new std::vector<int>(*(cpy.mNodeList));
} 

void Block::release() {
	if (InSDist != NULL) {
		int mNodeNum = mNodeList->size();
		for (int i = 0; i < mNodeNum; i++) {
			delete[] InSDist[i];
		}
		delete[] InSDist;
		InSDist = NULL;
	}

	if (NodeToPortal != NULL) {
		int mNodeNum = mNodeList->size();
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

	if (portalDistIndex != NULL) {
		delete []portalDistIndex;
		portalDistIndex = NULL;
	}
		
}

Block::~Block() {
	release();
}


void Block::Floyd() {
	int mNodeNum = mNodeList->size();
	for (int k = 0; k < mNodeNum; k++) {
		for(int i = 0; i < mNodeNum; i++) {
			if (!InSDist[i][k]) continue; //i to k unreachable
			for (int j = 0; j < mNodeNum; j++) {
				if (i == j) continue; // i to i
				if (!InSDist[k][j]) continue; //k to j unreachable
				int temp = InSDist[i][k] + InSDist[k][j];
				if (!InSDist[i][j] || temp < InSDist[i][j]) {
					InSDist[i][j] = temp;
				}
			}
		}
	}
}

void Block::NodeListToAdjMatrix(Graph& graph) {
		int mNodeNum = mNodeList->size();
		for (int node = 0; node < mNodeNum; node++) {
			//更新
			for (int i = 0; i < node; i++) {
				InSDist[node][i] = InSDist[i][node];  
			}
			InSDist[node][node] = 0;
			for (int i = node + 1; i < mNodeNum; i++) {
				if (!graph.TestAdjacent((*mNodeList)[node], (*mNodeList)[i], &InSDist[node][i]))
					InSDist[node][i] = 0;
			}
		}
}

void Block::ConstructAdjMatrix() {

	int mNodeNum = mNodeList->size();

	InSDist = new int *[mNodeNum];

	for (int i = 0; i < mNodeNum; i++) {
		InSDist[i] = new int[mNodeNum];
	}
}

void Block::CalNodeToPortalIndex() {

	int mNodeNum = mNodeList->size();
	int PortalCount = mNodeNum - mPNodePos;
	if (PortalCount) { //if this block has portal nodes
		NodeToPortal = new NodeDist *[mNodeNum];
		for (int node = 0; node < mNodeNum; node++) {
			NodeToPortal[node] = new NodeDist[PortalCount];
		}

		//for each node in block
		for (int node = 0; node < mNodeNum; node++) {
			//获取所有portal并排序
			for (int portal = mPNodePos, count = 0, end = mNodeList->size(); portal < end; ++portal, ++count) {
				NodeToPortal[node][count].NodeID = portal;
				NodeToPortal[node][count].Dist = InSDist[node][portal];
			}

			std::sort(NodeToPortal[node], NodeToPortal[node] + PortalCount);
		}
	}
	else
		NodeToPortal = NULL;
}


bool Block::OutputBlockIndexToDisk(char filename[]) {
	FILE* out = fopen(filename, "w");

	if (out == NULL)
		return false;

	int nodenum = mNodeList->size();

	//BlockID 除portal外的node数目
	fprintf(out, "%d %d %d\n", mID, mPNodePos,nodenum);   ////加了个nodenum

	//Nodes In Block
	for (std::vector<int>::iterator eachnode = mNodeList->begin(), end = mNodeList->end(); eachnode != end; ++eachnode) {
		fprintf(out, "%d ", *eachnode);
	}

	fprintf(out, "\n");

	//IntraBlockIndex

	for (int i = 0; i < nodenum; i++) {
		for (int j = 0; j < nodenum; j++) {
			fprintf(out, "%d ", InSDist[i][j]);
		}
		fprintf(out, "\n");
	}

	//NodeToPortalIndex
	for (int node = 0; node < nodenum; node++) {
		for (int portal = mPNodePos, count = 0, end = mNodeList->size(); portal < end; ++portal, ++count) {
			fprintf(out, "%d %d ", NodeToPortal[node][count].NodeID, NodeToPortal[node][count].Dist);
		}
		fprintf(out, "\n");
	}

	fclose(out);

	return true;
}
