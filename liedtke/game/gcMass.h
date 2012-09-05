#pragma once
#include "gamecomponent.h"
#include "GameObject.h"

static const float g_gravityModifier = 0.7f;
class gcMass :
	public GameComponent
{
public:
	gcMass(float mass);
	virtual ~gcMass(void);
	void OnCreate(const void* sender) const;
	void OnDestroy(const void* sender) const;
	void OnMove(const void* sender, double time, float elapsedTime) const;
	void OnHit(const void* sender, GameComponent* collider) const;
private:
	float mass;
};

