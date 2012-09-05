#include "ParticleSystem.h"

std::map<std::string, ParticleSystem*> ParticleSystem::g_ParticleSystems;
std::list<ParticleSystem*> ParticleSystem::g_activeParticleSystems;

ParticleSystem::ParticleSystem(ParticleSystem* p) : GameObject(p),
	emitedDirection(p->emitedDirection),
	maxCount(p->maxCount),
	intervall(p->intervall),
	emittedCount(p->emittedCount),
	nextEmit(p->nextEmit),
	tEndEmit(p->tEndEmit),
	emitsInWorldSpace(p->emitsInWorldSpace),
	emittedType(p->emittedType)
{
	emitedObject = p->emitedObject->Clone();
}

ParticleSystem::ParticleSystem(int textureIndex, float scale, float posX, float posY, float posZ, PositionType tPos, GameObject* emits,  float dirX, float dirY, float dirZ, size_t count, float tIntervall, float dur, bool emitedInWorldSpace) : GameObject(textureIndex, scale, posX, posY, posZ, dur,tPos),
	emitedObject(emits),
	emitedDirection(dirX, dirY, dirZ),
	maxCount(count),
	intervall(tIntervall),
	emittedCount(0),
	emitsInWorldSpace(emitedInWorldSpace),
	nextEmit(0),
	tEndEmit(0)

{
	myVertex.TextureIndex = textureIndex;
	emittedType = emits->GetObjectType();
}
 
ParticleSystem::ParticleSystem(std::string& m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType tPos, GameObject* emits,  float dirX, float dirY, float dirZ, size_t count, float tIntervall, float dur, bool emitedInWorldSpace) : GameObject(m, posX, posY, posZ, scale, rotX, rotY, rotZ, dur,tPos),
	emitedObject(emits),
	emitedDirection(dirX, dirY, dirZ),
	maxCount(count),
	intervall(tIntervall),
	emittedCount(0),
	emitsInWorldSpace(emitedInWorldSpace),
	nextEmit(0),
	tEndEmit(0)
{
	emittedType = emits->GetObjectType();
}


void ParticleSystem::StartEmit(double gameTime)
{
	tEndEmit = gameTime+duration;
	//g_activeParticleSystems.push_back(this);
}

void ParticleSystem::EndEmit(double gameTime)
{
	tEndEmit = gameTime;
}

void ParticleSystem::Emit(double gameTime)
{
	if(tEndEmit > gameTime && emittedCount < maxCount && nextEmit < gameTime)
	{
		nextEmit = gameTime+intervall;
		GameObject* p = emitedObject->Clone();
		D3DXVECTOR3 start = *GetPosition() + *emitedObject->GetPosition();
		p->TranslateTo(start);
		emittedParticles.push_back(p);
		emittedCount++;
		mapLifetimeParticles.push_back(std::pair<double, GameObject*>(gameTime+duration, p));
	}
}

void ParticleSystem::OnMove(double time, float elapsedTime)
{
	GameObject::OnMove(time, elapsedTime);
	for each(auto it in emittedParticles)
		it->OnMove(time, elapsedTime);
}

bool ParticleSystem::CheckParticles(double gameTime)
{
	for(int i = emittedParticles.size()-1; i >= 0; i--)
	{
		if(mapLifetimeParticles[i].first < gameTime)
		{
			SAFE_DELETE(emittedParticles[i]);
			mapLifetimeParticles.erase(mapLifetimeParticles.begin()+i); //sollte immer das ende der Liste sein darum nicht teuer
			emittedParticles.erase(emittedParticles.begin()+i);
		}
		else//falls gameTime >= sind alle anderen particle später instantiert und können übersprungen werden
			break;
	}
	if(emittedParticles.size() == 0 && tEndEmit < gameTime)
	{
		return true;
	}
	return false;
}


ParticleSystem* ParticleSystem::Clone()
{
	return new ParticleSystem(this);
}


ParticleSystem::~ParticleSystem(void)
{
	SAFE_DELETE(emitedObject);
}
