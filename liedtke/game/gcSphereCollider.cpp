#include "gcSphereCollider.h"


gcSphereCollider::gcSphereCollider(float s, std::string hit) : sphereSize(s),
	hitEffect(hit)
{
	myType = GameComponent::tSphereCollider;
}


gcSphereCollider::~gcSphereCollider(void)
{
}

void gcSphereCollider::OnCreate(const void* sender, double gameTime) const
{
	//nothing
}

void gcSphereCollider::OnMove(const void* sender, double t, float elapsedT)
{
	//nothing
}

void gcSphereCollider::OnHit(const void* sender,  const void* oponentGameObject) const
{
	//todo
}

void gcSphereCollider::OnDestroy(const void* sender) const
{
	//nothing
}
