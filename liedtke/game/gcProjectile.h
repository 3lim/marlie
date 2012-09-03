#pragma once
#include "gamecomponent.h"
class gcProjectile :
	public GameComponent
{
public:
	gcProjectile(int damage, float speed);
	virtual ~gcProjectile(void);
	void OnCreate() const;
	void OnDestroy() const;
	void OnMove(double time, float elapsedTime) const;
	void OnHit(GameComponent* collider) const;
	int GetDamage() { return dmg; }
	float GetSpeed() { return speed; }
private:
	int dmg;
	float speed;
};

