#pragma once
#include "gamecomponent.h"
#include "GameObject.h"
class gcProjectile :
	public GameComponent
{
public:
	gcProjectile(int damage, float speed, float cooldown, std::string createEffect = "", std::string deathEffect = "");
	virtual ~gcProjectile(void);
	void OnCreate(const void* sender, double gameTime) const;
	void OnDestroy(const void* sender) const;
	void OnMove(const void* sender, double time, float elapsedTime);
	void OnHit(const void* sender,  const void* oponentGameObject) const;
	int GetDamage() { return dmg; }
	float GetSpeed() { return speed; }
	float GetCooldown() { return cooldown; }
private:
	int dmg;
	float speed;
	float cooldown;
	double fTime;
	std::string deathEffect;
	std::string createEffect;
};

