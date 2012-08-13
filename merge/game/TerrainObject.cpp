#include "TerrainObject.h"
#include "Mesh.h"


TerrainObject::TerrainObject(std::string meshName, bool useNormal, float scale, float rotX, float rotY, float rotZ, float spacing, float offset, int maxCount, Mesh* objectMesh) 
	: ObjectTransformation(ObjectTransformation(meshName, false, scale, rotX, rotY, rotZ, 0.f,0.f,0.f)),
	o_UseNormal(useNormal),
	o_Spacing(spacing),
	o_Offset(offset),
	o_Mesh(objectMesh),
	o_MaxCount(maxCount)
{
}


TerrainObject::~TerrainObject(void)
{
}
