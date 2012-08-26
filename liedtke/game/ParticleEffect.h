#pragma once
#include "particle.h"
#include <list>
#include <map>
#include <vector>
#include <string>
class ParticleEffect :
	public Particle
{
public:
	static std::list<ParticleEffect> g_ActiveEffects;
	static std::map<std::string, ParticleEffect> g_ParticleEffects;

	ParticleEffect(void);
	ParticleEffect(std::string name, float duration, float size, int texIndex, int count, D3DXVECTOR3 direction, float speed, float mass);
	~ParticleEffect(void);

	void move(float tElapsedTime);

	bool isTimeOver() { return AnimationProgress > 1;}

	void setVertexPosition(D3DXVECTOR3 p) { Position = p; }
	void SetChildEffect(ParticleEffect* p) { m_Child.push_back(p); }
	void SetChildEffect(std::string p) { m_Child.push_back(&g_ParticleEffects[p]); }
	void setScale(float s);
	SpriteVertex getSpriteToRender() { return (SpriteVertex)*this; }

private:
	float m_Duration;
	float m_TimeElapsed;
	std::string m_name;

	float randomAngle(float min, float max);

	//Subparticle system
	size_t m_countSubParticle;
	float m_sizeSubParticle;
	std::vector<ParticleEffect*> m_Child;
};

