#pragma once

class GameComponent
{
public:
	//no perfect workaround for instanceof but a fast way
	enum componentType
	{
		tSphereCollider,
		tProjectile,
		tMass
	};
	virtual ~GameComponent() {};
	virtual void OnCreate(const void* sender) const = 0;
	virtual void OnDestroy(const void* sender) const = 0;
	virtual void OnMove(const void* sender, double time, float elapsedTime) const = 0;
	virtual void OnHit(const void* sender, GameComponent* collider) const = 0;
	virtual componentType GetType() { return myType; }
protected:
	componentType myType;
};