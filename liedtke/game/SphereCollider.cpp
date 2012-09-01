#include "SphereCollider.h"


SphereCollider::SphereCollider(float s) : sphereSize(s)
	
{
	myType = GameComponent::SphereCollider;
}


SphereCollider::~SphereCollider(void)
{
	myType = GameComponent::SphereCollider;
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
