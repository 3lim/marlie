#include "MeshRenderer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <fstream>

#include "DXUT/Core/dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "DXUT\Optional\SDKmisc.h"
#include "GameObject.h"

#include "Macros.h"
#include "debug.h"

using namespace std;

std::map<std::string, Mesh*> MeshRenderer::g_Meshes;
MeshRenderer::MeshRenderer(FrustumCulling* f) :
m_pEffect(NULL),
	m_RenderET(NULL),
	m_MeshInputLayout(NULL),
	m_pVertexBuffer(NULL),
	m_Pass1_Mesh(NULL),
	m_ShadowMesh(NULL),
	m_DiffuseEV(NULL),
	m_ShadowMapEV(NULL),
	m_GlowEV(NULL),
	m_SpecularEV(NULL),
	m_NormalEV(NULL),
	g_invView(NULL),
	g_LightViewProjMatrix(NULL),
	g_View(NULL),
	g_Proj(NULL),
	g_ViewProj(NULL)
{
	g_Frustum = f;

}

MeshRenderer::~MeshRenderer(void)
{
}

void MeshRenderer::setEffectVariables()
{
}

// void MeshRenderer::ShadowMeshes(ID3D11Device* pDevice, vector<GameObject*>* o, ID3D11RenderTargetView* LightBW)
//{
//	//stride = sizeof(T3dVertex);
//	//setEffectVariables();
//	//UINT offset = 0;
//	for(auto object = o->begin(); object != o->end(); object++)
//	{
//		RenderMesh(pDevice, *object, m_ShadowET,LightBW);
//	}
//}
//
// void MeshRenderer::ShadowMeshes(ID3D11Device* pDevice, list<Enemy*>* o, ID3D11RenderTargetView* LightBW)
//{
//	//stride = sizeof(T3dVertex);
//	//setEffectVariables();
//	//UINT offset = 0;
//	for(auto object = o->begin(); object != o->end(); object++)
//	{
//		RenderMesh(pDevice, *object, m_ShadowET,LightBW);
//	}
//}
//
// void MeshRenderer::RenderMeshes(ID3D11Device* pDevice, vector<GameObject*>* o, ID3D11RenderTargetView* LightBW)
//{
//	//stride = sizeof(T3dVertex);
//	//setEffectVariables();
//	//UINT offset = 0;
//	for(auto object = o->begin(); object != o->end(); object++)
//	{
//		RenderMesh(pDevice, *object, m_RenderET,LightBW);
//	}
//}
// void  MeshRenderer::RenderMeshes(ID3D11Device* pDevice, list<Enemy*>* o, ID3D11RenderTargetView* LightBW)
//{
//	//stride = sizeof(T3dVertex);
//	//setEffectVariables();
//	//UINT offset = 0;
//	for(auto object = o->begin(); object != o->end(); object++)
//	{
//		RenderMesh(pDevice, *object, m_RenderET,LightBW);
//	}
//}
//void inline MeshRenderer::RenderMesh(ID3D11Device* pDevice, ObjectTransformation* object, ID3DX11EffectTechnique* technique)
//{
//	mesh = MeshRenderer::g_Meshes[object->getName()];
//	vbs[0] = mesh->GetVertexBuffer();
//
//	pd3DContext->IASetInputLayout(m_MeshInputLayout);
//
//	m_DiffuseEV->SetResource(mesh->GetDiffuseSRV());
//	m_SpecularEV->SetResource(mesh->GetSpecularSRV());
//	m_GlowEV->SetResource(mesh->GetGlowSRV());
//	m_NormalEV->SetResource(mesh->GetNormalSRV());
//
//	WorldView = object->g_World;
//	WorldViewProjektion = WorldView;
//	//WorldLightViewProjMatrix = object->g_World*(*g_LightViewProjMatrix);
//	if(object->isCameraView()){
//		//WorldViewProjektion = object->g_World *(*g_Proj);
//		WorldView *= *g_invView;
//	} else {
//		WorldViewProjektion *= *g_View;
//	}
//	WorldViewProjektion *= *g_Proj;
//	D3DXMatrixInverse(&WorldViewNormals,0,&WorldView);
//	D3DXMatrixTranspose(&WorldViewNormals,&WorldViewNormals);
//
//	WorldLightViewProjMatrix = WorldView* *g_LightViewProjMatrix;
//	m_WorldViewEV->SetMatrix(WorldView);
//	m_WorldViewProjektionEV->SetMatrix(WorldViewProjektion);
//	m_WorldViewNormalsEV->SetMatrix(WorldViewNormals);
//	m_WorldLightViewProjMatrixEV->SetMatrix(WorldLightViewProjMatrix);
//	technique->GetPassByName("Mesh")->Apply(0, pd3DContext);
//	pd3DContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
//	pd3DContext->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexFormat(), 0);
//	pd3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	pd3DContext->DrawIndexed(mesh->GetIndexCount(), 0,0);
//}

void MeshRenderer::RenderMesh(ID3D11Device* pDevice, GameObject* object, RenderableTexture* shadowMap, RenderableTexture* vlsMap, bool drawShadow)
{
	if(!g_Frustum->IsObjectInFrustum(object))
		return;
	ID3DX11EffectTechnique* technique = drawShadow ? m_ShadowET : m_RenderET;
	mesh = object->GetMesh();
	vbs[0] = mesh->GetVertexBuffer();
	vbs[1] = mesh->GetInstanceBuffer();
	offset[0] = 0;
	offset[1] = 1;
	stride[0] = sizeof(T3dVertex);
	stride[1] = sizeof(MeshInstanceType);

	object->CalculateWorldMatrix();

	pd3DContext->IASetInputLayout(m_MeshInputLayout);

	m_DiffuseEV->SetResource(mesh->GetDiffuseSRV());
	m_SpecularEV->SetResource(mesh->GetSpecularSRV());
	m_GlowEV->SetResource(mesh->GetGlowSRV());
	m_NormalEV->SetResource(mesh->GetNormalSRV());

	m_LightColorEV->SetFloatVector(*g_LightColor);
	m_LightDirViewEV->SetFloatVector((float*)&g_LightDirView);
	if(shadowMap != NULL)
	m_pEffect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(shadowMap->GetShaderResource());
	m_LightViewProjMatrixEV->SetMatrix(*g_LightViewProjMatrix);

	WorldView = *object->GetWorld();
	WorldViewProjektion = WorldView;
	//WorldLightViewProjMatrix = object->g_World*(*g_LightViewProjMatrix);
	if(object->GetRelativePosition() == GameObject::CAMERA){
		//WorldViewProjektion = object->g_World *(*g_Proj);
		WorldView *= *g_invView;
	} else {
		WorldViewProjektion *= *g_View;
	}
	WorldViewProjektion *= *g_Proj;
	D3DXMatrixInverse(&WorldViewNormals,0,&WorldView);
	D3DXMatrixTranspose(&WorldViewNormals,&WorldViewNormals);

	WorldLightViewProjMatrix = WorldView* *g_LightViewProjMatrix;
	m_WorldViewEV->SetMatrix(WorldView);
	m_WorldViewProjektionEV->SetMatrix(WorldViewProjektion);
	m_WorldViewNormalsEV->SetMatrix(WorldViewNormals);
	m_WorldLightViewProjMatrixEV->SetMatrix(WorldLightViewProjMatrix);
	technique->GetPassByName("Mesh")->Apply(0, pd3DContext);
	pd3DContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
	pd3DContext->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexFormat(), 0);
	pd3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3DContext->DrawIndexedInstanced(mesh->GetIndexCount(), mesh->GetInstanceCount(),0, 0,0);
	//Light Scattering
	if(vlsMap != NULL)
	{
		ID3D11RenderTargetView* vlsTarget = vlsMap->GetRenderTarget();
		pd3DContext->OMSetRenderTargets(1, &vlsTarget, NULL);
		technique->GetPassByName("MeshBW")->Apply(0, pd3DContext);
		pd3DContext->DrawIndexedInstanced(mesh->GetIndexCount(), mesh->GetInstanceCount(),0, 0,0);
	}
}


HRESULT MeshRenderer::CreateResources(ID3D11Device* pDevice)
{
	for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->CreateResources(pDevice);

	pDevice->GetImmediateContext(&pd3DContext);
	Mesh::CreateInstanceLayout(pDevice, m_Pass1_Mesh, &m_MeshInputLayout);
	return S_OK;
}

HRESULT MeshRenderer::ReloadShader(ID3D11Device* pDevice)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"MeshRenderer.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V(D3DX11CreateEffectFromMemory(&effectBuffer[0], effectBuffer.size(), D3D11_CREATE_DEVICE_DEBUG, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", m_RenderET);
	SAFE_GET_TECHNIQUE(m_pEffect, "Shadow", m_ShadowET);

	SAFE_GET_PASS(m_RenderET, "Mesh", m_Pass1_Mesh);
	SAFE_GET_PASS(m_ShadowET, "Mesh", m_ShadowMesh);

	SAFE_GET_RESOURCE(m_pEffect, "Diffuse", m_DiffuseEV);
	SAFE_GET_RESOURCE(m_pEffect, "Normal", m_NormalEV);
	SAFE_GET_RESOURCE(m_pEffect, "Specular", m_SpecularEV);
	SAFE_GET_RESOURCE(m_pEffect, "Glow", m_GlowEV);
	//SAFE_GET_RESOURCE(m_pEffect, "g_ShadowMap", m_ShadowMapEV);

	SAFE_GET_MATRIX(m_pEffect, "WorldView", m_WorldViewEV);
	SAFE_GET_MATRIX(m_pEffect, "WorldViewProjection", m_WorldViewProjektionEV);
	SAFE_GET_MATRIX(m_pEffect, "WorldViewNormals", m_WorldViewNormalsEV);
	SAFE_GET_MATRIX(m_pEffect, "WorldLightViewProj", m_WorldLightViewProjMatrixEV);
	SAFE_GET_MATRIX(m_pEffect, "g_LightViewProjMatrix", m_LightViewProjMatrixEV);
	SAFE_GET_VECTOR(m_pEffect, "g_LightColor", m_LightColorEV);
	SAFE_GET_VECTOR(m_pEffect, "g_LightDirView", m_LightDirViewEV);
	//matrix g_ViewProjection; //MeshVS
	//matrix g_ViewNormals;
	//matrix g_LightWorldViewProjMatrix; // = g_mLightViewProj

return S_OK;
}

void MeshRenderer::ReleaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

void MeshRenderer::ReleaseResources()
{
	pd3DContext->Release();
	for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->ReleaseResources();
	SAFE_RELEASE(m_MeshInputLayout);
}

void MeshRenderer::Deinit()
{
	for(auto it=g_Meshes.begin(); it!= g_Meshes.end(); it++)
		SAFE_DELETE(it->second);
}
