#include "ParticleEffect.h"

std::map<std::string, ParticleEffect> ParticleEffect::g_ParticleEffects;
std::list<ParticleEffect> ParticleEffect::g_ActiveEffects;

ParticleEffect::ParticleEffect(void) :
Particle(0,0,D3DXVECTOR3(0,1,0)),
	m_Duration(0),
	m_TimeElapsed(0),
	m_countSubParticle(0)

{
}

ParticleEffect::ParticleEffect(std::string name, float duration, float size, int texIndex, int count, D3DXVECTOR3 direction, float speed, float mass) :
Particle(speed,mass,direction),
	m_Duration(duration),
	m_TimeElapsed(0),
	m_name(name),
	m_countSubParticle(count)
{
	Radius = size;
	TextureIndex = texIndex;
}

void ParticleEffect::setScale(float s)
{
	Radius = Radius*s;
	m_sizeSubParticle = s;
}

void ParticleEffect::move(float time)
{
	size_t s = m_Child.size();
	if(s > 0 && m_TimeElapsed == 0)
	{
		for(size_t i = 0; i < m_countSubParticle; i++){
			ParticleEffect p = ParticleEffect(*m_Child[i%s]);
			p.setVertexPosition(Position);
			p.setScale(m_sizeSubParticle);
			p.setDirection(D3DXVECTOR3(std::sin(randomAngle(0,180))*std::cos(randomAngle(0,360)), std::sin(randomAngle(0,180))*sin(randomAngle(0,360)), std::cos(randomAngle(0,180)))); 
			g_ActiveEffects.push_back(p);
		}

	}
	m_TimeElapsed += time;
	AnimationProgress = m_TimeElapsed/m_Duration;
	Opacity =1;
	Particle::move(time);
}

ParticleEffect::~ParticleEffect(void)
{
}

float ParticleEffect::randomAngle(float a, float b)
{
	return ((float)rand()/RAND_MAX)*(max(a,b)-min(a,b))+min(a,b);
}
