#pragma once

class GameComponent
{
public:
	virtual ~GameComponent() {};
	virtual void OnCreate() const = 0;
	virtual void OnDestroy() const = 0;
	virtual void OnMove(double time, float elapsedTime) const = 0;
	virtual void OnHit(GameComponent* collider) const = 0;
};