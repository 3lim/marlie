#pragma once
#include "objecttransformation.h"
#include "Mesh.h"
class TerrainObject :
	public ObjectTransformation
{
public:
	TerrainObject(std::string meshName, bool useNormal, float scale, float rotX, float rotY, float rotZ, float spacing, float offset, int max, Mesh* object);
	~TerrainObject(void);

	bool isAlignedToNormal() { return o_UseNormal;}
	float getOffset() { return o_Offset;}
	float getSpacing() { return o_Spacing;}
	Mesh* getMesh() { return o_Mesh; }
	int getMaxCount() { return o_MaxCount; }
	bool useNormal() { return o_UseNormal; }
private:
	Mesh* o_Mesh;
	float o_Offset;
	float o_Spacing;
	bool o_UseNormal;
	int o_MaxCount;
};

