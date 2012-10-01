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
#include "FrustumCulling.h"
#include "RenderableTexture.h"

class MeshRenderer
{
public:
	static std::vector<GameObject*> g_MeshesToRender;
	MeshRenderer(FrustumCulling* f);
	~MeshRenderer(void);
	HRESULT ReloadShader(ID3D11Device* pDevice);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* pDevice);
	void ReleaseResources();
	void AddToRenderPass(GameObject* object);
	//void RenderMeshes(ID3D11Device* pDevice, std::vector<GameObject*>* object, ID3D11RenderTargetView* LightBW);
	//void RenderMeshes(ID3D11Device* pDevice, std::list<Enemy*>* object, ID3D11RenderTargetView* LightBW);
	//void ShadowMeshes(ID3D11Device* pDevice, std::vector<GameObject*>* object, ID3D11RenderTargetView* LightBW);
	//void ShadowMeshes(ID3D11Device* pDevice, std::list<Enemy*>* object, ID3D11RenderTargetView* LightBW);
	//void RenderMesh(ID3D11Device* pDevice, ObjectTransformation* object, ID3DX11EffectTechnique* technik);
	void RenderMesh(ID3D11Device* pDevice, GameObject* object, RenderableTexture* shadowMap = NULL, RenderableTexture* vlsMap = NULL, bool drawShadow = true);
	void RenderMeshes(ID3D11Device* pDevice, RenderableTexture* shadowMap = NULL, RenderableTexture* vlsMap = NULL, bool drawShadow = true);
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
	void ResetInstances();
private:
	inline void setEffectVariables(void);
	FrustumCulling* g_Frustum;
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_RenderET;
	ID3DX11EffectTechnique* m_ShadowET;

	ID3D11InputLayout* m_MeshInputLayout;

	ID3D11DeviceContext* pd3DContext;
	ID3D11Buffer* vbs[2];
	UINT offset[2];
	UINT stride[2];
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
	ID3DX11EffectMatrixVariable*			m_ViewProjEV;
	ID3DX11EffectMatrixVariable*			m_LightViewProjMatrixEV;
	ID3DX11EffectMatrixVariable*			m_ViewEV;
	ID3DX11EffectVectorVariable*			m_LightDirViewEV;
	ID3DX11EffectMatrixVariable*			m_NormalsEV;

};

