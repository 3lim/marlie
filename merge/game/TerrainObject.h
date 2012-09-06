#pragma once
#include "GameObject.h"

class TerrainObject :
	public GameObject
{
public:
	TerrainObject(Mesh* m, float posX, float posZ, float offset, float scale, float rotX, float rotY, float rotZ, float spacing, int max);
	~TerrainObject(void);

	float getSpacing() { return o_Spacing;}
	int getMaxCount() { return o_MaxCount; }
private:
	float o_Spacing;
	int o_MaxCount;
};

