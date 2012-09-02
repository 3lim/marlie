#pragma once
#include "GameComponent.h"
class SphereCollider :
	public GameComponent
{
public:
	SphereCollider(float SphereSize);
	virtual ~SphereCollider(void);
	void OnCreate() const;
	void OnMove(double time, float elapsedTime) const;
	void OnHit(GameComponent* collider) const;
	void OnDestroy() const;
	float GetSphereRadius() { return sphereSize; }
private:
	float sphereSize;
};

