#pragma once
#include "SpriteRenderer.h"

class Particle : public SpriteVertex
{
public:
	Particle(void);
	Particle(float speed, float mass, D3DXVECTOR3 initDirection);
	~Particle(void);
	void move(float fElapsedTime);
	D3DXVECTOR3 getPosition() { return Position;}
	int getDamage() { return m_Damage; }
	void setVertexPosition(D3DXVECTOR3 p);
	void setVertexSize(float r);
	void setVertexTexindex(int i);
	void setSpeed(float s);
	void setMass(float m);
	void setDirection(D3DXVECTOR3 d);
	void setDamage(int);
	float getSize() {return Radius;}
	void onHit();
	void Destroy();
private:
	int m_Damage;
	float m_Mass;
	float m_initSpeed;
	D3DXVECTOR3 m_Velocity;
	D3DXVECTOR3 m_Direction;
};

