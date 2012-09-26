#include "renderSAT.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SDKmisc.h"
#include "Macros.h"
#include <vector>

using namespace std;

renderSAT::renderSAT(UINT passes) : satPasses(0,0),
	satReso(8),
	m_src(NULL),
	m_original(NULL),
	m_dest(NULL),
	shaderEffect(NULL),
	SATRenderSamples(passes)
{
	indecies[0] =  0;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Height = 0;
	m_Viewport.Width = 0;
}

HRESULT renderSAT::CreateResources(ID3D11Device* device, unsigned int width, unsigned int height, DXGI_FORMAT format) 
{
	HRESULT hr;
	m_device = device;
	imgHeight = m_Viewport.Height = height;
	imgWidth = m_Viewport.Width = width;
	
	m_dest = new RenderableTexture(device, width, height, 1, format);
	m_src = new RenderableTexture(device, width, height, 1, format);

	return S_OK;
}
void renderSAT::ReleaseResources()
{
	SAFE_DELETE(m_dest);
	SAFE_DELETE(m_src);
	m_original = 0;
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

	SAFE_GET_TECHNIQUE(shaderEffect, "SAT", SATRenderTechnique);
	SAFE_GET_RESOURCE(shaderEffect, "Source", m_sourceEV);

	return S_OK;
}

void renderSAT::ReleaseShader()
{
	SAFE_RELEASE(shaderEffect);
}

//RecursiveDouble
RenderableTexture* renderSAT::createSAT(ID3D11DeviceContext* pd3dContext, RenderableTexture* Input)
{

	shaderEffect->GetVariableByName("Samples")->AsScalar()->SetInt(SATRenderSamples);
	pd3dContext->RSSetViewports(1, &m_Viewport);
	m_original = Input;
	//HorizontalPass
	for(int i = 1; i < imgWidth; i *= SATRenderSamples)
	{
		int PassOffset[2] = { i, 0};

		int done = i/SATRenderSamples;
		D3D11_RECT region = { done, 0, imgWidth, imgHeight};
		//nur beim ersten pass muss das original bild genommen werden
		m_sourceEV->SetResource(i == 1 ? m_original->GetShaderResource() : m_src->GetShaderResource());
		shaderEffect->GetVariableByName("PassOffset")->AsVector()->SetIntVector(PassOffset);
		//avoids warning
		pd3dContext->OMSetRenderTargets(0, NULL, NULL);
		SATRenderTechnique->GetPassByIndex(0)->Apply(0, pd3dContext);

		ID3D11RenderTargetView* pRTV = m_dest->GetRenderTarget();
		pd3dContext->OMSetRenderTargets(1, &pRTV, NULL);
		pd3dContext->RSSetScissorRects(1, &region);
		pd3dContext->IASetInputLayout(NULL);
		pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		SATRenderTechnique->GetPassByIndex(0)->Apply(0, pd3dContext);
		pd3dContext->Draw(1,0);
		// Swap pointers (ping pong)
		std::swap(m_dest, m_src);
	}
	//vertical Pass
	for(int i = 1; i < imgHeight; i *= SATRenderSamples)
	{
		int PassOffset[2] = { 0,i};

		int done = i/SATRenderSamples;
		D3D11_RECT region = { 0, done, imgWidth, imgHeight};
		ID3D11ShaderResourceView* source = m_src->GetShaderResource();
		m_sourceEV->SetResource(source);
		shaderEffect->GetVariableByName("PassOffset")->AsVector()->SetIntVector(PassOffset);
		//avoids warning
		pd3dContext->OMSetRenderTargets(0, NULL, NULL);
		SATRenderTechnique->GetPassByIndex(0)->Apply(0, pd3dContext);

		ID3D11RenderTargetView* pRTV = m_dest->GetRenderTarget();
		pd3dContext->OMSetRenderTargets(1, &pRTV, NULL);
		pd3dContext->RSSetScissorRects(1, &region);
		pd3dContext->IASetInputLayout(NULL);
		pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		SATRenderTechnique->GetPassByIndex(0)->Apply(0, pd3dContext);
		pd3dContext->Draw(1,0);
		// Swap pointers (ping pong)
		std::swap(m_dest, m_src);
	}
	return m_dest;
}

renderSAT::~renderSAT(void)
{
	ReleaseResources();
}
