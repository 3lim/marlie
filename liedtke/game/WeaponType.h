#pragma once
#include "ProjectileType.h"
#include "Mesh.h"
#include "ObjectTransformation.h"
#include <string>
#include <vector>
#include <list>
#include "GameObject.h"

extern std::list<GameObject*> g_Particles;
class WeaponType
{
public:
	//WeaponType(std::string name, D3DXVECTOR3 spawnpoint, ObjectTransformation* weapon, std::vector<ProjectileType*> projectiles);
	//WeaponType(std::string name, D3DXVECTOR3 spawnpoint, std::vector<ProjectileType*> projectiles);
	//WeaponType(std::string name, float x, float y, float z, std::vector<ProjectileType*> projectiles);
	WeaponType(std::string name, float x, float y, float z, std::vector<GameObject*> projectiles);
	//WeaponType(std::string name, D3DXVECTOR3* spawnPos, std::vector<ProjectileType*> projectiles, std::list<Particle>* particles);
	~WeaponType(void);
	void fire(size_t projecctileType, CFirstPersonCamera* camera, double& gameTime);

private:
	std::string m_Name;
	ObjectTransformation* m_Weapon;
	std::vector<GameObject*> m_tProjectile;
	D3DXVECTOR3 m_Spawnpoint;
	std::vector<double> m_nextSpawnTimes;

};

