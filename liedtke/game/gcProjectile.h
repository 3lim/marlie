#pragma once
#include "gamecomponent.h"
class gcProjectile :
	public GameComponent
{
public:
	gcProjectile(int damage);
	virtual ~gcProjectile(void);
	void OnCreate() const;
	void OnDestroy() const;
	void OnMove(double time, float elapsedTime) const;
	void OnHit(GameComponent* collider) const;
	int GetDamage() { return dmg; }
private:
	int dmg;
};

