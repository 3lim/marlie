#pragma once
#include "GameComponent.h"
#include "GameObject.h"
class gcSphereCollider :
	public GameComponent
{
public:
	gcSphereCollider(float SphereSize, std::string hitEffect = "");
	virtual ~gcSphereCollider(void);
	void OnCreate(const void* sender, double gameTime) const;
	void OnMove(const void* sender, double time, float elapsedTime) ;
	void OnHit(const void* sender, const void* oponentGameObject) const;
	void OnDestroy(const void* sender) const;
	float GetSphereRadius() { return sphereSize; }
	std::string GetHitEffect() { return hitEffect; }
private:
	float sphereSize;
	std::string hitEffect;
};

