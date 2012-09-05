#include "SphereCollider.h"


SphereCollider::SphereCollider(float s) : sphereSize(s)
{
	myType = GameComponent::tSphereCollider;
}


SphereCollider::~SphereCollider(void)
{
}

void SphereCollider::OnCreate(const void* sender) const
{
	//nothing
}

void SphereCollider::OnMove(const void* sender, double t, float elapsedT) const
{
	//nothing
}

void SphereCollider::OnHit(const void* sender, GameComponent* object) const
{
	//todo
}

void SphereCollider::OnDestroy(const void* sender) const
{
	//nothing
}
