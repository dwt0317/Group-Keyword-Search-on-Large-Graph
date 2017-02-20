#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include "EdgeNode.h"

#define MAX_NODE_NUM 1070376

//图的定义类
class Graph
{
public:
	Graph(int node); //for graph

	bool ReadAdjList(char filename[]); //读取邻接表

	bool ReadLimitAdjList(char filename[], int NodeNum); 

	int getNodeNum(void) { return NodeNum; }

	std::vector<EdgeNode>& getNodeAdjList(int NodeID) { return (*AdjList)[NodeID]; }

	bool TestAdjacent(int NodeA, int NodeB, int *Dist); //测试两点是否相连

	~Graph();

private:
	int NodeNum; //graph node number
	std::vector<std::vector<EdgeNode>> *AdjList; //adjacent list
};


#endif