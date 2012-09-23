#pragma once

#include <cstdint>
#include <DXUT.h>
#include "Effects11\Inc\d3dx11effect.h"

#include <vector>
#include <string>
#include "RenderableTexture.h"


struct PtfHeader{
	int16_t magicNumber; 	// Must be 0x00DA
	int16_t version;		// Must be 1
	int32_t heightSize;	    // Height data size
	int32_t colorSize;	    // Color data size
	int32_t normalSize;	    // Normal data size
};

class TerrainRenderer
{
public:
	TerrainRenderer(CHAR path[], float width, float depth, float height, bool spinning = false, float spinningSpeed = 0.f);
	~TerrainRenderer(void);
	HRESULT ReloadShader(ID3D11Device* pDevice);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* pDevice);
	void ReleaseResources();
	void OnMove( double fTime, float fElapsedTime);
	void RenderTerrain(ID3D11Device* pDevice, RenderableTexture* shadowMap, ID3D11RenderTargetView* VolumetricLightScattering);
	void ShadowTerrain(ID3D11Device* pDevice);
	void Deinit();
	static std::vector<unsigned short> g_TerrainHeights;
	float* getDepth() { return &m_TerrainDepth; }
	float* getWidth() { return &m_TerrainWidth; }
	float* getHeight() { return &m_TerrainHeight; }

	//liest Höhe des Terrains in bezug auf die länge des Terrains im Spiel
	float getHeightAtPoint(float x, float y);
	//liest Hohe des Terrains direkt aus dem HeightField
	float getHeightAtPoint(int x, int y);
	
	D3DXVECTOR3 getNormalAtPoint(float x, float y, size_t size = 1);

	D3DXMATRIX* g_ViewProj;
	D3DXVECTOR4* g_LightDir;
	D3DXMATRIX* g_LightViewProjMatrix;
private:
	inline void setEffectVariables(void);
	// Terrain meta information
	PtfHeader		m_TerrainHeader; 
	int				m_TerrainResolution;
	int				m_TerrainNumVertices;
	int				m_TerrainNumTriangles;
	float			m_TerrainWidth;
	float			m_TerrainDepth;
	float			m_TerrainHeight;
	bool			m_TerrainSpinning;
	float			m_TerrainSpinSpeed;
	int				m_TerrainVertexCount;
	std::string			m_TerrainPath;

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_RenderET;
	ID3DX11EffectTechnique* m_ShadowET;

	ID3D11DeviceContext* pd3DContext;
	ID3D11Buffer* vbs[1];
	UINT offset;
	UINT stride;

		//Effectvariablen
	ID3DX11EffectPass*						m_RenderTerrain;
	ID3DX11EffectPass*						m_ShadowTerrain;
	ID3DX11EffectShaderResourceVariable*	m_ShadowMapEV;
	// Terrain rendering resources
	ID3D11Buffer*							m_TerrainHeightBuf; 
	ID3D11ShaderResourceView*				m_TerrainHeightSRV; 
	ID3DX11EffectShaderResourceVariable*	m_TerrainHeightEV;
	ID3D11Texture2D*						m_TerrainNormalTex; 
	ID3D11ShaderResourceView*				m_TerrainNormalSRV; 
	ID3DX11EffectShaderResourceVariable*	m_TerrainNormalEV;
	ID3D11Texture2D*                        m_TerrainDiffuseTex; // The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               m_TerrainDiffuseSRV; // Describes the structure of the diffuse texture to the shader stages
	ID3DX11EffectShaderResourceVariable*	m_TerrainDiffuseEV;

	ID3DX11EffectScalarVariable*			m_TerrainResEV;
	ID3DX11EffectScalarVariable*			m_TerrainQuadsEV;

	D3DXMATRIX								m_World;
	ID3DX11EffectMatrixVariable*			m_WorldEV;
	ID3DX11EffectMatrixVariable*			m_ViewProjectionEV;
	ID3DX11EffectMatrixVariable*			m_LightViewProjMatrixEV;
	ID3DX11EffectVectorVariable*			m_LightDirEV;
};

