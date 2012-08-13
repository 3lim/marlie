#pragma once

#include <string>
#include <vector>
#include <list>
#include <cstdint>

#include <map>


#include "ObjectTransformation.h"
#include "EnemyInstance.h"
#include "Mesh.h"
#include "T3d.h"

class MeshRenderer
{
public:
	MeshRenderer(void);
	~MeshRenderer(void);
	HRESULT ReloadShader(ID3D11Device* pDevice, ID3DX11Effect* e);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* pDevice);
	void ReleaseResources();
	void RenderMeshes(ID3D11Device* pDevice, std::vector<ObjectTransformation>* object);
	void RenderMeshes(ID3D11Device* pDevice, std::list<EnemyInstance>* object);
	void ShadowMeshes(ID3D11Device* pDevice, std::vector<ObjectTransformation>* object);
	void ShadowMeshes(ID3D11Device* pDevice, std::list<EnemyInstance>* object);
	void RenderMesh(ID3D11Device* pDevice, ObjectTransformation* object);
	void ShadowMesh(ID3D11Device* pDevice, ObjectTransformation* object);
	void Deinit();
	static std::map<std::string, Mesh*> g_Meshes;
	//HRESULT LoadFile(const char* filename, std::vector<uint8_t>& data);

private:
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

		//Effectvariablen
	ID3DX11EffectPass*						m_Pass1_Mesh;
	ID3DX11EffectPass*						m_ShadowMesh;
	ID3DX11EffectShaderResourceVariable*	m_DiffuseEV;
	ID3DX11EffectShaderResourceVariable*	m_ShadowMapEV;
	ID3DX11EffectShaderResourceVariable*	m_GlowEV;
	ID3DX11EffectShaderResourceVariable*	m_SpecularEV;
	ID3DX11EffectShaderResourceVariable*	m_NormalEV;
	ID3DX11EffectMatrixVariable*			m_WorldEV;
	ID3DX11EffectScalarVariable*			m_isCameraObjectEV;

};

