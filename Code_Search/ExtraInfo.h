#ifndef _KEYWORD_H
#define _KEYWORD_H

#include <string>
#include "macro.h"

#define MAX_KEYWORD_NUM 10 

#define MAX_RATING 10 //rate <= MAX_RATING

extern int NodeKW[MAX_NODE_NUM]; //node keyword info
extern int NodeRating[MAX_NODE_NUM]; //node rate

typedef enum KeyWord {
	Non = 0,
	Shop = 1,
	Bar = 2,
	Hospital = 3,
	Library = 4,
	Restaurant = 5,
	Hotel = 6,
	Park = 7,
	Cinema = 8,
	Gym = 9,
	Bank = 10
}KeyWord;

//生成keyword
bool generateKeyword(int NodeNum, int MaxKeyWordNum, std::string filename);

//范围 [0...MaxRating - 1]
bool generateRating(int NodeNum, int MaxRating, std::string filename);

//读取rate和keyword信息
bool getData(std::string filename, int NodeNum, int *Store);

#endif

