#include "gcMass.h"


gcMass::gcMass(float m): mass(m)
{
	myType = GameComponent::tMass;
}


gcMass::~gcMass(void)
{
}

void gcMass::OnCreate(const void* sender) const
{
}

void gcMass::OnDestroy(const void* sender) const
{
}

void gcMass::OnMove(const void* sender,double time, float elapsedTime) const
{
	((GameObject*)sender)->AddForce(mass*g_gravityModifier, D3DXVECTOR3(0,-1,0));
}

void gcMass::OnHit(const void* sender, GameComponent* o) const
{
}