#include "renderSAT.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SDKmisc.h"
#include "Macros.h"
#include <vector>

using namespace std;

renderSAT::renderSAT(void) : satPasses(0,0),
	satReso(2),
	satSRV(NULL),
	satTarget(NULL),
	satTexture(NULL),
	shaderEffect(NULL)
{
}

HRESULT renderSAT::CreateResources(ID3D11Device* pd3DDevice ,D3D11_TEXTURE2D_DESC inputDesc)
{
	HRESULT hr;
	d3dDevice = pd3DDevice;
	
	V(pd3DDevice->CreateTexture2D(&inputDesc, NULL, &satTexture));

	D3D11_RENDER_TARGET_VIEW_DESC satTaret_Desc;
	satTaret_Desc.Format = inputDesc.Format;
	satTaret_Desc.Texture2D.MipSlice = 0;
	satTaret_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	V(pd3DDevice->CreateRenderTargetView(satTexture, &satTaret_Desc, &satTarget));
	
	D3D11_SHADER_RESOURCE_VIEW_DESC satSRV_Desc;
	satSRV_Desc.Format =  inputDesc.Format;
	satSRV_Desc.Texture2D.MipLevels = 1;
	satSRV_Desc.Texture2D.MostDetailedMip = 0;
	satSRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	
	V(pd3DDevice->CreateShaderResourceView(satTexture, &satSRV_Desc, &satSRV));

	satPasses = D3DXVECTOR2(log(inputDesc.Width)/log(satReso), log(inputDesc.Height)/log(satReso));

	return S_OK;
}
void renderSAT::ReleaseResources()
{
	SAFE_RELEASE(satTexture);
	SAFE_RELEASE(satSRV);
	SAFE_RELEASE(satTarget);
	//SAFE_DELETE(d3dDevice); //maybe unsafe
}

HRESULT renderSAT::ReloadShader(ID3D11Device* pDevice)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"SAT.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &shaderEffect));    
	assert(shaderEffect->IsValid());


	return S_OK;
}

void renderSAT::ReleaseShader()
{
	SAFE_RELEASE(shaderEffect);
}

ID3D11ShaderResourceView* renderSAT::createSAT(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* inputSRV)
{
	shaderEffect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(inputSRV);
	shaderEffect->GetVariableByName("imgPasses")->AsScalar()->SetFloat(satPasses.x);//only cubic images
	shaderEffect->GetVariableByName("range")->AsScalar()->SetFloat(satReso);

	shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dContext);
	
	pd3dContext->ClearRenderTargetView(satTarget, D3DXVECTOR4(1,1,1,1));
	pd3dContext->OMSetRenderTargets(1, &satTarget, NULL);
	pd3dContext->DrawAuto();
	//Swap input and result

	//shaderEffect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(satSRV);
	//shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(1)->Apply(0, pd3dContext);
	//pd3dContext->DrawAuto();
	
	return satSRV;
}

renderSAT::~renderSAT(void)
{
	ReleaseResources();
}
