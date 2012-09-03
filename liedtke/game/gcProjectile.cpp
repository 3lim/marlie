#include "gcProjectile.h"
#include "Enemy.h"

gcProjectile::gcProjectile(int d, float s) : dmg(d),
	speed(s)
{
	myType = GameComponent::tProjectile;
}


gcProjectile::~gcProjectile(void)
{
}

void gcProjectile::OnCreate() const
{
}

void gcProjectile::OnDestroy() const
{
}
	void gcProjectile::OnMove(double time, float elapsedTime) const
	{
	}

void gcProjectile::OnHit(GameComponent* o) const
{
}