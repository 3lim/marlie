#pragma once
#include <DXUT.h>
#include "GameObject.h"

class FrustumCulling
{
public:
	FrustumCulling(void);
	void CalculateFrustum(D3DXMATRIX* viewProj, const D3DXMATRIX* View);
	bool IsObjectInFrustum(GameObject* object);
	~FrustumCulling(void);
private:
	D3DXPLANE frustum[6];
	float FrustumgSphereRadius;
	D3DXVECTOR3 FrustumCenter;
};

