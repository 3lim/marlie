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
	indecies[0] =  0;
}

HRESULT renderSAT::CreateResources(ID3D11Device* pd3DDevice ,D3D11_TEXTURE2D_DESC inputDesc)
{
	HRESULT hr;
	d3dDevice = pd3DDevice;
	
	V(pd3DDevice->CreateTexture2D(&inputDesc, NULL, &satTexture));
	V(pd3DDevice->CreateTexture2D(&inputDesc, NULL, &satTextureV));

	D3D11_RENDER_TARGET_VIEW_DESC satTaret_Desc;
	satTaret_Desc.Format = inputDesc.Format;
	satTaret_Desc.Texture2D.MipSlice = 0;
	satTaret_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	V(pd3DDevice->CreateRenderTargetView(satTexture, &satTaret_Desc, &satTarget));
	V(pd3DDevice->CreateRenderTargetView(satTexture, &satTaret_Desc, &satTargetV));
	
	D3D11_SHADER_RESOURCE_VIEW_DESC satSRV_Desc;
	satSRV_Desc.Format =  inputDesc.Format;
	satSRV_Desc.Texture2D.MipLevels = 1;
	satSRV_Desc.Texture2D.MostDetailedMip = 0;
	satSRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	
	V(pd3DDevice->CreateShaderResourceView(satTexture, &satSRV_Desc, &satSRV));
	V(pd3DDevice->CreateShaderResourceView(satTexture, &satSRV_Desc, &satSRVV));

	satPasses = D3DXVECTOR2(log(inputDesc.Width)/log(satReso), log(inputDesc.Height)/log(satReso));
	imgWidth = inputDesc.Width;
	imgHeight = inputDesc.Height;
	return S_OK;
}
void renderSAT::ReleaseResources()
{
	SAFE_RELEASE(satTexture);
	SAFE_RELEASE(satSRV);
	SAFE_RELEASE(satTarget);
	SAFE_RELEASE(satTextureV);
	SAFE_RELEASE(satSRVV);
	SAFE_RELEASE(satTargetV);
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

void renderSAT::createSAT(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView** inputSRV, ID3D11DepthStencilView* inDepthView)
{
	shaderEffect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(*inputSRV);
	shaderEffect->GetVariableByName("imgSize")->AsVector()->SetFloatVector(D3DXVECTOR2(2048,2048));//only cubic images
	shaderEffect->GetVariableByName("range")->AsScalar()->SetFloat(satReso);
	struct Vertex
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR2 Texel;
	};
	ID3D11InputLayout* inLayout;
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "SV_POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3DX11_PASS_DESC pd;
	shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->GetDesc(&pd);
	d3dDevice->CreateInputLayout(layout, sizeof(layout)/sizeof(layout[0]), pd.pIAInputSignature,pd.IAInputSignatureSize, &inLayout);
	pd3dContext->OMSetRenderTargets(1, &satTarget, inDepthView);
	pd3dContext->IASetInputLayout(NULL);
	pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dContext);
	pd3dContext->Draw(1,0);
	//for(float i = 0; i < imgWidth; i++)
	//{
	//	float dx = 1.f/imgWidth;
	//	Vertex v[2] = 
	//		{{ D3DXVECTOR3(-1+2.f*dx*i,1,0.1), D3DXVECTOR2(dx*i, 0)},
	//		{ D3DXVECTOR3(-1+2.f*dx*i,-1,0.1), D3DXVECTOR2(dx*i, 1)},};
	//	ID3D11Buffer* vBuffer;
	//	D3D11_SUBRESOURCE_DATA id;
	//	id.pSysMem = &v[0];
	//	id.SysMemPitch = sizeof(Vertex);
	//	id.SysMemSlicePitch = sizeof(Vertex)*2;
	//	D3D11_BUFFER_DESC buffer_Desc;
	//	buffer_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//	buffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	//	buffer_Desc.ByteWidth = sizeof(Vertex);
	//	buffer_Desc.StructureByteStride = 0;
	////UINT indexBuffer[1] = { 0 };
	////pd3dContext->IASetIndexBuffer(dummyIB, DXGI_FORMAT_R32_UINT, 0);

	SAFE_RELEASE(inLayout);
	//shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dContext);
	//pd3dContext->Draw(2,0);
	//}
	//Swap input and result
	pd3dContext->ClearRenderTargetView(satTargetV, D3DXCOLOR(1,1,1,0));
	shaderEffect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(satSRV);
	pd3dContext->OMSetRenderTargets(1, &satTargetV, inDepthView);
	pd3dContext->IASetInputLayout(NULL);
	pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	shaderEffect->GetTechniqueByIndex(0)->GetPassByIndex(1)->Apply(0, pd3dContext);
	pd3dContext->Draw(1,0);
	
}

renderSAT::~renderSAT(void)
{
	ReleaseResources();
}
