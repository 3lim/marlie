#include "SphereCollider.h"


SphereCollider::SphereCollider(float s, std::string hit) : sphereSize(s),
	hitEffect(hit)
{
	myType = GameComponent::tSphereCollider;
}


SphereCollider::~SphereCollider(void)
{
}

void SphereCollider::OnCreate(const void* sender, double gameTime) const
{
	//nothing
}

void SphereCollider::OnMove(const void* sender, double t, float elapsedT)
{
	//nothing
}

void SphereCollider::OnHit(const void* sender,  const void* oponentGameObject) const
{
	//todo
}

void SphereCollider::OnDestroy(const void* sender) const
{
	//nothing
}
