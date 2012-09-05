#include "GameObject.h"
#include "MeshRenderer.h"

#include "SphereCollider.h"
#include "gcProjectile.h"
#include "gcMass.h"

#include "Macros.h"

GameObject::GameObject(void) : tPosition(GameObject::WORLD),
	tObject(GameObject::SPRITE),
	velocity(0,0,0),
	worldMatrix(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	duration(1.f)
{
	myVertex.AnimationProgress = 0;
	myVertex.AnimationSize = 1;
	myVertex.TextureIndex = 0;
	myVertex.Opacity = 1;
	SetColor(D3DXCOLOR(0,0,0,0));
	RotateTo(0,0,0);
	TranslateTo(0,0,0);
	ScaleTo(0.f);
}

//shallow Copy
GameObject::GameObject(GameObject* o) : tObject(o->tObject),
	tPosition(o->tPosition),
	velocity(o->velocity),
	lookDirection(o->lookDirection),
	mMeshOirentation(o->mMeshOirentation),
	mAnim(o->mAnim),
	mTranslation(o->mTranslation),
	mRotate(o->mRotate),
	mScale(o->mScale),
	worldMatrix(o->worldMatrix),
	myMesh(o->myMesh),
	myVertex(o->myVertex),
	duration(o->duration)
{
	//cheat to deep Copy
	for(auto it = o->myComponents.begin(); it != o->myComponents.end(); it++){
		//TODO verschiedene typen
		GameComponent::componentType t = it->first;
		for each(GameComponent* c in it->second)
			switch(t){
			case(GameComponent::tSphereCollider):
					AddComponent(new SphereCollider(*static_cast<SphereCollider*>(c)));
					break;
			case(GameComponent::tProjectile):
				AddComponent(new gcProjectile(*static_cast<gcProjectile*>(c)));
				break;
			case(GameComponent::tMass):
				AddComponent(new gcMass(*static_cast<gcMass*>(c)));
				break;
		}
	}
}

GameObject::GameObject(int textureIndex, float scale, float posX, float posY, float posZ, float dur, PositionType tPos) :
//	myMesh(NULL),
	//position(&v.Position),
	//color(&v.Color),
	tObject(SPRITE),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	duration(dur)
{
	myVertex.TextureIndex = textureIndex;
	myVertex.AnimationSize = 0;
	myVertex.AnimationProgress = 0;
	myVertex.Opacity = 1.f;
	SetColor(D3DXCOLOR(0,0,0,0));
	ScaleTo(scale);
	TranslateTo(posX,posY,posZ);
	RotateTo(0,0,0);
}

GameObject::GameObject(Mesh* m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, float dur,PositionType tPos) :
	//position(NULL),
	//color(&myMesh.Color),
	tObject(MESH),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	duration(dur)
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
GameObject::GameObject(std::string& m, float& posX, float& posY, float& posZ, float scale, float rotX, float rotY, float rotZ, float dur,PositionType tPos) :
//position(NULL),
	//color(&myMesh.Color),
	tObject(MESH),
	tPosition(tPos),
	velocity(0,0,0),
	lookDirection(1,0,0),
	mMeshOirentation(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
	duration(dur)
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
void GameObject::TranslateTo(D3DXVECTOR3& p)
{
	if(tObject == MESH){
		myMesh.Position = p;
	} else {
		myVertex.Position = p;
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

void GameObject::OnCreate(double gameTime)
{
	for(auto Components = myComponents.begin(); Components != myComponents.end(); Components++)
	for(auto it = Components->second.begin(); it != Components->second.end(); it++)
	{
		(*it)->OnCreate(this, gameTime);
	}
}

void GameObject::OnMove(double time, float elapsedTime)
{
	for(auto Components = myComponents.begin(); Components != myComponents.end(); Components++)
	for(auto it = Components->second.begin(); it != Components->second.end(); it++)
	{
		(*it)->OnMove(this, time, elapsedTime);
	}
	Translate(velocity.x*elapsedTime, velocity.y*elapsedTime, velocity.z*elapsedTime);
	if(tObject == SPRITE)
		//calculate SpriteProgress
		myVertex.AnimationProgress += elapsedTime/duration;

}

void GameObject::OnHit(GameObject* p)
{
	for(auto Components = myComponents.begin(); Components != myComponents.end(); Components++)
	for(auto it = Components->second.begin(); it != Components->second.end(); it++)
	{
		//todo
		(*it)->OnHit(this, p);
	}
}

void GameObject::OnDestroy()
{
	for(auto Components = myComponents.begin(); Components != myComponents.end(); Components++)
	for(auto it = Components->second.begin(); it != Components->second.end(); it++)
	{
		(*it)->OnDestroy(this);
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
	myComponents[component->GetType()].push_back(component);
}

std::vector<GameComponent*>* GameObject::GetComponent(GameComponent::componentType t)
{
	//vermutlich bei vielen Components langsam
	//std::vector<GameComponent*> result;
	//for(auto it = Components.begin(); it != Components.end(); it++)
	//{
	//	if((*it)->GetType() == t)
	//		result.push_back(*it);
	//}
	//return result;
	//ist map besser? ich hoffe es
	return &myComponents[t];
}
GameObject::~GameObject(void)
{
	for(auto it = myComponents.begin(); it != myComponents.end(); it++)
	{
		for(auto i = it->second.begin(); i != it->second.end(); i++)
			SAFE_DELETE(*i);
		it->second.clear();
	}
	myComponents.clear();
}
