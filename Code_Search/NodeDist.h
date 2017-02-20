#ifndef _NODEDIST_H
#define _NODEDIST_H

typedef struct NodeDist {
	
	NodeDist():NodeID(0), Dist(0) {};
	NodeDist(const NodeDist& cpy) :NodeID(cpy.NodeID), Dist(cpy.Dist) {};
	
	int NodeID;
	int Dist;

}NodeDist;

#endif