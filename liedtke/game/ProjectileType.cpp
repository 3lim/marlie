#include "ProjectileType.h"
#include "SphereCollider.h"
#include "gcProjectile.h"

ProjectileType::ProjectileType(std::string name, float radius, int texture, float speed, float mass, float cooldown, int damage) :GameObject(texture,radius, 0,0,0, 0, GameObject::WORLD),
	m_TypeName(name),
	m_Cooldown(cooldown),
	m_Damage(damage),
	m_speed(speed)
{
	AddComponent(new SphereCollider(radius));
	AddComponent(new gcProjectile(damage, speed, cooldown));

}

GameObject* ProjectileType::fire(D3DXVECTOR3 direction, double& gameTime)
{
	GameObject* p = NULL;
	if(m_NextSpawn < gameTime){
 		m_NextSpawn = gameTime + m_Cooldown;
		p = Clone();
		p->AddForce(m_speed, direction);
		//*p= Particle(m_Speed, m_Mass, direction); 
		//p->setDirection(direction);
		//p->setMass(m_Mass);
		//p->setSpeed(m_Speed);
		//p->setDamage(m_Damage);
		//p->setVertexPosition(m_Sprite.Position);
		//p->setVertexSize(GetScale());
		//p->setVertexTexindex(myVertex.TextureIndex);
		return p;
	}
	return NULL;
}

ProjectileType::~ProjectileType(void)
{
}

void ProjectileType::setIndexOffset(int i)
{
	myVertex.TextureIndex = i;
}
