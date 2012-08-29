#include "TerrainObject.h"


TerrainObject::TerrainObject(Mesh* m, float posX, float posZ, float offset, float scale, float rotX, float rotY, float rotZ, float spacing, int max) 
	: GameObject(m, posX, offset, posZ, scale, rotX, rotY, rotZ, TERRAIN),
	o_Spacing(spacing),
	o_MaxCount(max)
{
}


TerrainObject::~TerrainObject(void)
{
}
