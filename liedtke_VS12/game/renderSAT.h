#pragma once
#include <DXUT.h>


#include "Effects11/Inc/d3dx11effect.h"
//#include "debug.h"
class renderSAT
{
public:
	renderSAT(void);
	HRESULT ReloadShader(ID3D11Device* pDevice);
	void ReleaseShader();
	HRESULT CreateResources(ID3D11Device* device, D3D11_TEXTURE2D_DESC input);
	void ReleaseResources();
	ID3D11ShaderResourceView* createSAT(ID3D11DeviceContext* contex, ID3D11ShaderResourceView* input);
	~renderSAT(void);

private:
	unsigned int			satReso;
	D3DXVECTOR2				satPasses;
	ID3D11RenderTargetView* satTarget;
	ID3D11ShaderResourceView* satSRV;
	ID3D11Texture2D*		satTexture;
	ID3D11Device*			d3dDevice;
	
	ID3DX11Effect*			shaderEffect;


};

