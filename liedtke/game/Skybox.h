#pragma once

#include <DXUT.h>
#include <DXUTcamera.h>
#include "Effects11/Inc/d3dx11effect.h"
#include <string>

#include "SpriteRenderer.h"

class Skybox
{
public:
	Skybox(std::string texturePath, float sunDistance);
	~Skybox(void);

	HRESULT ReloadShader(ID3D11Device*);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device*);
	void ReleaseResources();
	HRESULT RenderSkybox(ID3D11Device*, const CFirstPersonCamera& camera);
	void OnMove(double fTime, float fElapsedTime);
	static D3DXCOLOR g_LightColor;
	static D3DXVECTOR4 g_LightDir;
	SpriteVertex* getSun() { return &m_Sun; }
	void setSunDistance(float p) { m_SunDistance = p; }
	void HorizontColor(float r, float g, float b) { horizontColor += D3DXCOLOR(r,g,b,0);}
	void ApexColor(float r, float g, float b) { apexColor += D3DXCOLOR(r,g,b,0);}
private:
	std::string m_SkyboxPath;
	SpriteVertex m_Sun;
	float m_SunSpeed;
	float m_SunDistance;

	ID3D11Texture2D* m_SkyboxTex;
	ID3D11ShaderResourceView* m_SkyboxSRV;

	ID3D11DeviceContext*			m_Context;
	ID3DX11Effect*					m_pEffect;
	ID3DX11EffectPass*				m_SkyPass;
	ID3DX11EffectTechnique*			m_SkyboxTechnique;
	ID3DX11EffectVectorVariable*	m_TopLeft;
	ID3DX11EffectVectorVariable*	m_Down;
	ID3DX11EffectVectorVariable*	m_Right;
	ID3DX11EffectVectorVariable*	m_Eye;

	ID3DX11EffectVectorVariable*	m_horizontColorEV;
	ID3DX11EffectVectorVariable*	m_apexColorEV;
	D3DXCOLOR						horizontColor;
	D3DXCOLOR						apexColor;
};

