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

#include "Macros.h"

#include "MeshRenderer.h"

using namespace std;

std::map<std::string, Mesh*> MeshRenderer::g_Meshes;
MeshRenderer::MeshRenderer(void) :
m_pEffect(NULL),
	m_RenderET(NULL),
	m_pInputLayout(NULL),
	m_pVertexBuffer(NULL),
	//stride(0),
	offset(0)
{
	//stride = sizeof(T3dVertex);
	vbs[1] = NULL;
}

MeshRenderer::~MeshRenderer(void)
{
}


	//Effectvariablen
	ID3DX11EffectPass*						m_Pass1_Mesh = NULL;
	ID3DX11EffectShaderResourceVariable*	m_DiffuseEV = NULL;
	ID3DX11EffectShaderResourceVariable*	m_ShadowMapEV = NULL;
	ID3DX11EffectShaderResourceVariable*	m_GlowEV = NULL;
	ID3DX11EffectShaderResourceVariable*	m_SpecularEV = NULL;
	ID3DX11EffectShaderResourceVariable*	m_NormalEV = NULL;
	ID3DX11EffectMatrixVariable*			m_WorldEV = NULL;
	ID3DX11EffectScalarVariable*			m_isCameraObjectEV = NULL;
void MeshRenderer::RenderMeshes(ID3D11Device* pDevice, vector<ObjectTransformation>* o)
{
	stride = sizeof(T3dVertex);
	//UINT offset = 0;
	for(auto object = o->begin(); object != o->end(); object++)
	{
		RenderMesh(pDevice, object._Ptr);
	}
}
void MeshRenderer::RenderMeshes(ID3D11Device* pDevice, list<EnemyInstance>* o)
{
	stride = sizeof(T3dVertex);
	//UINT offset = 0;
	for(auto object = o->begin(); object != o->end(); object++)
	{
		RenderMesh(pDevice, object->getObject());
	}
}

void inline MeshRenderer::RenderMesh(ID3D11Device* pDevice, ObjectTransformation* object)
{
	mesh = MeshRenderer::g_Meshes[object->getName()];
	ID3D11Buffer* vbs[] = { mesh->GetVertexBuffer()};
	//vbs[0] = mesh->GetVertexBuffer();

	m_DiffuseEV->SetResource(mesh->GetDiffuseSRV());//benötigt: MeshPS
	m_SpecularEV->SetResource(mesh->GetSpecularSRV());//benötigt: MeshPS
	m_GlowEV->SetResource(mesh->GetGlowSRV());//benötigt: MeshPS
	m_NormalEV->SetResource(mesh->GetNormalSRV());//benötigt: MeshPS

	m_isCameraObjectEV->SetBool(object->isCameraView());
	m_WorldEV->SetMatrix((float*)object->g_World);

	m_Pass1_Mesh->Apply(0, pd3DContext);
	pd3DContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
	pd3DContext->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexFormat(), 0);
	pd3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3DContext->DrawIndexed(mesh->GetIndexCount(), 0,0);
}

HRESULT MeshRenderer::CreateResources(ID3D11Device* pDevice)
{
	for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->CreateResources(pDevice);

	pDevice->GetImmediateContext(&pd3DContext);
	return S_OK;
}

HRESULT MeshRenderer::ReloadShader(ID3D11Device* pDevice, ID3DX11Effect* m_pEffect)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", m_RenderET);

	SAFE_GET_PASS(m_RenderET, "P1_Mesh", m_Pass1_Mesh);

	SAFE_GET_RESOURCE(m_pEffect, "g_Diffuse", m_DiffuseEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Normal", m_NormalEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Specular", m_SpecularEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Glow", m_GlowEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_ShadowMap", m_ShadowMapEV);
	SAFE_GET_SCALAR(m_pEffect, "objectOfCamera", m_isCameraObjectEV);

	SAFE_GET_MATRIX(m_pEffect, "g_World", m_WorldEV);
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
	
}

void MeshRenderer::Deinit()
{
	for(auto it=g_Meshes.begin(); it!= g_Meshes.end(); it++)
		SAFE_DELETE(it->second);
}

