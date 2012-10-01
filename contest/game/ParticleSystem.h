#pragma once
#include "gameobject.h"
#include "SpriteRenderer.h"
#include <list>
class ParticleSystem :
	public GameObject
{
public:
	static std::list<ParticleSystem*> g_activeParticleSystems;
	static std::map<std::string, ParticleSystem*> g_ParticleSystems;

	ParticleSystem(ParticleSystem* toClone);
	ParticleSystem(int textureIndex, float scale, float posX, float posY, float posZ, PositionType tPos, GameObject* emmits, float dirX, float dirY, float dirZ, float emitSpeed, size_t count, float tIntervall, float durationOfEmitter, bool emitedInWorldSpace = true);
	ParticleSystem(std::string& m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType tPos, GameObject* emmits, float dirX, float dirY, float dirZ, float emitSpeed, size_t count, float tIntervall, float durationOfEmitter, bool emitedInWorldSpace = true);
	virtual ~ParticleSystem(void);
	void EmitsInWorldSpace() { emitsInWorldSpace = true;}
	void EmitsInObjectSpace() { emitsInWorldSpace = false;}
	void Emit(double gameTime);
	void StartEmit(double gameTime);
	void EndEmit(double gameTime);
	void OnMove(double gameTime, float elapsedTime);
	ParticleSystem* Clone();
	std::vector<GameObject*>* DisplayedParticles() { return &emittedParticles; }
	//check if particles have to be removed, retunrs true if particleSystem is over
	bool CheckParticles(double gameTime);
	GameObject::ObjectType GetType() { return emittedType; }
	void SetEmittedAnimationSize(int size) { emitedObject->SetSpriteAnimationSize(size); }
	int GetEmittedAnimationSize() { return emitedObject->GetSpriteAnimationSize(); }
	void SetEmittedTextureIndex(int i) { emitedObject->SetTextureIndex(i); }
	int GetEmittedTextureIndex() { return emitedObject->GetTextureIndex(); }
	void AddSubsystem(std::string object) { toAddedSystems.push_back(object); }
	void SetEmitSpeed(float s) { emitSpeed = s; }
	inline static float  RandomAnlge(float a, float b);
private:
	std::vector<std::string> toAddedSystems;
	std::list<ParticleSystem*> mySubsystems;
	GameObject::ObjectType emittedType;
	GameObject* emitedObject;
	D3DXVECTOR3 emitedDirection;
	D3DXVECTOR3 emitOffset;
	size_t maxCount;
	float intervall;
	size_t emittedCount;
	//float duration; //of particle in seconds
	double nextEmit;
	double tEndEmit;
	bool emitsInWorldSpace;
	std::vector<std::pair<double, GameObject*>> mapLifetimeParticles;
	std::vector<GameObject*> emittedParticles;
	float emitSpeed;
};

