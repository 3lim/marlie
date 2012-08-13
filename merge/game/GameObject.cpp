#include "GameObject.h"

GameObject::GameObject(void)
	: m_Position(0,0,0)
{
}
GameObject::GameObject(float x, float y, float z)
	: m_Position(x,y,z)
{
}

void GameObject::translate(D3DXVECTOR3 v)
{
	m_Position += v;
}

void GameObject::translate(float x, float y, float z)
{
	m_Position.x +=x;
	m_Position.y += y;
	m_Position.z += z;
}

GameObject::~GameObject(void)
{
}
