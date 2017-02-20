#include <cstdlib>
#include <cstdio>
#include <time.h>
#include "ExtraInfo.h"
#include "macro.h"

int NodeKW[MAX_NODE_NUM] = {0};
int NodeRating[MAX_NODE_NUM] = {0}; 

bool generateKeyword(int NodeNum, int MaxKeyWordNum, std::string filename) {
	FILE* outKeyFile = fopen(filename.c_str(), "w+");

	if (!outKeyFile)
		return false;

	srand((unsigned)time(NULL));

	for (int i = 0; i < NodeNum; i++) {
		int keyword = rand() % MaxKeyWordNum;

		if (keyword) {
			fprintf(outKeyFile, "%d\n", keyword);
		}
		else
			fprintf(outKeyFile, "0\n");
	}

	fclose(outKeyFile);
	return true;
}

bool generateRating(int NodeNum, int MaxRating, std::string filename) {
	FILE* outRatingFile = fopen(filename.c_str(), "w+");

	if (!outRatingFile)
		return false;

	srand((unsigned)time(NULL));

	for (int i = 0; i < NodeNum; i++) {
		int rating = rand() % MaxRating;
		fprintf(outRatingFile, "%d\n", rating);
	}

	fclose(outRatingFile);
	return true;
}

bool getData(std::string filename, int NodeNum, int *Store) {
	FILE* File = fopen(filename.c_str(), "r");

	if (!File)
		return false;

	//get Store[]
	for (int i = 0; i < NodeNum; i++) {
		fscanf(File, "%d", Store + i);
	}

	fclose(File);
	return true;
}