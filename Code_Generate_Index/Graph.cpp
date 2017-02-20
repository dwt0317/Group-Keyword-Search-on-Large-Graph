#include "Graph.h"
int **AdjMatrix;

//生成 adjlist 
Graph::Graph(int node):NodeNum(node) {
	AdjList = new std::vector<std::vector<EdgeNode>>(NodeNum, std::vector<EdgeNode>());
}

//格式 Vi Vj W; RealNodeID - 1 = CurrentNodeID
bool Graph::ReadAdjList(char filename[]) {
	FILE *alfile = fopen(filename, "r");

	if (alfile == NULL)
		return false;

	while (!feof(alfile)) {
		int input[3];
		fscanf(alfile, "a %d %d %d\n", input, input + 1, input + 2);

		(*AdjList)[input[0] - 1].push_back(EdgeNode(input[1] - 1, input[2]));
	}

	fclose(alfile);

	return true;
}


bool Graph::ReadLimitAdjList(char filename[], int NodeNum) {
	
	FILE* AdjFile = fopen(filename, "r");
	if (!AdjFile)
		return false;

	bool input = true;
	while (input) {
		int info[3]; //U, V, W
		fscanf(AdjFile, "a %d %d %d\n", info, info + 1, info + 2);
		if (*info < NodeNum && *(info + 1) < NodeNum)
			(*AdjList)[info[0] - 1].push_back(EdgeNode(info[1] - 1, info[2]));
		else
			input = false;
	}

	fclose(AdjFile);
	return true;
}

//是否邻接
bool Graph::TestAdjacent(int NodeA, int NodeB, int *Dist) {
	bool adj = false;
	for (std::vector<EdgeNode>::iterator eachAdj = (*AdjList)[NodeA].begin(), end = (*AdjList)[NodeA].end(); !adj && eachAdj != end; eachAdj++) {
		if (eachAdj->node == NodeB) {
			adj = true;
			if (Dist != NULL) *Dist = eachAdj->weight;
		}		
	}
	return adj;
}


Graph::~Graph() {
	delete AdjList;
}