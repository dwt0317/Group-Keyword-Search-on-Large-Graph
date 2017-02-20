#include "Partition.h"

#include <iostream>
#include <hash_map>
using namespace std;

vector<vector<int>> *BlockID = NULL; //each nodes' blockID
vector<Block> *BlockArray = NULL; //store blocks

int **portalMatrix;
hash_map<int, int> portalMtrxIndex;
int portalNum=0;
vector<NodeDist>** portalDistIndex;
extern int** AdjMatrix;
hash_map<int, int> rePortalMtrxIndex;

Partition::Partition(int GraphNodeNum) {
	visited = new bool[GraphNodeNum];
	memset(visited, false, sizeof(bool)*GraphNodeNum);

	BlockID = new vector<vector<int>>(GraphNodeNum, vector<int>(1, 0));

	BlockArray = new vector<Block>();
}

Partition::~Partition()
{
	delete [] visited;
	delete BlockID;
	delete BlockArray;
}

void Partition::ConstructBlock(int StartNode, int BlockSize, Graph& graph) {
	int curBlockID = 0;

	int NodeList[MAX_BLOCK_NODE]; //store each block's node id

	int blockSize = BFS(StartNode, BlockSize, curBlockID, NodeList, graph);
	BlockArray->push_back(Block(curBlockID, blockSize, NodeList));

	++curBlockID;

	for (int i = 0; i < MAX_NODE_NUM; i++) {
		if (!visited[i]) {
			//�ֿ�
			int blockSize = BFS(i, BlockSize, curBlockID, NodeList, graph);
			BlockArray->push_back(Block(curBlockID, blockSize, NodeList));
			++curBlockID;
		}
	}
}


int Partition::BFS(int StartNode, int BlockSize, int curBlockID, int NodeList[MAX_BLOCK_NODE], Graph& graph) {
	if (visited[StartNode])
		return 0;
	queue<int> myQueue;

	//blockSize
	int curBlockSize = 0;
	bool enough = false;

	//current node enque
	myQueue.push(StartNode);
	NodeList[0] = StartNode; 
	visited[StartNode] = true;
	(*BlockID)[StartNode][0] = curBlockID;
	++curBlockSize;

	while (!myQueue.empty() && !enough)
	{
		int curNode = myQueue.front();
		//EdgeNode������һ����ͱߵ�Ȩ��
		vector<EdgeNode>& curAdjList = graph.getNodeAdjList(curNode);
		for (vector<EdgeNode>::iterator eachAdj = curAdjList.begin(), end = curAdjList.end(); !enough && eachAdj != end; ++eachAdj) {			
			int node = eachAdj->node;
			if (!visited[node]) { //didn't visited	
				myQueue.push(node);
				visited[node] = true;
				(*BlockID)[node][0] = curBlockID;
				NodeList[curBlockSize] = node;
				++curBlockSize;
				if (curBlockSize == BlockSize)
					enough = true;
			}
		}
		myQueue.pop();
	}
	int t = 0;
	return curBlockSize;
}

void Partition::findPartitionEdges(vector<list<int>> &PEdgeArray, Graph& graph) {
	int NodeNum = graph.getNodeNum();

	for (int i = 0; i < NodeNum; i++) {	
		//��ȡ i ���ڽӱ�
		vector<EdgeNode>& curAdjlist = graph.getNodeAdjList(i);
		for (vector<EdgeNode>::iterator eachNode = curAdjlist.begin(), end = curAdjlist.end(); eachNode != end; ++eachNode) {		
			//[0]�ǵ���ԭʼ��block,����Ķ��ǳ�Ϊportal����ӵ�,�ҳ����ڲ�ͬblock�ı�
			if ((*BlockID)[i][0] != (*BlockID)[eachNode->node][0]) {
				PEdgeArray[i].push_back(eachNode->node);
			}

		}
	}

}

void Partition::findBlockPortalNodes(Graph& graph) {
	int NodeNum = graph.getNodeNum();

	//PEdgeArray�洢��ÿ������������λ������block�ĵ�
	vector<list<int>> PEdgeArray(NodeNum,list<int>());

	//��partition edge
	findPartitionEdges(PEdgeArray, graph);
	cout << PEdgeArray.size();
	//ʹ�� partition edges ��portal nodes
	bool *BePortalNode = new bool[NodeNum];
	memset(BePortalNode, false, sizeof(bool)*NodeNum);

	//for each node in Graph
	for (int i = 0; i < MAX_NODE_NUM; i++) {
		//����Ѿ���portal�ˣ����pedge��ȥ��
		for (list<int>::iterator itr = PEdgeArray[i].begin(); itr != PEdgeArray[i].end();) {
			if (BePortalNode[*itr])
				itr = PEdgeArray[i].erase(itr);
			else
				itr++;
		}

		int Ei = PEdgeArray[i].size(); //node i��pedge����Ŀ
		if (Ei) { //�й��ڴ˵��boundary edge
			int BSi = (*BlockArray)[(*BlockID)[i][0]].getBlockSize(); //node i����block�Ĵ�С
			bool Stop = false;
			for (list<int>::iterator itr = PEdgeArray[i].begin(); !Stop && itr != PEdgeArray[i].end(); itr++) {
				//modify Ej 
				for (list<int>::iterator itrEj = PEdgeArray[*itr].begin(); itrEj != PEdgeArray[*itr].end();) {
					if (BePortalNode[*itrEj])
						itrEj = PEdgeArray[*itr].erase(itrEj);
					else
						++itrEj;
				}
				int Ej = PEdgeArray[*itr].size();
				if (Ej) {  //����Ϊfalse,��Ϊ�ܻ���(itr,i)�ļ�¼
					int BSj = (*BlockArray)[(*BlockID)[*itr][0]].getBlockSize();

					//��һ������ѡ����ֵ�ϸߵĵ�
					if (Ei + BSi > Ej + BSj) {
						Stop = true;
						BePortalNode[i] = true;

						portalMtrxIndex[i] = portalNum;
						rePortalMtrxIndex[portalNum] = i;
						portalNum++;
						//startPortal = i;
						//���뵽���� adjacent block
						for (list<int>::iterator itrAdj = PEdgeArray[i].begin();
							itrAdj != PEdgeArray[i].end(); itrAdj++) {
							int blockID = (*BlockID)[*itrAdj][0];
							//�����Ƿ�ոռ����
							if (!(*BlockArray)[blockID].TestRepetition(i)) {

								(*BlockArray)[blockID].AddNode(i);
								(*BlockArray)[blockID].AddPivot(i);
								(*BlockID)[i].push_back(blockID);
							}
						}

						//portal node add to itselfs block
						(*BlockArray)[(*BlockID)[i][0]].AddNode(i);
						(*BlockArray)[(*BlockID)[i][0]].AddPivot(i);
					}
					else {
						BePortalNode[*itr] = true;
						portalMtrxIndex[*itr] = portalNum;

						rePortalMtrxIndex[portalNum] = *itr;
						portalNum++;
						//add to all adjblock
						for (list<int>::iterator itrAdj = PEdgeArray[*itr].begin();
							itrAdj != PEdgeArray[*itr].end(); itrAdj++) {

							int blockID = (*BlockID)[*itrAdj][0];

							if (!(*BlockArray)[blockID].TestRepetition(*itr)) {
								(*BlockArray)[blockID].AddNode(*itr);
								(*BlockArray)[blockID].AddPivot(*itr);
								(*BlockID)[*itr].push_back(blockID);
							}
						}
						//portal node add to itselfs block
						(*BlockArray)[(*BlockID)[*itr][0]].AddNode(*itr);
						(*BlockArray)[(*BlockID)[*itr][0]].AddPivot(*itr);
						PEdgeArray[*itr].clear();
					}
				}
			}
		}//if(Ei)
	}//foreach node
	PEdgeArray.clear();
	delete [] BePortalNode;
	cout << portalNum << endl;
	cout << (*BlockArray).size() << endl;

	return;
}

void Partition::ConstructIntraBlockIndex(Graph& graph) {

	char filename[20] = "Index\\Block_";
	char charBlockID[10] = { '\0' };

	int count = 0;
	for (vector<Block>::iterator itr = BlockArray->begin(); itr != BlockArray->end(); itr++, ++count) {
		itr->CalIntraBlockIndex(graph);
		itr->CalNodeToPortalIndex();

		//construct output name
		sprintf(charBlockID, "%d", count);
		int len = strlen(charBlockID);
		for (int cat = 0; cat < len; ++cat) {
			filename[cat + 12] = charBlockID[cat];
		}
		filename[len + 12] = '\0';
		itr->OutputBlockIndexToDisk(filename);
	}
}

void Partition::intPortalMatrix(){
	portalDistIndex = new vector<NodeDist> *[portalNum];
	for (int i = 0; i < portalNum; i++){
		portalDistIndex[i] = new vector<NodeDist>();
	}
}


void Partition::forPortalMatrix(int expandLimit){
	bool *routineNode = new bool[MAX_NODE_NUM];
	memset(routineNode, false, sizeof(bool)*MAX_NODE_NUM);
		for (int k = 0; k < (*BlockArray).size(); k++){
			int startPortal = 0;
			vector<int> *nodeList = (*BlockArray)[k].mNodeList;
			int pNodePos = (*BlockArray)[k].mPNodePos;
			int portalCount = (*nodeList).size() - pNodePos;
			if (portalCount){
				for (vector<int>::iterator it = nodeList->begin() + pNodePos; it != nodeList->end(); it++){
					startPortal = *it;
					int lastBlock = -1;
					vector<int> portalRoutine;   //̽��·��
					portalRoutine.push_back(startPortal);
					routineNode[*it] = true;
					buildPortalMatrix(startPortal, 0, expandLimit, portalRoutine, routineNode);
					routineNode[*it] = false;
					vector<int>().swap(portalRoutine);
				}
			}
			cout << k << endl;
		}
	delete[]routineNode;
	buildPortalIndex();
	cout << OutputPortalMtrxIndex("Result\\PortalMtrxIndex.txt") << endl;
}


void Partition::buildPortalMatrix(int startPortal, int curDist, int expandLimit, vector<int> &portalRoutine, bool* routineNode){

	int blockcount = (*BlockID)[startPortal].size();
	int x = 0, y = 0, z = 0; //for simplifying 
	int distIncrement = -1; //for calculate distance 
	int prsize = 0; //portalRoutine.size()

	for (int BCount = 0; BCount < blockcount; BCount++) {
		//��intra id
		int startPortalIntraId = 0, addPortalIntraId = 0;
		int blockID = (*BlockID)[startPortal][BCount];

		vector<int> *BlockNodes = ((*BlockArray)[blockID].mNodeList);
		int end = BlockNodes->size();
		int pNodePos = (*BlockArray)[blockID].mPNodePos;
		for (int i = pNodePos; i < end; i++) {
			if (startPortal == (*BlockNodes)[i]){
				startPortalIntraId = i;
				break;
			}
		}

		vector<int>::iterator it;
		for (it = (*(*BlockArray)[blockID].pNodeList).begin(); it != (*(*BlockArray)[blockID].pNodeList).end(); it++){

			if (routineNode[*it])
				continue;

			for (int i = pNodePos; i < end; i++) {
			//for (int i = 0; i < end; i++) {
				if (*it == (*BlockNodes)[i]){
					addPortalIntraId = i;
					break;
				}
			}

			//Сͼ�ڲ�����������
			int **IntraIndex = (*BlockArray)[blockID].InSDist;

			distIncrement = IntraIndex[startPortalIntraId][addPortalIntraId];

			if (distIncrement == 0)
				continue;

			if (curDist + distIncrement <= expandLimit){
				routineNode[*it] = true;
				portalRoutine.push_back(*it);
				prsize = portalRoutine.size();

				z = portalMtrxIndex[portalRoutine[prsize - 1]];  //portalRoutine��ĩβԪ�ص�matrix id
				y = portalMtrxIndex[portalRoutine[prsize - 2]];  //portalRoutine�ĵ����ڶ�λԪ��matrix id		
				x = portalMtrxIndex[portalRoutine[0]];    //���Ը�Ϊֻ������ʼ�������յľ���
				int xnode1 = 0, znode = 0;;
				xnode1 = portalRoutine[0];
				znode = portalRoutine[prsize - 1];
				int dddist = curDist + distIncrement;

				//�ڽӱ����ʽ
				vector<NodeDist> *xVector = portalDistIndex[x];
				vector<NodeDist> *zVector = portalDistIndex[z];
				int xPos = 0, zPos = 0;
				int xDist = 0, zDist = 0;
				bool exist = false;
				//��z��x�е�λ��
				for (vector<NodeDist>::iterator xit = xVector->begin(); xit != xVector->end(); xit++){
					if (xit->NodeID == portalRoutine[prsize - 1]){
						xDist = xit->Dist;
						exist = true;
						break;
					}
					xPos++;
				}
				if (exist&&curDist + distIncrement <= xDist){
					//�ҵ�x��zVector�е�λ��
					for (vector<NodeDist>::iterator zit = zVector->begin(); zit != zVector->end(); zit++){
						if (zit->NodeID == portalRoutine[0]){
							zDist = zit->Dist;
							break;
						}
						zPos++;
					}
					(*xVector)[xPos].Dist = curDist + distIncrement;
					(*zVector)[zPos].Dist = curDist + distIncrement;
				}
				else if (!exist){
					NodeDist nd;
					nd.NodeID = portalRoutine[prsize - 1];
					nd.Dist = curDist + distIncrement;
					xVector->push_back(nd);
					nd.NodeID = portalRoutine[0];
					zVector->push_back(nd);
				}
				else{
					portalRoutine.pop_back();
					routineNode[*it] = false;
					continue;
				}

				curDist += distIncrement;
				buildPortalMatrix(*it, curDist, expandLimit, portalRoutine, routineNode);
				//�˻ص���һ��
				curDist -= distIncrement;
				portalRoutine.pop_back();
				routineNode[*it] = false;
			}
		}
	}
}



void Partition::buildPortalIndex(){
	for (int i = 0; i < portalNum; i++){
		sort(portalDistIndex[i]->begin(), portalDistIndex[i]->end());
	}

	cout << OutputPortalDistIndex("Result\\PortalDistIndex.txt") << endl;
}

bool Partition::OutputPortalDistIndex(string filename){
	FILE *OutFile = fopen(filename.c_str(), "w+");
	if (!OutFile)
		return false;
	fprintf(OutFile, "%d\n", portalNum);

	for (int i = 0; i < portalNum; i++){
		hash_map<int, int>::iterator h_it;
		h_it = rePortalMtrxIndex.find(i);
		int portalId = h_it->second;
		int availableSize = 1, portalDist = 0;
		availableSize += portalDistIndex[i]->size();
		fprintf(OutFile, "%d\n", availableSize);			//�ܹ��ж����ǿ��õ�
		fprintf(OutFile, "%d %d\n", portalId, 0);   //���Լ��ӽ�ȥ

		for (vector<NodeDist>::iterator it = portalDistIndex[i]->begin(); it !=portalDistIndex[i]->end(); it++){
			fprintf(OutFile, "%d %d\n", (*it).NodeID, (*it).Dist);
		}
	}
	fclose(OutFile);

	return true;
}


bool Partition::OutputPortalMtrxIndex(string filename){
	FILE *OutFile = fopen(filename.c_str(), "w+");
	if (!OutFile)
		return false;
	fprintf(OutFile, "%d\n", portalNum);
	for (int i = 0; i < MAX_NODE_NUM; i++){
			fprintf(OutFile, "%d\n", portalMtrxIndex[i]);
	}
	fclose(OutFile);

	return true;
}

bool Partition::OutputPortalMatrix(string filename){
	FILE *OutFile = fopen(filename.c_str(), "w+");
	if (!OutFile)
		return false;
	fprintf(OutFile, "%d\n", portalNum);

	for (int i = 0; i < portalNum; i++){
		for (int j = 0; j < portalNum; j++){
			fprintf(OutFile, "%d %d %d\n", i, j, portalMatrix[i][j]);
		}
	}
	fclose(OutFile);
	return true;
}


bool Partition::OutputBlockIDtoDisk(char filename[], int GraphNodeNum) {
	FILE *OutFile = fopen(filename, "w");

	if (OutFile == NULL)
		return false;

	for (int node = 0; node < GraphNodeNum; node++) {
		fprintf(OutFile, "%d ", (*BlockID)[node].size());
		for (vector<int>::iterator eachBlock = (*BlockID)[node].begin(), end = (*BlockID)[node].end(); eachBlock != end; ++eachBlock) {
			fprintf(OutFile, "%d ", *eachBlock);
		}
		fprintf(OutFile, "\n");
	}

	return true;
}
