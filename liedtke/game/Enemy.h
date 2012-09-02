#pragma once
#include "GameObject.h"
#include "ParticleEffect.h"

class Enemy :
	public GameObject
{
public:
	//Copy Contructor
	Enemy(Enemy* toCLone);
	//Enemy(int hitpoints, int maxUnits, GameObject* object);
	Enemy(int hitpoints, int maxUnits, std::string& meshName, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType relativeTo);
	virtual ~Enemy(void);
	Enemy* Clone();
	void SetMovement(float speed, D3DXVECTOR3& dir);
	void SetSpeed(float speed) { this->speed = speed; }
	//void SetDirection(D3DXVECTOR3& dir) { lookDirection = dir; }
	float GetSpeed() { return speed; }
	void SetDeathEffect(ParticleEffect* p) { effect = p; }
	int GetCountMaxUnits() { return maxUnits; }
	int SpawnedEnemies;
	bool IsDead() { return hitpoints < takenDamage; }
	int GetPoints() { return hitpoints; }
private:
		int hitpoints;
		int takenDamage;
		float speed;
		int maxUnits;
		ParticleEffect* effect;
};

