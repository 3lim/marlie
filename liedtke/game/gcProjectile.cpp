#include "gcProjectile.h"
#include "ParticleSystem.h"
#include "SphereCollider.h"

gcProjectile::gcProjectile(int d, float s, float c, std::string create, std::string death) : dmg(d),
	speed(s),
	cooldown(c),
	fTime(0.0),
	createEffect(create),
	deathEffect(death)
{
	myType = GameComponent::tProjectile;
}


gcProjectile::~gcProjectile(void)
{
}

void gcProjectile::OnCreate(const void* sender, double gameTime) const
{
	if(createEffect.length() > 0){
		GameObject* object = (GameObject*)sender;
		ParticleSystem::g_activeParticleSystems.push_front(ParticleSystem::g_ParticleSystems[createEffect]->Clone());
		(*ParticleSystem::g_activeParticleSystems.begin())->TranslateTo(*object->GetPosition());
		(*ParticleSystem::g_activeParticleSystems.begin())->StartEmit(gameTime);
	}
}

void gcProjectile::OnDestroy(const void* sender) const
{
	if(deathEffect.length() > 0){
		GameObject* object = (GameObject*)sender;
		ParticleSystem::g_activeParticleSystems.push_front(ParticleSystem::g_ParticleSystems[deathEffect]->Clone());
		(*ParticleSystem::g_activeParticleSystems.begin())->TranslateTo(*object->GetPosition());
		(*ParticleSystem::g_activeParticleSystems.begin())->StartEmit(fTime);
	}
}
void gcProjectile::OnMove(const void* sender, double time, float elapsedTime)
{
	fTime = time;//nicht schön aber warum nicht?
}

void gcProjectile::OnHit(const void* sender, const void* gameObject) const
{
	GameObject* oponent = (GameObject*)gameObject;
	SphereCollider* sp = (SphereCollider*)oponent->GetComponent(GameComponent::tSphereCollider)->at(0);
	if( sp->GetHitEffect().length() > 0){
		ParticleSystem::g_activeParticleSystems.push_front(ParticleSystem::g_ParticleSystems[sp->GetHitEffect()]->Clone());
		(*ParticleSystem::g_activeParticleSystems.begin())->TranslateTo(*oponent->GetPosition());
		(*ParticleSystem::g_activeParticleSystems.begin())->StartEmit(fTime);
	}
}