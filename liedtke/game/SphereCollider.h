#pragma once
#include "GameComponent.h"
#include "GameObject.h"
class SphereCollider :
	public GameComponent
{
public:
	SphereCollider(float SphereSize, std::string hitEffect = "");
	virtual ~SphereCollider(void);
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

