#ifndef _PARTITION_H
#define _PARTITION_H

#include <queue> //for bfs
#include <vector> //for store block
#include <list> //for store Partition Edges
#include "Graph.h" //for graph
#include "Block.h" //for block class

//������ͼ
class Partition
{
public:
	Partition(int GraphNodeNum);
	~Partition();

	//partition graph to blocks
	void ConstructBlock(int StartNode, int BlockSize, Graph& graph);

	void findBlockPortalNodes(Graph& graph);

	//����IntraBlockIndex
	void ConstructIntraBlockIndex(Graph& graph);

	//���BlockID
	bool OutputBlockIDtoDisk(char filename[], int GraphNodeNum);
	bool OutputPortalMatrix(std::string filename);
	bool OutputPortalDistIndex(std::string filename);
	bool OutputPortalMtrxIndex(std::string filename);

	void intPortalMatrix();
	void forPortalMatrix(int);
	void buildPortalMatrix(int, int, int, std::vector<int> &, bool* );
	void buildPortalIndex();

private:
	//BFS �ֿ�
	int BFS(int StartNode, int BlockSize, int curBlockID, int NodeList[MAX_BLOCK_NODE], Graph& graph);

	//�� partition edges;���� PEdgeArray��
	void findPartitionEdges(std::vector<std::list<int>> &PEdgeArray, Graph& graph);

private:
	bool *visited; //visited flag array for contruct block
};



#endif