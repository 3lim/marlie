#pragma once

class GameComponent
{
public:
	//no perfect workaround for instanceof but a fast way
	enum componentType
	{
		tSphereCollider
	};
	virtual ~GameComponent() {};
	virtual void OnCreate() const = 0;
	virtual void OnDestroy() const = 0;
	virtual void OnMove(double time, float elapsedTime) const = 0;
	virtual void OnHit(GameComponent* collider) const = 0;
	virtual componentType GetType() { return myType; }
protected:
	componentType myType;
};