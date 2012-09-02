#include "SphereCollider.h"


SphereCollider::SphereCollider(float s) : sphereSize(s)
{
	myType = GameComponent::tSphereCollider;
}


SphereCollider::~SphereCollider(void)
{
}

void SphereCollider::OnCreate() const
{
	//nothing
}

void SphereCollider::OnMove(double t, float elapsedT) const
{
	//nothing
}

void SphereCollider::OnHit(GameComponent* object) const
{
	//todo
}

void SphereCollider::OnDestroy() const
{
	//nothing
}
