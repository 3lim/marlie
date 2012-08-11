#include "SpriteRenderer.h"
#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "DXUT\Core\DXUT.h"
#include "DXUTmisc.h"
#include "SDKmisc.h"
#include "debug.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <vector>
#include <cstdint>
#include "ntx\NTX.h"

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}
#define null NULL

ID3DX11EffectMatrixVariable* g_ViewProjectionEV(NULL);
ID3DX11EffectVectorVariable* g_CameraRightEV(NULL);
ID3DX11EffectVectorVariable* g_CameraUpEV(NULL);
std::vector<ID3D11ShaderResourceView*> g_TextureSRVA;
std::vector<ID3D11Texture2D*> g_DiffuseTexA;
ID3DX11EffectShaderResourceVariable* g_TextureEVA(NULL);


using namespace std;


SpriteRenderer::SpriteRenderer(const std::vector<SpriteFile>& textureFilenames) :
m_textureFilenames(textureFilenames),
	m_pEffect(NULL),
	m_spriteSRV(NULL),
	m_spriteTex(NULL),
	m_spriteCountMax(NULL),
	m_pVertexBuffer(NULL),
	m_pInputLayout(NULL)
{
}


SpriteRenderer::~SpriteRenderer(void)
{
}

HRESULT SpriteRenderer::ReloadShader(ID3D11Device* pDevice)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"SpriteRender.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), D3D11_CREATE_DEVICE_DEBUG, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", g_ViewProjectionEV);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraUp", g_CameraUpEV);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraRight", g_CameraRightEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_SpriteDiffuseArray", g_TextureEVA);
	return S_OK;
}

void SpriteRenderer::ReleaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::CreateResources(ID3D11Device* pDevice){
	m_spriteCountMax  = 2048;
	//7.3.3
	HRESULT hr;

	D3D11_BUFFER_DESC bdV;
	D3D11_SUBRESOURCE_DATA idV;

	idV.pSysMem = NULL;
	idV.SysMemPitch = sizeof(SpriteVertex);
	idV.SysMemSlicePitch = 0;

	bdV.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdV.ByteWidth = m_spriteCountMax*sizeof(SpriteVertex);
	bdV.CPUAccessFlags = 0;
	bdV.MiscFlags = 0;
	bdV.Usage = D3D11_USAGE_DEFAULT;

	V_RETURN(pDevice->CreateBuffer(&bdV, NULL, &m_pVertexBuffer)); 

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXTUREINDEX", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "ANIMATIONTIME", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "ALPHABLENDING", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "BLENDINGCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	D3DX11_PASS_DESC pd;
	V_RETURN(m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0_Sprite")->GetDesc(&pd));
	V_RETURN(pDevice->CreateInputLayout(layout, sizeof(layout)/sizeof(layout[0]), pd.pIAInputSignature, pd.IAInputSignatureSize, &m_pInputLayout ));

	g_DiffuseTexA.resize(m_textureFilenames.size());
	g_TextureSRVA.resize(m_textureFilenames.size());
	for(size_t i = 0; i < m_textureFilenames.size(); i++)
	{
		D3D11_TEXTURE2D_DESC tex2DDesc;
		bool sRgb;
		vector<D3D11_SUBRESOURCE_DATA> subresourcesData;
		vector<vector<unsigned char>> data;
		subresourcesData.resize(m_textureFilenames.size());
		LoadNtxArrayFromFiles(m_textureFilenames[i].file, m_textureFilenames[i].begin, m_textureFilenames[i].end, &tex2DDesc, data, subresourcesData, sRgb);
		V_RETURN(pDevice->CreateTexture2D(&tex2DDesc, &subresourcesData[0], &g_DiffuseTexA[i]));
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = tex2DDesc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		shaderDesc.Texture2DArray.ArraySize = tex2DDesc.ArraySize;
		shaderDesc.Texture2DArray.FirstArraySlice = 0;
		shaderDesc.Texture2DArray.MipLevels = tex2DDesc.MipLevels;
		shaderDesc.Texture2DArray.MostDetailedMip = 0;
		V_RETURN(pDevice->CreateShaderResourceView(g_DiffuseTexA[i], &shaderDesc, &g_TextureSRVA[i]));
	}

	return S_OK;
}

void SpriteRenderer::ReleaseResources(){
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexBuffer);
	for(size_t i = 0; i < m_textureFilenames.size(); i++){
		SAFE_RELEASE(g_TextureSRVA[i]);
	SAFE_RELEASE(g_DiffuseTexA[i]);
	}
}

void SpriteRenderer::RenderSprites(ID3D11Device* pDevice, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera){
	HRESULT hr;
	ID3D11DeviceContext* pd3DContext;
	ID3DX11EffectPass* myPass = m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0_Sprite");
	pDevice->GetImmediateContext(&pd3DContext);
	D3D11_BOX box; 
	box.left  = 0;
	box.right = sprites.size() * sizeof(SpriteVertex); 
	box.top   = 0;
	box.bottom = 1; 
	box.front = 0;
	box.back   = 1; 
	pd3DContext->UpdateSubresource(m_pVertexBuffer, 0, &box, &sprites[0], 0,0);
	UINT stride[1] = {sizeof(SpriteVertex)};
	UINT offset[1] = {0};
	ID3D11Buffer* vertexbuffer[] = { m_pVertexBuffer};
	pd3DContext->IASetVertexBuffers(0, 1, vertexbuffer, stride, offset);
	pd3DContext->IASetInputLayout(m_pInputLayout);
	pd3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3DXMATRIX viewProjection = (*camera.GetViewMatrix())*(*camera.GetProjMatrix());
	g_ViewProjectionEV->SetMatrix((float*)viewProjection);
	g_CameraRightEV->SetFloatVector(*camera.GetWorldRight());
	g_CameraUpEV->SetFloatVector(*camera.GetWorldUp());
	for(size_t i = 0; i < m_textureFilenames.size(); i++)
		g_TextureEVA->GetElement(i)->AsShaderResource()->SetResource(g_TextureSRVA[i]);

	myPass->Apply(0,pd3DContext);
	pd3DContext->Draw(sprites.size(),0);
	SAFE_RELEASE(pd3DContext);


}
HRESULT SpriteRenderer::LoadFile(const char * filename, std::vector<uint8_t>& data)
{
	FILE * filePointer = NULL;
	errno_t error = fopen_s(&filePointer, filename, "rb");
	if (error) 	{ return E_INVALIDARG; }
	fseek(filePointer, 0, SEEK_END);
	long bytesize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	data.resize(bytesize);
	fread(&data[0], 1, bytesize, filePointer);
	fclose(filePointer);
	return S_OK;
}
