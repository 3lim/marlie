#pragma once
#include <DXUT.h>
#include "GameObject.h"

class FrustumCulling
{
public:
	FrustumCulling(void);
	void CalculateFrustum(D3DXMATRIX* viewProj);
	bool IsObjectInFrustum(GameObject* object);
	~FrustumCulling(void);
private:
	D3DXPLANE frustum[6];
};

