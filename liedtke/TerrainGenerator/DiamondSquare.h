#pragma once
#include <vector>
#include <string>

#ifdef _DEBUG
static const bool DEBUG = false;
#endif  // _DEBUG
#ifndef _DEBUG
static const bool DEBUG = false;
#endif


class DiamondSquare
{
private:
	uint16_t* outputPointer;
	//std::vector<uint16_t> myFieldArray;
	uint16_t* myFieldArray;
	int resolution;
	float roughness;
	static const int threadCountMax = 2;
	int threadCount;
	int threadStepSize;
	//std::vector<uint16_t>* ptrSmoothedTerrain;
	uint16_t* ptrSmoothedTerrain;
	inline int randomize(int max);
	inline int randomize(int min, int max);
	inline int randVariance(int min, int max,unsigned int iteration);
	inline int randVariance(unsigned int iteration);
	void insertStartEdges(int size);
	inline void insertIntoField(int x, int y,int height);
	inline void insertIntoField(int x, int y,int height, int offset);
	inline uint16_t getHeightOfField(int x, int y);
	void GenerateField_v1(int size, float roughness);
	void GenerateField_v2(int size, float roughness);
	int calculateCenterSquare(int height1, int height2, int height3, int height4);
	int calculateCenterPointDiamond(int height1, int height2, int height3, int height4);
public:
	void threadSmoothing();
	DiamondSquare(int size,float roughness);
	~DiamondSquare(void);
	std::uint16_t* GetTerrainField(void);
	void SmoothTerrain(unsigned int iterations);
	void writeArray(bool writeOnRun);
};

