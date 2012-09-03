#pragma once
#include <string>
#include "SpriteRenderer.h"
#include <D3D11.h>
#include <ctime>
#include "Particle.h"

#include "GameObject.h"
class ProjectileType : public GameObject {
public:
	ProjectileType(std::string name, float radius, int texture, float speed, float mass, float cooldown, int damage);
	~ProjectileType(void);
	GameObject* fire(D3DXVECTOR3 direction, double& gameTime);
	void setIndexOffset(int index);
private:
	
	std::string m_TypeName;
	float m_Cooldown;
	float m_Damage;
	double m_NextSpawn;
	float m_speed;
};

