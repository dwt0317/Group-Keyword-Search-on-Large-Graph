#ifndef _KEYWORD_H
#define _KEYWORD_H

#include <string>
#include "macro.h"

#define MAX_KEYWORD_NUM 10 

#define MAX_RATING 10 //rating score <= MAX_RATING

extern int NodeKW[MAX_NODE_NUM]; //node keyword info
extern int NodeRating[MAX_NODE_NUM]; //node rating score

//enumeration �����޶���������ķ�Χ����������б��оͻᱨ��ͬһö����ö���ӵ�ȡֵ����ҪΨһ
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

//generate keyword from Non to MaxKeywordNumber [0, 1, 2, 4, ..., 2^MaxKeyWordNum]
//then output to file filename
bool generateKeyword(int NodeNum, int MaxKeyWordNum, std::string filename);

//random generate rating score for NodeNum
//rating score range from [0...MaxRating - 1]
bool generateRating(int NodeNum, int MaxRating, std::string filename);

//read the data from file and store them in Store array
bool getData(std::string filename, int NodeNum, int *Store);

#endif

