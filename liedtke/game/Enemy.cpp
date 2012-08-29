#include "Enemy.h"


Enemy::Enemy(int points, int units, GameObject& o) : GameObject(o),
	hitpoints(points),
	maxUnits(units),
	takenDamage(0),
	SpawnedEnemies(0)
{
}

//void Enemy::OnMove(double time, float elapsedTime)
//{
//
//	Translate(velocity.x, velocity.y, velocity.z);
//}

void Enemy::OnDestroy()
{
	SpawnedEnemies--;
}

void Enemy::OnHit(Particle* p)
{
	//TODO
}

void Enemy::SetMovement(float speed, D3DXVECTOR3& dir)
{
	D3DXVec3Normalize(&dir, &dir);
	this->speed = speed;
	lookDirection = dir;
	AddForce(speed, dir);
}


Enemy::~Enemy(void)
{
}
