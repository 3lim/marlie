#include "Enemy.h"
#include "gcProjectile.h"
Enemy::Enemy(Enemy* e) : GameObject(e),
	hitpoints(e->hitpoints),
	takenDamage(e->takenDamage),
	speed(e->speed),
	maxUnits(e->maxUnits),
	effect(e->effect),
	SpawnedEnemies(e->SpawnedEnemies),
	enemyPrefab(e)
{
	e->SpawnedEnemies++;
}
//
//Enemy::Enemy(int points, int units, GameObject* o) : GameObject(o),
//	hitpoints(points),
//	maxUnits(units),
//	takenDamage(0),
//	SpawnedEnemies(0)
//{
//}

Enemy::Enemy(int points, int units, std::string& meshName, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType relativeTo) : GameObject(meshName, posX, posY, posZ, scale, rotX, rotY, rotZ, 0 ,relativeTo),
	hitpoints(points),
	maxUnits(units),
	takenDamage(0),
	SpawnedEnemies(0),
	enemyPrefab(NULL)
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

void Enemy::OnHit(GameObject* o)
{
	takenDamage += ((gcProjectile*)o->GetComponent(GameComponent::tProjectile)->at(0))->GetDamage();
}

Enemy* Enemy::Clone()
{
	return new Enemy(this);
}

Enemy::~Enemy(void)
{
	if(enemyPrefab != NULL)
		enemyPrefab->SpawnedEnemies--;
}
