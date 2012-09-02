#include "GameObject.h"
#include "MeshRenderer.h"

#include "SphereCollider.h"

#include "Macros.h"

//shallow Copy
GameObject::GameObject(GameObject* o) : tObject(o->tObject),
	tPosition(o->tPosition),
	velocity(o->velocity),
	lookDirection(o->lookDirection),
	mMeshOirentation(o->mMeshOirentation),
	colliderSize(0),
	mAnim(o->mAnim),
	mTranslation(o->mTranslation),
	mRotate(o->mRotate),
	mScale(o->mScale),
	worldMatrix(o->worldMatrix),
	myMesh(o->myMesh),
	myVertex(o->myVertex)
{
	//cheat to deep Copy
	for each(GameComponent* c in o->Components)
		//TODO verschiedene typen
		AddComponent(new SphereCollider(*static_cast<SphereCollider*>(c)));
}

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

GameObject* GameObject::Clone()
{
	return new GameObject(this);
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

void GameObject::OnCreate()
{
	for(auto it = Components.begin(); it != Components.end(); it++)
	{
		(*it)->OnCreate();
	}
}

void GameObject::OnMove(double time, float elapsedTime)
{
	for(auto it = Components.begin(); it != Components.end(); it++)
	{
		(*it)->OnMove(time, elapsedTime);
	}
	Translate(velocity.x*elapsedTime, velocity.y*elapsedTime, velocity.z*elapsedTime);
}

void GameObject::OnHit(Particle* p)
{
	for(auto it = Components.begin(); it != Components.end(); it++)
	{
		(*it)->OnHit(NULL);
	}
}

void GameObject::OnDestroy()
{
	for(auto it = Components.begin(); it != Components.end(); it++)
	{
		(*it)->OnDestroy();
	}
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

void GameObject::AddComponent(GameComponent* component)
{
	Components.push_back(component);
}

GameObject::~GameObject(void)
{
	for(auto it = Components.begin(); it != Components.end(); it++)
	{
		SAFE_DELETE(*it);
	}
	Components.clear();
}
