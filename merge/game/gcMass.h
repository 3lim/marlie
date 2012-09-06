#pragma once
#include "gamecomponent.h"
#include "GameObject.h"

static const float g_gravityModifier = 0.15f;
class gcMass :
	public GameComponent
{
public:
	gcMass(float mass);
	virtual ~gcMass(void);
	void OnCreate(const void* sender, double gameTime) const;
	void OnDestroy(const void* sender) const;
	void OnMove(const void* sender, double time, float elapsedTime);
	void OnHit(const void* sender, const void* oponentGameObject) const;
private:
	float mass;
};

