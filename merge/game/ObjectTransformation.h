#pragma once
#include <DXUT.h>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cstdint>

class ObjectTransformation
{
public:
	ObjectTransformation(std::string name, bool isCameraObject, float scale, float rotX, float rotY, float rotZ, float transX, float transY, float transZ);
	ObjectTransformation(std::string name, bool isCameraObject, float scale, float rotX, float rotY, float rotZ, float transX, float offsetY, float transZ, bool automaticHeight);
	virtual ~ObjectTransformation(void);
	std::string getName() { return o_name;};
	void getTranslation(D3DXVECTOR3* pout);
	void getTranslation(D3DXMATRIX* pout);
	void getTranslation(float* poutX, float* poutY, float* poutZ);
	float getTranslationX() { return o_TransM._41;}
	float getTranslationY() { return o_TransM._42;}
	float getTranslationZ() { return o_TransM._43;}
	D3DXVECTOR3 getPosition() { return D3DXVECTOR3(o_TransM._41, o_TransM._42, o_TransM._43);}
	D3DXVECTOR3 getPositionInWorldSpace(D3DXMATRIX* invViewMatrix);
	void getScale(D3DXMATRIX* pout);
	void getScale(float* poutS);
	float getScale() { return o_ScaleM._11;}
	void getRotation(D3DXMATRIX* pout);
	void getRotation(float* poutX, float* poutY, float* poutZ);
	bool isCameraView() {return o_isCameraObject;}
	bool automaticHeight() { return o_AutomaticHeight;}
	void translate(float x, float y, float z);
	void scale(float s);
	void rotate(float x, float y, float z);
	void translateTo(float x, float y, float z);
	void calculateWorldMatrix();
	D3DXMATRIX g_World;
private:
	std::string o_name;
	bool o_isCameraObject, o_AutomaticHeight;
	float o_Scale;
	float o_RotX, o_RotY, o_RotZ;
	float o_TransX, o_TransY, o_TransZ;
	D3DXMATRIX o_TransM, o_RotM, o_ScaleM;
};