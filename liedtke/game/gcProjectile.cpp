#include "gcProjectile.h"
#include "Enemy.h"

gcProjectile::gcProjectile(int d, float s, float c) : dmg(d),
	speed(s),
	cooldown(c)
{
	myType = GameComponent::tProjectile;
}


gcProjectile::~gcProjectile(void)
{
}

void gcProjectile::OnCreate(const void* sender) const
{
}

void gcProjectile::OnDestroy(const void* sender) const
{
}
	void gcProjectile::OnMove(const void* sender, double time, float elapsedTime) const
	{
	}

void gcProjectile::OnHit(const void* sender, GameComponent* o) const
{
}