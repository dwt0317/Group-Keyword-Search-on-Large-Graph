#ifndef _VISITINFO_H
#define _VISITINFO_H

#include <vector>
#include "ExtraInfo.h" 

class VisitInfo
{
public:
	VisitInfo();
	VisitInfo(int QueryKeywordNumber, std::vector<std::vector<int>> *invertList);
	~VisitInfo();

	int StartLevel[MAX_KEYWORD_NUM + 1];
	std::vector<int> *f[MAX_KEYWORD_NUM + 1][MAX_KEYWORD_NUM + 1];

private:
	int mKeywordNum;
};

#endif