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

//void Enemy::OnCreate()
//{
//	GameObject::OnCreate();
//	SpawnedEnemies++;
//}
//
//void Enemy::OnDestroy()
//{
//	GameObject::OnDestroy();
//	SpawnedEnemies--;
//}

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
