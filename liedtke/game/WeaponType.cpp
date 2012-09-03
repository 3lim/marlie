#include "WeaponType.h"

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
WeaponType::WeaponType(std::string name,float x, float y, float z, std::vector<ProjectileType*> projectiles, list<GameObject*>* particles)
	: m_Name(name),
	//m_Weapon(weapon),
	m_particles(particles),
	m_tProjectile(projectiles),
	m_Spawnpoint(D3DXVECTOR3(x,y,z))
{
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
	GameObject* newShot = m_tProjectile[min(m_tProjectile.size(),max(0,i))]->fire(*camera->GetWorldAhead(), gameTime);
	//m_Weapon->getPosition();
	if(newShot!=NULL){
		newShot->Translate(m_Spawnpoint.x,m_Spawnpoint.y,m_Spawnpoint.z);//verschiebe das Projektil zur Kanone
		D3DXVec3TransformCoord(newShot->GetPosition(), newShot->GetPosition(), camera->GetWorldMatrix());//Offset durch den Spawnpoint berechnen und anschließend in World Space übersetzten
		m_particles->push_back(newShot);
	} 
}

WeaponType::~WeaponType(void)
{
	//delete(&m_Weapon);
}
