#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include <DXUT.h>

#include "Effects11/Inc/d3dx11effect.h"
#include "ntx\NTX.h"
#include "T3d.h"
#include "GameObject.h"
#include "Enemy.h"

extern ID3D11ShaderResourceView*		g_ShadowMapSRV;


class MeshRenderer
{
public:
	MeshRenderer(void);
	~MeshRenderer(void);
	HRESULT ReloadShader(ID3D11Device* pDevice);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* pDevice);
	void ReleaseResources();
	void RenderMeshes(ID3D11Device* pDevice, std::vector<GameObject*>* object);
	void RenderMeshes(ID3D11Device* pDevice, std::list<Enemy*>* object);
	void ShadowMeshes(ID3D11Device* pDevice, std::vector<GameObject*>* object);
	void ShadowMeshes(ID3D11Device* pDevice, std::list<Enemy*>* object);
	//void RenderMesh(ID3D11Device* pDevice, ObjectTransformation* object, ID3DX11EffectTechnique* technik);
	void RenderMesh(ID3D11Device* pDevice, GameObject* object, ID3DX11EffectTechnique* technik);
	void Deinit();
	static std::map<std::string, Mesh*> g_Meshes;
	//HRESULT LoadFile(const char* filename, std::vector<uint8_t>& data);
	//EffectVariabeln welche von auﬂen gesetzt werden
	D3DXCOLOR* g_LightColor;
	D3DXVECTOR4* g_LightDirView;
	D3DXMATRIX* g_LightView;
	D3DXMATRIX* g_LightProj;
	D3DXMATRIX* g_LightViewProjMatrix;
	D3DXMATRIX* g_Proj;
	D3DXMATRIX* g_View;
	D3DXMATRIX* g_ViewProj;
	D3DXMATRIX* g_invView;
private:
	inline void setEffectVariables(void);
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_RenderET;
	ID3DX11EffectTechnique* m_ShadowET;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_MeshInputLayout;

	ID3D11DeviceContext* pd3DContext;
	ID3D11Buffer* vbs[1];
	UINT offset;
	UINT stride;
	Mesh* mesh;
	D3DXMATRIX WorldViewProjektion;
	D3DXMATRIX WorldViewNormals;
	D3DXMATRIX WorldLightViewProjMatrix;
	D3DXMATRIX WorldView;

		//Effectvariablen
	ID3DX11EffectPass*						m_Pass1_Mesh;
	ID3DX11EffectPass*						m_ShadowMesh;
	ID3DX11EffectShaderResourceVariable*	m_DiffuseEV;
	ID3DX11EffectShaderResourceVariable*	m_ShadowMapEV;
	ID3DX11EffectShaderResourceVariable*	m_GlowEV;
	ID3DX11EffectShaderResourceVariable*	m_SpecularEV;
	ID3DX11EffectShaderResourceVariable*	m_NormalEV;

	ID3DX11EffectVectorVariable*			m_LightColorEV;
	ID3DX11EffectMatrixVariable*			m_WorldViewProjektionEV;
	ID3DX11EffectMatrixVariable*			m_LightViewProjMatrixEV;
	ID3DX11EffectMatrixVariable*			m_WorldLightViewProjMatrixEV;
	ID3DX11EffectMatrixVariable*			m_WorldViewEV;
	ID3DX11EffectVectorVariable*			m_LightDirViewEV;
	ID3DX11EffectMatrixVariable*			m_WorldViewNormalsEV;

};

