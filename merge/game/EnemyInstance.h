#pragma once
#include <string>
#include "EnemyTransformation.h"
#include "DXUT\Core\DXUT.h"
#include "Particle.h"
#include "ParticleEffect.h"
class EnemyInstance
{
public:
	EnemyInstance(EnemyTransformation* enemyObject, D3DXVECTOR3 startPosition, D3DXVECTOR3 MovingDirection);
	~EnemyInstance(void);
	void move(float time);
	D3DXVECTOR3 getCurrentPosition() { return e_CurrentPosition; }
	D3DXVECTOR3 getDirection() { return e_Direction; }
	D3DXMATRIX getAnimation(void);
	void calculateWorldMatrix();
	EnemyTransformation* getObject() { return e_MyObject; }
	int getLife() { return e_hitpoints;}
	void OnHit(Particle* p);
	void Destroy();
	void Death();
	void SetDestroyEffect(ParticleEffect* p);
	ParticleEffect* getDeathEffect() { return e_MyObject->getDeathEffect(); }
private:
	std::string e_EnemyType;
	D3DXVECTOR3 e_CurrentPosition;
	D3DXVECTOR3 e_Velocity;
	D3DXVECTOR3 e_Direction;
	EnemyTransformation* e_MyObject;
	int e_hitpoints;
};

