#pragma once
#include <DXUT.h>
#include "RenderableTexture.h"


#include "Effects11/Inc/d3dx11effect.h"
//#include "debug.h"
class renderSAT
{
public:
	renderSAT(UINT RenderPasses);
	HRESULT ReloadShader(ID3D11Device* pDevice);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* device, unsigned int width, unsigned int height, DXGI_FORMAT format);
	void ReleaseResources();
	RenderableTexture* createSAT(ID3D11DeviceContext* contex, RenderableTexture* Input);

	RenderableTexture* src() { return m_src; }
	RenderableTexture* dest() {return m_dest; }
	RenderableTexture* original() { return m_original; }
	~renderSAT(void);

private:
	UINT imgWidth;
	UINT imgHeight;
	D3D11_VIEWPORT m_Viewport;
	const UINT SATRenderSamples;
	unsigned int			satReso;
	D3DXVECTOR2				satPasses;
	ID3DX11EffectShaderResourceVariable* m_sourceEV;
	ID3D11Device*			m_device;
	RenderableTexture*		m_src;
	RenderableTexture*		m_dest;
	RenderableTexture*		m_original;
	
	UINT					indecies[1];
	ID3D11Buffer*			dummyIB;
	ID3DX11Effect*			shaderEffect;
	ID3DX11EffectTechnique*	SATRenderTechnique;


};

