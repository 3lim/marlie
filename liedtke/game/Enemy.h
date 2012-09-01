#pragma once
#include "GameObject.h"
#include "ParticleEffect.h"

class Enemy :
	public GameObject
{
public:
	Enemy(int hitpoints, int maxUnits, GameObject* object);
	~Enemy(void);
	Enemy* Clone();
	//void OnCreate();
	//void OnMove(double time, float elapsedTime);
	//void OnHit(Particle* p);
	//void OnDestroy();
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

