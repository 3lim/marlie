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
	emittedType(p->emittedType),
	emitSpeed(p->emitSpeed),
	toAddedSystems(p->toAddedSystems)
{
	emitedObject = p->emitedObject->Clone();
}

ParticleSystem::ParticleSystem(int textureIndex, float scale, float posX, float posY, float posZ, PositionType tPos, GameObject* emits,  float dirX, float dirY, float dirZ, float speed, size_t count, float tIntervall, float dur, bool emitedInWorldSpace) : GameObject(textureIndex, scale, posX, posY, posZ, dur,tPos),
	emitedObject(emits),
	emitedDirection(dirX, dirY, dirZ),
	maxCount(count),
	intervall(tIntervall),
	emittedCount(0),
	emitsInWorldSpace(emitedInWorldSpace),
	nextEmit(0),
	tEndEmit(0),
	emitSpeed(speed)

{
	myVertex.TextureIndex = textureIndex;
	emittedType = emits->GetObjectType();
}
 
ParticleSystem::ParticleSystem(std::string& m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType tPos, GameObject* emits,  float dirX, float dirY, float dirZ, float speed, size_t count, float tIntervall, float dur, bool emitedInWorldSpace) : GameObject(m, posX, posY, posZ, scale, rotX, rotY, rotZ, dur,tPos),
	emitedObject(emits),
	emitedDirection(dirX, dirY, dirZ),
	maxCount(count),
	intervall(tIntervall),
	emittedCount(0),
	emitsInWorldSpace(emitedInWorldSpace),
	nextEmit(0),
	tEndEmit(0),
	emitSpeed(speed)
{
	emittedType = emits->GetObjectType();
}


void ParticleSystem::StartEmit(double gameTime)
{
	tEndEmit = gameTime+duration;
	for each(auto it in toAddedSystems)
	{
		ParticleSystem* p = g_ParticleSystems[it]->Clone();
		mySubsystems.push_back(p); //nur als referenz für Particle Effekte, verwaltet wird es durch das Chilsystem
		p->TranslateTo(*GetPosition());
		p->StartEmit(gameTime);
		AddChild(p);
		g_activeParticleSystems.push_back(p);
	}
}

void ParticleSystem::EndEmit(double gameTime)
{
	tEndEmit = gameTime;
	for each(auto it in mySubsystems)
		it->EndEmit(gameTime);
}

void ParticleSystem::Emit(double gameTime)
{
	if(tEndEmit > gameTime && (emittedCount < maxCount || emittedCount == 0) && nextEmit < gameTime)
	{
		nextEmit = gameTime+intervall;
		GameObject* p = emitedObject->Clone();
		D3DXVECTOR3 start = *GetPosition() + *emitedObject->GetPosition();
		p->TranslateTo(start);
		//todo Emittrichtung beeinflussen funktioniert noch nicht wirklich
		p->AddForce(emitSpeed, D3DXVECTOR3(std::sin(RandomAnlge(0,180))*std::cos(RandomAnlge(0,360)), std::sin(RandomAnlge(0,180))*sin(RandomAnlge(0,360)), std::cos(RandomAnlge(0,180))));
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
			//sollte immer das ende der Liste sein darum nicht teuer
			mapLifetimeParticles.erase(mapLifetimeParticles.begin()+i);
			emittedParticles.erase(emittedParticles.begin()+i);
		}
		else
			//falls gameTime >= sind alle anderen particle später instantiert und können übersprungen werden
			break;
	}
	for each(auto child in mySubsystems)//falls ein childsystem noch aktiv ist bleibt das Objekt bestehen
		if(!child->CheckParticles(gameTime))
			return false;
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
	if(parent != NULL)
		((ParticleSystem*)parent)->mySubsystems.remove(this);
	for(auto it = emittedParticles.begin(); it != emittedParticles.end(); it++)
		SAFE_DELETE(*it);
	emittedParticles.end();
	SAFE_DELETE(emitedObject);
}

inline float ParticleSystem::RandomAnlge(float a, float b)
{
	return ((float)rand()/RAND_MAX)*(max(a,b)-min(a,b))+min(a,b);
}
