#pragma once
#include "objecttransformation.h"
#include <string>
#include "Mesh.h"
#include "ParticleEffect.h"

class EnemyTransformation :
	public ObjectTransformation
{
public:
	EnemyTransformation(std::string name, int hitpoint, int CountUnits, int speed, Mesh* object, float scale, float rotX, float rotY, float rotZ, float transX, float transY, float ransZ, float SpereSize);
	~EnemyTransformation(void);
	int getHitpoints(){ return e_Hitpoint; }
	int getCountUnits() { return e_CountUnits; }
	int getSpeed() { return e_Speed; }
	Mesh* getObjectMesh() { return e_Object; }
	float getSphereSize() { return e_SphereSize*getScale(); }
	ParticleEffect* getDeathEffect() { return e_DeathEffect; }
	int SpawnedEnemies;
	void setDeathEffect(ParticleEffect* p);
	int getPoints() { return e_Points; }
private:
	int e_Hitpoint;
	int e_CountUnits;
	int e_Speed;
	Mesh* e_Object;
	float e_SphereSize;
	int e_Points;
protected:
	ParticleEffect* e_DeathEffect;
};

