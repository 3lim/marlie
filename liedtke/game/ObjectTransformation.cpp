#include "ObjectTransformation.h"
#include <DXUT.h>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cstdint>


#define DEG2RAD( a ) ( (a) * D3DX_PI / 180.f )

ObjectTransformation::ObjectTransformation(std::string name, bool isCameraObject, float scale, float rotX, float rotY, float rotZ, float transX, float transY, float transZ) :
	o_name (name),
	o_isCameraObject (isCameraObject),
	o_Scale(scale),
	//o_RotX (rotX),
	//o_RotY (rotY),
	//o_RotZ (rotZ),
	//o_TransX(transX),
	//o_TransY(transY),
	//o_TransZ(transZ),
	o_AutomaticHeight(false)
	{
		D3DXMatrixTranslation(&o_TransM,  transX,  transY,transZ);
		D3DXMatrixRotationYawPitchRoll(&o_RotM,  DEG2RAD(rotY), DEG2RAD(rotX), DEG2RAD(rotZ));
		D3DXMatrixScaling(&o_ScaleM, scale, scale, scale);
	}
ObjectTransformation::ObjectTransformation(std::string name, bool isCameraObject, float scale, float rotX, float rotY, float rotZ, float transX, float offsetY, float transZ, bool automaticHeight) :
	o_name (name),
	o_isCameraObject (isCameraObject),
	o_Scale(scale),
	//o_RotX (rotX),
	//o_RotY (rotY),
	//o_RotZ (rotZ),
	//o_TransX(transX),
	//o_TransY(offsetY),
	//o_TransZ(transZ),
	o_AutomaticHeight(automaticHeight)
	{
		D3DXMatrixTranslation(&o_TransM, transX,offsetY,  transZ);
		D3DXMatrixRotationYawPitchRoll(&o_RotM,  DEG2RAD(rotY), DEG2RAD(rotX), DEG2RAD(rotZ));
		D3DXMatrixScaling(&o_ScaleM, scale, scale, scale);
}

void ObjectTransformation::getRotation(D3DXMATRIX* pout){
	//D3DXMATRIX x, y, z;
	//if(o_RotX >= 360 || o_RotX <= -360){
	//	o_RotX=0;
	//}
	//if(o_RotY >= 360 || o_RotY <= -360){
	//	o_RotY=0;
	//}
	//if(o_RotZ >= 360 || o_RotZ <= -360){
	//	o_RotZ=0;
	//}
	//D3DXMatrixRotationX(&x, D3DXToRadian(o_RotX));
	//D3DXMatrixRotationY(&y,  D3DXToRadian(o_RotY));
	//D3DXMatrixRotationZ(&z,  D3DXToRadian(o_RotZ));
	//*pout = x*y*z;
	*pout = o_RotM;
}
void ObjectTransformation::getRotation(float* x, float* y, float* z){
	//*x = o_RotX;
	//*y = o_RotY;
	//*z = o_RotZ;
	*x = o_RotM._11;
	*y = o_RotM._22;
	*z = o_RotM._33;
}

D3DXMATRIX tmp;
void ObjectTransformation::getScale(D3DXMATRIX* pout){
	D3DXMatrixScaling(pout, o_Scale, o_Scale, o_Scale);
}
void ObjectTransformation::getScale(float* s){
	*s = o_ScaleM._11;
}

void ObjectTransformation::getTranslation(float* x, float* y, float* z){
	//*x = o_TransX;
	//*y = o_TransY;
	//*z = o_TransZ;
	*x = o_TransM._41;
	*y = o_TransM._42;
	*z = o_TransM._43;
}

void ObjectTransformation::getTranslation(D3DXMATRIX* p)
{
	*p = o_TransM;
}
void ObjectTransformation::getTranslation(D3DXVECTOR3* p)
{
	//p->x= o_TransX; p->y= o_TransY; p->z = o_TransZ;
	p->x= o_TransM._41; p->y= o_TransM._42; p->z = o_TransM._43;
}

void ObjectTransformation::translate(float x, float y, float z){
	//o_TransX += x;
	//o_TransY += y;
	//o_TransZ += z;
	D3DXMatrixTranslation(&o_TransM, o_TransM._41+x, o_TransM._42+y, o_TransM._43+z);
}
void ObjectTransformation::scale(float s){
	D3DXMatrixScaling(&o_ScaleM, o_ScaleM._11+s, o_ScaleM._11+s, o_ScaleM._11+s);
}
void ObjectTransformation::rotate(float x, float y, float z){
	//o_RotX += x;
	//o_RotY += y;
	//o_RotZ += z;
	D3DXMatrixRotationYawPitchRoll(&tmp,   DEG2RAD(y), DEG2RAD(x), DEG2RAD(z));
	o_RotM *=tmp;
}

void ObjectTransformation::translateTo(float x, float y, float z){
	//o_TransX = x;
	//o_TransY = y;
	//o_TransZ = z;
	D3DXMatrixTranslation(&o_TransM, x,y,z);
}

void ObjectTransformation::calculateWorldMatrix()
{
	g_World = o_ScaleM*o_RotM*o_TransM;
}

ObjectTransformation::~ObjectTransformation(void)
{
}