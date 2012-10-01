#include "WeaponType.h"
#include "gcProjectile.h"
using namespace std;

//WeaponType::WeaponType(std::string name, D3DXVECTOR3 spawnpoint, ObjectTransformation* weapon, std::vector<ProjectileType*> projectiles)
//	: m_Name(name),
//	m_Weapon(weapon),
//	m_tProjectile(projectiles),
//	m_Spawnpoint(spawnpoint)
//{
//}
//
//WeaponType::WeaponType(std::string name,float x, float y, float z, std::vector<ProjectileType*> projectiles)
//	: m_Name(name),
//	//m_Weapon(weapon),
//	m_tProjectile(projectiles),
//	m_Spawnpoint(D3DXVECTOR3(x,y,z))
//{
//}
WeaponType::WeaponType(std::string name,float x, float y, float z, std::vector<GameObject*> projectiles) : GameObject(name, x, y, z, 0,0,0,0),
	m_Name(name),
	//m_Weapon(weapon),
	m_tProjectile(projectiles),
	m_Spawnpoint(D3DXVECTOR3(x,y,z))
{
	m_nextSpawnTimes.resize(projectiles.size());
	for(size_t i = 0; i < m_nextSpawnTimes.size(); i++)
		m_nextSpawnTimes[i] = 0.0;
}
//WeaponType::WeaponType(std::string name, D3DXVECTOR3* spawnPos, std::vector<ProjectileType*> projectiles, list<Particle>* particles)
//	: m_Name(name),
//	//m_Weapon(weapon),
//	m_particles(particles),
//	m_tProjectile(projectiles),
//	m_Spawnpoint(*spawnPos)
//{
//}
//
//WeaponType::WeaponType(std::string name, D3DXVECTOR3 spawnpoint, std::vector<ProjectileType*> projectiles)
//	: m_Name(name),
//	//m_Weapon(weapon),
//	m_tProjectile(projectiles),
//	m_Spawnpoint(spawnpoint)
//{
//}

void WeaponType::fire(size_t i, CFirstPersonCamera* camera, double& gameTime)
{
	if(m_nextSpawnTimes[i] < gameTime)
	{
		GameObject* newShot = m_tProjectile[min(m_tProjectile.size(),max(0,i))]->Clone();
		gcProjectile* p = (gcProjectile*)newShot->GetComponent(GameComponent::tProjectile, 0);
		m_nextSpawnTimes[i] = gameTime+p->GetCooldown();
		newShot->Translate(m_Spawnpoint.x,m_Spawnpoint.y,m_Spawnpoint.z);//verschiebe das Projektil zur Kanone
		newShot->AddForce(p->GetSpeed(), (D3DXVECTOR3)*camera->GetWorldAhead());
		D3DXVec3TransformCoord(newShot->GetPosition(), newShot->GetPosition(), camera->GetWorldMatrix());//Offset durch den Spawnpoint berechnen und anschließend in World Space übersetzten
		
		newShot->OnCreate(gameTime);
		g_Particles.push_back(newShot);
	}
}

WeaponType::~WeaponType(void)
{
	//delete(&m_Weapon);
}
