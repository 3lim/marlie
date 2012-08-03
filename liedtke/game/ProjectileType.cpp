#include "ProjectileType.h"


ProjectileType::ProjectileType(){

};
ProjectileType::ProjectileType(std::string name, float radius, int texture, float speed, float mass, float cooldown, float damage) :
			m_TypeName(name),
			m_Speed(speed),
			m_Cooldown(cooldown),
			m_Damage(damage),
			m_Sprite(),
			m_Mass(mass)
{
	m_Sprite.Position = D3DXVECTOR3(0,0,0);
	m_Sprite.Radius = radius;
	m_Sprite.TextureIndex = texture;
}

bool ProjectileType::fire(Particle* p, D3DXVECTOR3 direction, double& gameTime)
{
	if(m_NextSpawn < gameTime){
 		m_NextSpawn = gameTime + m_Cooldown;
		//*p= Particle(m_Speed, m_Mass, direction); 
		p->setDirection(direction);
		p->setMass(m_Mass);
		p->setSpeed(m_Speed);
		p->setDamage(m_Damage);
		p->setVertexPosition(m_Sprite.Position);
		p->setVertexSize(m_Sprite.Radius);
		p->setVertexTexindex(m_Sprite.TextureIndex);
		return true;
	} else return false;
}

ProjectileType::~ProjectileType(void)
{
}
