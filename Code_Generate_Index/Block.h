#ifndef _BLOCK_H
#define _BLOCK_H

#include <string.h> //for memcpy
#include <vector>
#include "Graph.h" //for graph
#include "NodeDist.h" //for nodedist struct for node to portal index

#define MAX_BLOCK_NODE 300
extern int** AdjMatrix;
class Block
{
public:
	Block();
	Block(int ID, int NodeNum, int *NodeList); //����nodelist of block
	Block(const Block& cpy);
	~Block();

	void AddNode(int NodeID) { //Ҫ�� TestRepetition֮����ã������ظ�
		mNodeList->push_back(NodeID); //���node
	}
	void AddPivot(int NodeID){
		pNodeList->push_back(NodeID);
	}
	bool TestRepetition(int NodeID) { return NodeID == mNodeList->back(); }

	int getBlockSize() {return mNodeList->size();}

	void CalIntraBlockIndex(Graph& graph) {
		ConstructAdjMatrix(); 
		NodeListToAdjMatrix(graph); 
		Floyd();
	} //

	void Floyd();

	void CalNodeToPortalIndex();

	bool OutputBlockIndexToDisk(char filename[]);

	void release(void);

	int mID;
	int mPNodePos; //the number of node except portal nodes
	int pnodeNum;
	
	std::vector<int> *mNodeList;
	int **InSDist;
	std::vector<int> *pNodeList; //store portal numbers in the block
	NodeDist **NodeToPortal;
	NodeDist **portalDistIndex;
private:
	void ConstructAdjMatrix();
	void NodeListToAdjMatrix(Graph& graph);
	
};




#endif