#pragma once
#include "DXUT\Core\DXUT.h"
class GameObject
{
public:
	GameObject(void);
	GameObject(float x, float y, float z);
	~GameObject(void);
	void translate(float x, float y, float z);
	void translate(D3DXVECTOR3 v);
	D3DXVECTOR3* getPosition() { return &m_Position;}

private:
	D3DXVECTOR3 m_Position;
};

