#ifndef _EDGENODE_H
#define _EDGENODE_H

struct EdgeNode {
	EdgeNode() :node(0), weight(std::numeric_limits<int>::max()) {};
	EdgeNode(int n, int w) :node(n), weight(w) {};
	EdgeNode(const EdgeNode& cpy) { node = cpy.node; weight = cpy.weight; }
	bool operator<(const EdgeNode& r) { return weight < r.weight; }
		
	int node;
	int weight;
};


#endif