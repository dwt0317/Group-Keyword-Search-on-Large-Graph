#ifndef _BLOCKPOOL_H
#define _BLOCKPOOL_H

#include "Block.h" 
#include "macro.h"
#include<iostream>

//�洢block��Ϣ�Ľṹ
class BlockPool
{
public:
	BlockPool();
	BlockPool(const BlockPool& cpy);
	~BlockPool();

	//Read BlockID to Mem
	bool ReadBlockIDtoMem(char filename[], int GraphNodeNum);

	//��ȡblock��Ϣ
	Block* findBlock(int BlockID);
	bool GetPortalMatrix(std::string filename);
	bool GetPortalDistIndex(std::string filename);
	void intPortalMatrix();
private:
	Block* ReadBlocktoPool(int BlockID);
	void AddBlock(Block* pblock);

private:

};


#endif