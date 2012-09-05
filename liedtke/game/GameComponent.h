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
	virtual void OnCreate(const void* sender, double gameTime) const = 0;
	virtual void OnDestroy(const void* sender) const = 0;
	virtual void OnMove(const void* sender, double time, float elapsedTime) = 0;
	virtual void OnHit(const void* sender, const void* oponentGameObject) const = 0;
	virtual componentType GetType() { return myType; }
protected:
	componentType myType;
};