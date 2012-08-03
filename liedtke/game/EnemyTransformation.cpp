#include "EnemyTransformation.h"


EnemyTransformation::EnemyTransformation(std::string name, int hitpoint, int CountUnits, int speed, Mesh* object, float scale, float rotX, float rotY, float rotZ, float transX, float transY, float transZ, float shpereSize)
	: ObjectTransformation(name, false, scale, rotX, rotY, rotZ, transX, transY, transZ),
	 e_Hitpoint(hitpoint),
	 e_CountUnits(CountUnits),
	 e_Speed(speed),
	 e_Object(object),
	 e_SphereSize(shpereSize),
	 SpawnedEnemies(0),
	 e_Points(hitpoint*0.5f)
{
}


EnemyTransformation::~EnemyTransformation(void)
{
}


void EnemyTransformation::setDeathEffect(ParticleEffect* p)
{
	e_DeathEffect = p;
}
