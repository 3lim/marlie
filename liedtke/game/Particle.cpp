#include "Particle.h"

#include "ParticleEffect.h"


Particle::Particle(float speed, float mass, D3DXVECTOR3 initDirection)
	: m_Mass(mass)
{
	D3DXVec3Normalize(&initDirection, &initDirection);
	m_Velocity = initDirection*speed;
	m_initSpeed = speed;
	Opacity = 1.f;
	AnimationProgress = 0.f;
}
Particle::Particle(void)
{
	Radius = 0.f;
	TextureIndex = 0;
	Opacity = 1.f;
	AnimationProgress = 0.f;
}


Particle::~Particle(void)
{
}

void Particle::setVertexPosition(D3DXVECTOR3 p)
{
	Position = p;
}

void Particle::setVertexSize(float r)
{
	Radius = r;
}

void Particle::setVertexTexindex(int i)
{
	TextureIndex = i;
}

void Particle::setDirection(D3DXVECTOR3 d)
{
	D3DXVec3Normalize(&m_Direction, &d);
	m_Velocity = m_Direction*m_initSpeed;
}

void Particle::setMass(float m)
{
	m_Mass = m;
}

void Particle::setSpeed(float s)
{
	m_Velocity = m_Direction*s;
}

void Particle::move(float fElapsedTime)
{
	m_Velocity = m_Velocity +D3DXVECTOR3(0,-m_Mass, 0)*fElapsedTime;
	Position = Position + m_Velocity*fElapsedTime;
}

void Particle::setDamage(int i)
{
	m_Damage = i;
}

void Particle::onHit()
{
				ParticleEffect p = ParticleEffect(ParticleEffect::g_ParticleEffects["hit"]);
			p.setVertexPosition(getPosition());
			p.setScale(1);
			ParticleEffect::g_ActiveEffects.push_back(p);

}

void Particle::Destroy()
{
}
