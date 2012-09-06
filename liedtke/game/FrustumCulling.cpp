#include "FrustumCulling.h"
#include "gcSphereCollider.h"

FrustumCulling::FrustumCulling(void)
{
}


void FrustumCulling::CalculateFrustum(D3DXMATRIX* viewProj)
{
		//matrix = *proj;
	//// Calculate the minimum Z distance in the frustum.
	//zMinimum = -proj->_43 / proj->_33;
	////screendepth
	//r = (cameraVP.MaxDepth - cameraVP.MinDepth) / (cameraVP.MaxDepth - cameraVP.MinDepth - zMinimum);
	//matrix._33 = r;
	//matrix._43 = -r * zMinimum;

	// Create the frustum matrix from the view matrix and updated projection matrix.
	// D3DXMatrixMultiply(&matrix, view, &matrix);
	//matrix = static_cast<D3DXMATRIX>(viewProj);

	// Calculate near plane of frustum.
	frustum[0].a = viewProj->_14 + viewProj->_13;
	frustum[0].b = viewProj->_24 + viewProj->_23;
	frustum[0].c = viewProj->_34 + viewProj->_33;
	frustum[0].d = viewProj->_44 + viewProj->_43;
	D3DXPlaneNormalize(&frustum[0], &frustum[0]);

	// Calculate far plane of frustum.
	frustum[1].a = viewProj->_14 - viewProj->_13; 
	frustum[1].b = viewProj->_24 - viewProj->_23;
	frustum[1].c = viewProj->_34 - viewProj->_33;
	frustum[1].d = viewProj->_44 - viewProj->_43;
	D3DXPlaneNormalize(&frustum[1], &frustum[1]);

	// Calculate left plane of frustum.
	frustum[2].a = viewProj->_14 + viewProj->_11; 
	frustum[2].b = viewProj->_24 + viewProj->_21;
	frustum[2].c = viewProj->_34 + viewProj->_31;
	frustum[2].d = viewProj->_44 + viewProj->_41;
	D3DXPlaneNormalize(&frustum[2], &frustum[2]);

	// Calculate right plane of frustum.
	frustum[3].a = viewProj->_14 - viewProj->_11; 
	frustum[3].b = viewProj->_24 - viewProj->_21;
	frustum[3].c = viewProj->_34 - viewProj->_31;
	frustum[3].d = viewProj->_44 - viewProj->_41;
	D3DXPlaneNormalize(&frustum[3], &frustum[3]);

	// Calculate top plane of frustum.
	frustum[4].a = viewProj->_14 - viewProj->_12; 
	frustum[4].b = viewProj->_24 - viewProj->_22;
	frustum[4].c = viewProj->_34 - viewProj->_32;
	frustum[4].d = viewProj->_44 - viewProj->_42;
	D3DXPlaneNormalize(&frustum[4], &frustum[4]);

	// Calculate bottom plane of frustum.
	frustum[5].a = viewProj->_14 + viewProj->_12;
	frustum[5].b = viewProj->_24 + viewProj->_22;
	frustum[5].c = viewProj->_34 + viewProj->_32;
	frustum[5].d = viewProj->_44 + viewProj->_42;
	D3DXPlaneNormalize(&frustum[5], &frustum[5]);

}

bool FrustumCulling::IsObjectInFrustum(GameObject* o)
{
	std::vector<GameComponent*>* v = o->GetComponent(GameComponent::tSphereCollider);
	if(v->size() == 0)
		return true;
	float r = static_cast<gcSphereCollider*>((*v)[0])->GetSphereRadius();
	for(int i=0; i<6; i++) 
	{
		if(D3DXPlaneDotCoord(&frustum[i], o->GetPosition()) < -r)
		{
			return false;
		}
	}
	return true;
}

FrustumCulling::~FrustumCulling(void)
{
}
