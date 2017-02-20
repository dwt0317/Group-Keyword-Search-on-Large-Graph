#ifndef _NODEDIST_H
#define _NODEDIST_H

//用来构建邻接表，用于构建索引
typedef struct NodeDist {	
	NodeDist():NodeID(0), Dist(0) {};	
	int NodeID;
	int Dist;
	bool operator< (const NodeDist &lb) const { return Dist < lb.Dist; }
}NodeDist;

#endif