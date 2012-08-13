#pragma once
#include <string>
#include "SpriteRenderer.h"
#include <D3D11.h>
#include <ctime>
#include "Particle.h"

#include "objecttransformation.h"
class ProjectileType {
public:
	ProjectileType(void);
	ProjectileType(std::string name, float radius, int texture, float speed, float mass, float cooldown, float damage);
	~ProjectileType(void);
	bool fire(Particle* p, D3DXVECTOR3 direction, double& gameTime);
private:
	
	std::string m_TypeName;
	SpriteVertex m_Sprite;
	float m_Speed;
	float m_Mass;
	float m_Cooldown;
	float m_Damage;
	double m_NextSpawn;

	
};

