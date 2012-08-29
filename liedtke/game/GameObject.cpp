#include "GameObject.h"
#include "MeshRenderer.h"

#include "Macros.h"

GameObject::GameObject(SpriteVertex v, PositionType tPos) : myVertex(v),
//	myMesh(NULL),
	//position(&v.Position),
	//color(&v.Color),
	tObject(SPRITE),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	colliderSize(0)
{
	calcScale();
	calcTranslation();
	D3DXMatrixRotationYawPitchRoll(&mRotate, 0,0,0);
}

GameObject::GameObject(Mesh* m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType tPos) :
	//position(NULL),
	//color(&myMesh.Color),
	tObject(MESH),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	colliderSize(0)
{
	myMesh.MeshToRender = m;
	myMesh.Position =  D3DXVECTOR3(posX, posY, posZ);;
	//position = &myMesh.Position;
	myMesh.Color = D3DXCOLOR(0,0,0,0);
	//color = &myMesh.Color;
	myMesh.Scale = D3DXVECTOR3(scale, scale, scale);
	calcScale();
	calcTranslation();
	RotateTo(rotX, rotY, rotZ);
}
GameObject::GameObject(std::string& m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType tPos) :
//position(NULL),
	//color(&myMesh.Color),
	tObject(MESH),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	colliderSize(0)
{
	myMesh.Name = m;
	myMesh.MeshToRender = MeshRenderer::g_Meshes[m];
	myMesh.Position =  D3DXVECTOR3(posX, posY, posZ);;
	//position = &myMesh.Position;
	myMesh.Color = D3DXCOLOR(0,0,0,0);
	//color = &myMesh.Color;
	myMesh.Scale = D3DXVECTOR3(scale, scale, scale);
	calcScale();
	calcTranslation();
	RotateTo(rotX, rotY, rotZ);
}


void GameObject::Scale(float s)
{
	if(tObject == MESH){
		myMesh.Scale.x += s;
		myMesh.Scale.y += s;
		myMesh.Scale.z += s;
	} else {
		myVertex.Radius += s;
	}
	calcScale();
}

void GameObject::ScaleTo(float s)
{
	if(tObject == MESH){
		myMesh.Scale.x = s;
		myMesh.Scale.y = s;
		myMesh.Scale.z = s;
	} else {
		myVertex.Radius = s;
	}
	calcScale();
}

void GameObject::Translate(float x, float y, float z)
{
	//tObject==MESH ? position = &myMesh.Position : position = &myVertex.Position;
	if(tObject == MESH){
		myMesh.Position.x += x;
		myMesh.Position.y += y;
		myMesh.Position.z += z;
	} else {
		myVertex.Position.x += x;
		myVertex.Position.y += y;
		myVertex.Position.z += z;
	}
	calcTranslation();
}

void GameObject::TranslateTo(float x, float y, float z)
{
	if(tObject == MESH){
		myMesh.Position.x = x;
		myMesh.Position.y = y;
		myMesh.Position.z = z;
	} else {
		myVertex.Position.x = x;
		myVertex.Position.y = y;
		myVertex.Position.z = z;
	}
	calcTranslation();
}

void GameObject::Rotate(float x, float y, float z)
{
	D3DXMATRIX tmp;
	D3DXMatrixRotationYawPitchRoll(&tmp,   DEG2RAD(y), DEG2RAD(x), DEG2RAD(z));
	mRotate *=tmp;
}

void GameObject::RotateTo(float x, float y, float z)
{
	D3DXMatrixRotationYawPitchRoll(&mRotate,   DEG2RAD(y), DEG2RAD(x), DEG2RAD(z));
}

inline void GameObject::calcScale()
{
	tObject == MESH ? D3DXMatrixScaling(&mScale, myMesh.Scale.x, myMesh.Scale.y, myMesh.Scale.z) : D3DXMatrixScaling(&mScale, myVertex.Radius, myVertex.Radius, myVertex.Radius);
}
inline void GameObject::calcTranslation()
{
		tObject == MESH ? 
			D3DXMatrixTranslation(&mTranslation,  myMesh.Position.x, myMesh.Position.y,  myMesh.Position.z)
			: D3DXMatrixTranslation(&mTranslation,  myVertex.Position.x, myVertex.Position.y,  myVertex.Position.z);

}

void GameObject::CalculateWorldMatrix()
{
	worldMatrix = mScale*mMeshOirentation*mRotate*mTranslation;
}

void GameObject::OnMove(double time, float elapsedTime)
{
	Translate(velocity.x*elapsedTime, velocity.y*elapsedTime, velocity.z*elapsedTime);
}

void GameObject::AddForce(float power, float& x, float& y, float& z)
{
	D3DXVECTOR3 dir(x,y,z);
	D3DXVec3Normalize(&dir, &dir);
	dir*= power;
	D3DXVec3Add(&velocity, &velocity, &dir);
}
void GameObject::AddForce(float power, D3DXVECTOR3& dir)
{
	//dir muss vorher schon normiert sein!
	//D3DXVec3Normalize(&dir, &dir);
	dir*= power;
	D3DXVec3Add(&velocity, &velocity, &dir);
}

GameObject::~GameObject(void)
{
}
