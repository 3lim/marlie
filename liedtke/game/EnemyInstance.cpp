#include "EnemyInstance.h"


EnemyInstance::EnemyInstance(EnemyTransformation* enemyObject, D3DXVECTOR3 startPosition, D3DXVECTOR3 velocity)
	: e_EnemyType(enemyObject->getName()),
	e_CurrentPosition(startPosition),
	e_Velocity(velocity),
	e_MyObject(enemyObject)
{
	e_Direction = e_Velocity;
	D3DXVec3Normalize(&e_Direction, &e_Direction);
	e_Velocity *= enemyObject->getSpeed();
	e_hitpoints = enemyObject->getHitpoints();
	
}


EnemyInstance::~EnemyInstance(void)
{
}

void EnemyInstance::move(float time)
{
	e_CurrentPosition += time*e_Velocity;
}

D3DXMATRIX EnemyInstance::getAnimation(){
	return D3DXMATRIX(e_Direction.x, e_Direction.y,e_Direction.z, 0, 
						0, 1, 0 ,0, 
						-e_Direction.z, e_Direction.y, e_Direction.x, 0,
						e_CurrentPosition.x, e_CurrentPosition.y, e_CurrentPosition.z, 1);
}

void EnemyInstance::OnHit(Particle* p)
{
	e_hitpoints -= p->getDamage();
}
void EnemyInstance::Destroy()
{
	e_MyObject->SpawnedEnemies--;
};
void EnemyInstance::Death()
{
	if(e_MyObject->getDeathEffect() !=NULL)
	{
		ParticleEffect p = ParticleEffect(*e_MyObject->getDeathEffect());
		p.setVertexPosition(e_CurrentPosition);
		p.setScale(1);
		ParticleEffect::g_ActiveEffects.push_back(p);
	}
	Destroy();
}

void EnemyInstance::calculateWorldMatrix()
{
	e_MyObject->calculateWorldMatrix();
	e_MyObject->g_World*= getAnimation();
}
