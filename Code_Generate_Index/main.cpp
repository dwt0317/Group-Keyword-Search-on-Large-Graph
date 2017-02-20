#include "Graph.h"
#include "Partition.h"
#include <time.h>
#include <iostream>
using namespace std;

int main(void) {
	double TotalTime = 0;
	double total = 0;
	FILE* StaFile = NULL;
	Graph *Newyork = new Graph(MAX_NODE_NUM);

	///Newyork->ReadLimitAdjList("Data\\USA-road-d.NY.gr", Newyork->getNodeNum());
	Newyork->ReadLimitAdjList("Data\\USA-road-t.FLA.gr", Newyork->getNodeNum());
	//Newyork->ReadAdjList("Data\\USA-road-d.NY.gr");
	Partition myPartition(MAX_NODE_NUM);
	myPartition.ConstructBlock(0, MAX_BLOCK_NODE, *Newyork);

	myPartition.findBlockPortalNodes(*Newyork);
	myPartition.ConstructIntraBlockIndex(*Newyork);
	//½¨Á¢portal to portal Ë÷Òý
	myPartition.intPortalMatrix();
	myPartition.forPortalMatrix(40001);
	myPartition.OutputBlockIDtoDisk("BlockIndex\\BlockID", Newyork->getNodeNum());

	delete Newyork;


	return 0;
}