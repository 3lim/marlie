#pragma once
#include "gamecomponent.h"
#include "GameObject.h"
class gcProjectile :
	public GameComponent
{
public:
	gcProjectile(int damage, float speed, float cooldown);
	virtual ~gcProjectile(void);
	void OnCreate(const void* sender) const;
	void OnDestroy(const void* sender) const;
	void OnMove(const void* sender, double time, float elapsedTime) const;
	void OnHit(const void* sender, GameComponent* collider) const;
	int GetDamage() { return dmg; }
	float GetSpeed() { return speed; }
	float GetCooldown() { return cooldown; }
private:
	int dmg;
	float speed;
	float cooldown;
};

