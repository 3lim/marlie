#pragma once

#include <DXUT.h>
#include <DXUTcamera.h>
#include "Effects11/Inc/d3dx11effect.h"
#include <string>

class Skybox
{
public:
	Skybox(std::string texturePath);
	~Skybox(void);

	HRESULT ReloadShader(ID3D11Device*);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device*);
	void ReleaseResources();
	HRESULT RenderSkybox(ID3D11Device*, const CFirstPersonCamera& camera);
private:
	std::string m_SkyboxPath;
	ID3D11Texture2D* m_SkyboxTex;
	ID3D11ShaderResourceView* m_SkyboxSRV;

	ID3D11DeviceContext* m_Context;
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectPass* m_SkyPass;
	ID3DX11EffectTechnique* m_SkyboxTechnique;
	ID3DX11EffectVectorVariable* m_TopLeft;
	ID3DX11EffectVectorVariable* m_Down;
	ID3DX11EffectVectorVariable* m_Right;
	ID3DX11EffectVectorVariable* m_Eye;
};

