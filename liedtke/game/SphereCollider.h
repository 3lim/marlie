#pragma once
#include "GameComponent.h"
#include "GameObject.h"
class SphereCollider :
	public GameComponent
{
public:
	SphereCollider(float SphereSize);
	virtual ~SphereCollider(void);
	void OnCreate(const void* sender) const;
	void OnMove(const void* sender, double time, float elapsedTime) const;
	void OnHit(const void* sender, GameComponent* collider) const;
	void OnDestroy(const void* sender) const;
	float GetSphereRadius() { return sphereSize; }
private:
	float sphereSize;
};

