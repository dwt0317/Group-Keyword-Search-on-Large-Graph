#ifndef _BLOCK_H
#define _BLOCK_H

#include <vector>
#include "macro.h" 
#include "NodeDist.h" 

class Block
{
public:
	Block();
	Block(int ID, int PnodePos, int NodeNum, int *NodeList); //construct Nodelist array
	Block(const Block& cpy);
	~Block();

	void ConstructNodeToPortal(NodeDist matrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE / 2]);
	void ConstructAdjMatrix(int matrix[MAX_SWAP_SPACE][MAX_SWAP_SPACE]);
	void release(void);

	int mID;
	int mNodeNum; 
	int mPNodePos; 
	
	std::vector<int> *mNodeList;
	int **InSDist;

	NodeDist **NodeToPortal;

};

#endif