#include "VisitInfo.h"

VisitInfo::VisitInfo() {};

VisitInfo::VisitInfo(int QueryKeywordNumber, std::vector<std::vector<int>> *invertList):mKeywordNum(QueryKeywordNumber) {
	//f[0, 1]; f[0, 2], f[1, 2]; f[0, 3], f[1, 3], f[2, 3];...
	for (int i = 1; i <= mKeywordNum; i++) {
		for (int j = 0; j < i; j++) {
			f[j][i] = new std::vector<int>();
		}
	}

	//initial continuous array, ´ýËÑË÷µãarray
	for (int i = 0; i < (MAX_KEYWORD_NUM + 1); i++) {
		StartLevel[i] = 0;
	}
	//f[0][1]´æµÄÊÇinvertlist[0]
	//f[0, 1...KeywordNum] store the raw line of invertList[0, ..., KeywordNum - 1]
	for (int kpos = 0; kpos < mKeywordNum; ++kpos) {
		*f[0][kpos + 1] = (*invertList)[kpos];
	}
}

VisitInfo::~VisitInfo() {
	//release f[][] array
	for (int i = 1; i <= mKeywordNum; i++) {
		for (int j = 0; j < i; j++) {
			delete f[j][i];
		}
	}
}

