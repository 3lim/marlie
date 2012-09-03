#pragma once
#include "SpriteRenderer.h"
#include "ParticleEffect.h"
#include "GameComponent.h"
#include <map>
struct MeshObject
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Scale;
	D3DXCOLOR Color;
	std::string Name;
	Mesh* MeshToRender;
};

class GameObject
{
	public:
		enum ObjectType{
		MESH,
		SPRITE
	};

	enum PositionType{
		WORLD,
		TERRAIN,
		CAMERA
	};
	GameObject(GameObject* toClone);
	GameObject(SpriteVertex v, PositionType relativeTo);
	GameObject(Mesh* m, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType relativeTo);
	GameObject(std::string& meshName, float& posX, float& posY, float& posZ, float& scale, float& rotX, float& rotY, float& rotZ, PositionType relativeTo);
	virtual ~GameObject(void);

	GameObject* Clone();

	//Setzt Die Farbe, welche über die Textur geblendet werden soll a = 0 == keine Farbveränderung
	void SetColor(D3DXCOLOR& c) { tObject == MESH ? myMesh.Color = c : myVertex.Color = c;}
	void Scale(float s); //addition on scale
	void ScaleTo(float s); //Set scale to
	void Translate(float x, float y, float z); //addition
	void TranslateTo(float x, float y, float z); //set
	void Rotate(float x, float y, float z);//addition
	void RotateTo(float x, float y, float z);//set
	void OnCreate();
	void OnMove(double time, float elapsedTime);
	void OnHit(GameObject* p);
	void OnDestroy();

	void SetMeshOrientation(float rotX, float rotY, float rotZ, float transX = 0.f, float transY = 0.f, float transZ = 0.f) 
		{mMeshOirentation = D3DXMATRIX(rotX, rotY, rotZ,0,
				0,1,0,0,
				-rotZ, rotY, rotX,0,
				transX,transY,transZ,1);}
	//Fügt eine auf das GO wirkende Kraft hinzu
	void AddForce(float power, float& directionX, float& directionY, float& directionZ);
	//Fügt eine auf das GO wirkende Kraft hinzu, NormDir muss normalisiert sein!
	void AddForce(float power, D3DXVECTOR3& NormDir);
	void CalculateWorldMatrix(); //should calculate one time the Worldmatrix (per Frame) to improve Performace
	D3DXVECTOR3* GetPosition() { return tObject==MESH ? &myMesh.Position : &myVertex.Position; }
	D3DXMATRIX* GetWorld() { return &worldMatrix; }
	float GetScale() { return tObject == MESH ? myMesh.Scale.x : myVertex.Radius; }
	ObjectType GetObjectType() { return tObject;}
	PositionType GetRelativePosition() { return tPosition; }
	Mesh* GetMesh() { return myMesh.MeshToRender; }
	std::string* GetName() { return tObject == MESH ? &myMesh.Name : NULL; }
	SpriteVertex* GetSprite() { return &myVertex; }

	void AddComponent(GameComponent* c);
	std::vector<GameComponent*>* GetComponent(GameComponent::componentType cType);
protected:
	//std::vector<GameComponent*> Components;
	std::map<GameComponent::componentType, std::vector<GameComponent*>> myComponents;
	D3DXVECTOR3 lookDirection;
	SpriteVertex myVertex;
	MeshObject myMesh;
private:

	inline void calcScale();
	inline void calcTranslation();
	inline void calcAnim();
	PositionType tPosition;
	ObjectType tObject;
	D3DXVECTOR3 velocity;

	D3DXMATRIX worldMatrix;
	D3DXMATRIX mScale;
	D3DXMATRIX mRotate;
	D3DXMATRIX mTranslation;
	D3DXMATRIX mAnim;
	D3DXMATRIX mMeshOirentation;
};

