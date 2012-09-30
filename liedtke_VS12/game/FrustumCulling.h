#pragma once
#include <DXUT.h>
#include "GameObject.h"

class FrustumCulling
{
public:
	FrustumCulling(void);
	void CalculateFrustum(D3D11_VIEWPORT vp, CFirstPersonCamera* cam);
	bool IsObjectInFrustum(GameObject* object);
	bool FrustumCulling::IsObjectInFrustum(D3DXVECTOR3* pos, int maxCount);
	bool FrustumCulling::IsObjectInFrustum(D3DXVECTOR3* pos);
	~FrustumCulling(void);
private:
	D3DXPLANE frustum[6];
	float FrustumgSphereRadius;
	D3DXVECTOR3 FrustumCenter;
};

