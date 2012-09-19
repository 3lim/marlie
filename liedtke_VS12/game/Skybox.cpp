#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "DXUT\Core\DXUT.h"
#include "DXUTmisc.h"
#include "SDKmisc.h"
#include "debug.h"
#include "Skybox.h"
#include "ntx\NTX.h"

#include "Macros.h"

using namespace std;


D3DXCOLOR Skybox::g_LightColor(1.0f, 1.0f, 1.0f,1.0f);
D3DXVECTOR4 Skybox::g_LightDir(1.f,1.f,-1.f,0.f);

bool Skybox::InitializeSkyPlane(int skyPlaneResolution, float skyPlaneWidth, float skyPlaneTop, float skyPlaneBottom, int textureRepeat)
{
	float quadSize, radius, constant, textureDelta;
	int i, j, index;
	float positionX, positionY, positionZ, tu, tv;


	m_VertexCount = ((skyPlaneResolution + 1) * (skyPlaneResolution + 1));
	m_skyPlane.resize(m_VertexCount);
	m_IndexCount = m_VertexCount*6;
	vector<uvVertex> planeCords;
	planeCords.resize(m_VertexCount);

	quadSize = skyPlaneWidth / (float)skyPlaneResolution;
	radius = skyPlaneWidth / 2.0f;
	constant = (skyPlaneTop - skyPlaneBottom) / (radius * radius);
	textureDelta = (float)textureRepeat / (float)skyPlaneResolution;

	for(j=0; j<=skyPlaneResolution; j++)
	{
		for(i=0; i<=skyPlaneResolution; i++)
		{
			positionX = (-0.5f * skyPlaneWidth) + ((float)i * quadSize);
			positionZ = (-0.5f * skyPlaneWidth) + ((float)j * quadSize);
			positionY = skyPlaneTop - (constant * ((positionX * positionX) + (positionZ * positionZ)));

			tu = (float)i * textureDelta;
			tv = (float)j * textureDelta;

			index = j * (skyPlaneResolution + 1) + i;

			m_skyPlane[index].Pos = D3DXVECTOR3(positionX, positionY, positionZ);
			m_skyPlane[index].Tex = D3DXVECTOR2(tu, tv);
			m_skyPlane[index].Plane = D3DXVECTOR2(i/(float)skyPlaneResolution,j/(float)skyPlaneResolution);
		}
	}

	indices.resize(m_IndexCount);
	index = 0;
	int index1, index2, index3, index4;
	for(j=0; j<skyPlaneResolution; j++)
	{
		for(i=0; i<skyPlaneResolution; i++)
		{
			index1 = j * (skyPlaneResolution + 1) + i;
			index2 = j * (skyPlaneResolution + 1) + (i+1);
			index3 = (j+1) * (skyPlaneResolution + 1) + i;
			index4 = (j+1) * (skyPlaneResolution + 1) + (i+1);
			// Triangle 1 - Upper Left
			indices[index] = index1;
			index++;
			// Triangle 1 - Upper Right
			indices[index] = index2;
			index++;
			// Triangle 1 - Bottom Left
			indices[index] = index3;
			index++;
			// Triangle 2 - Bottom Left
			indices[index] = index3;
			index++;
			// Triangle 2 - Upper Right
			indices[index] = index2;
			index++;
			// Triangle 2 - Bottom Right
			indices[index] = index4;
			index++;
		}
	}

	return true;
}

Skybox::Skybox(string path, float sunDistance) :
m_SkyboxPath(path),
	m_Context(NULL),
	m_pEffect(NULL),
	m_Down(NULL),
	m_Right(NULL),
	m_TopLeft(NULL),
	m_Eye(NULL),
	m_SkyboxTechnique(NULL),
	m_SunDistance(sunDistance),
	m_SunSpeed(0.4f),
	m_horizontColorEV(NULL),
	m_apexColorEV(NULL),
	horizontColor(D3DXCOLOR(0.8,0.4,0.6 ,1)),
	apexColor(D3DXCOLOR(0.0,0.12,0.8,1)),
	m_DeltaSun(0),
	cloudTex1(NULL),
	cloudTex2(NULL),
	cloud1SRV(NULL),
	cloud2SRV(NULL),
	m_CloudVB(NULL)
{
	//pair<Horizont, Apex>
	dayColor[0] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.76, 0.4, 0.18,1), D3DXCOLOR(0.1, 0.16, 0.58 ,1));//Sonnenaufgang
	dayColor[1] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.34, 0.52, 1,1), D3DXCOLOR(0.08, 0.02, 0.8 ,1));//Tag
	dayColor[2] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.9, 0.18, 0.12,1), D3DXCOLOR(0.08, 0.1, 0.1 ,1));//Dämmerung
	dayColor[3] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.02, 0.02, 0.04,1), D3DXCOLOR(0.01, 0.01, 0.025 ,1));//Nacht

	// Setup the cloud translation speed increments.
	translationSpeed[0] = D3DXVECTOR2(0.0001f, 0.0f);   // First texture
	translationSpeed[1] = D3DXVECTOR2(0.000035f,0.0f);  // Second texture
	textureTranslation[0] = D3DXVECTOR2(0,0);
	textureTranslation[1] = D3DXVECTOR2(0,0);
}


Skybox::~Skybox(void)
{
}

wstring Skybox::AktColor()
{ 
	wstringstream result; result << "H: " << horizontColor.r << " " << horizontColor.g << " "<<horizontColor.b << "A: "<< apexColor.r << " "<<apexColor.g <<" " << apexColor.b; 
	return result.str();
}


HRESULT Skybox::ReloadShader(ID3D11Device* pDevice)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"Skybox.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V(D3DX11CreateEffectFromMemory(&effectBuffer[0], effectBuffer.size(), D3D11_CREATE_DEVICE_DEBUG, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());
	SAFE_GET_TECHNIQUE(m_pEffect, "tSkybox", m_SkyboxTechnique);

	SAFE_GET_PASS(m_SkyboxTechnique, "drawSkyCube", m_SkyPass);

	SAFE_GET_VECTOR(m_pEffect, "g_TopLeft", m_TopLeft);
	SAFE_GET_VECTOR(m_pEffect, "g_Down", m_Down);
	SAFE_GET_VECTOR(m_pEffect, "g_Right", m_Right);
	SAFE_GET_VECTOR(m_pEffect, "g_Eye", m_Eye);
	SAFE_GET_VECTOR(m_pEffect, "horizontColor", m_horizontColorEV);
	SAFE_GET_VECTOR(m_pEffect, "apexColor", m_apexColorEV);
	return S_OK;
}

void Skybox::ReleaseShader()
{
	SAFE_RELEASE(m_pEffect);
}


HRESULT Skybox::CreateResources(ID3D11Device* pDevice, float skyPlaneWidth, float skyPlaneHeight)
{
	HRESULT hr;
	m_Context = DXUTGetD3D11DeviceContext();

	bool rgb;
	vector<vector<unsigned char>> data;
	vector<D3D11_SUBRESOURCE_DATA> textureData;
	D3D11_TEXTURE2D_DESC tex2DDesc;
	V_RETURN(LoadNtxFromFile(m_SkyboxPath, &tex2DDesc, data, textureData, rgb));
	pDevice->CreateTexture2D(&tex2DDesc,&textureData[0],&m_SkyboxTex);
	pDevice->CreateShaderResourceView(m_SkyboxTex,NULL,&m_SkyboxSRV);

	V_RETURN(LoadNtxFromFile("resources/clouds1.ntx", &tex2DDesc, data, textureData, rgb));
	pDevice->CreateTexture2D(&tex2DDesc,&textureData[0],&cloudTex1);
	pDevice->CreateShaderResourceView(cloudTex1,NULL,&cloud1SRV);

	V_RETURN(LoadNtxFromFile("resources/clouds_2.ntx", &tex2DDesc, data, textureData, rgb));
	pDevice->CreateTexture2D(&tex2DDesc,&textureData[0],&cloudTex2);
	pDevice->CreateShaderResourceView(cloudTex2,NULL,&cloud2SRV);
	//Sonne
	m_Sun.Opacity = 1;
	//nun im onMove
	m_Sun.Position = (D3DXVECTOR3)(g_LightDir)*m_SunDistance;
	m_Sun.Radius = 100.f;
	m_Sun.TextureIndex = 0;
	m_Sun.AnimationProgress = 0;
	m_Sun.Color = g_LightColor;

	//SkyPlane

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PLANECOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3DX11_PASS_DESC pd;
	m_pEffect->GetTechniqueByName("tSkybox")->GetPassByName("Clouds")->GetDesc(&pd);
	V(pDevice->CreateInputLayout(layout, sizeof(layout)/sizeof(layout[0]), pd.pIAInputSignature,pd.IAInputSignatureSize, &m_SkydomeLayout));

	// Set the sky plane parameters.
	size_t skyPlaneResolution = 5;
	float skyPlaneTop = 0.0f;
	float skyPlaneBottom = skyPlaneHeight*0.4;
	size_t textureRepeat = 2;

	InitializeSkyPlane(skyPlaneResolution, skyPlaneWidth*2, skyPlaneHeight+skyPlaneTop, skyPlaneBottom, textureRepeat);

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;


	m_IndexCount =  m_VertexCount*6;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(uvVertex) * m_VertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = &m_skyPlane[0];
	vertexData.SysMemPitch = sizeof(uvVertex);
	vertexData.SysMemSlicePitch = 0;

	V(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_CloudVB));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_IndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	V(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_CloudIB));

	return S_OK;
}

void Skybox::ReleaseResources()
{
	SAFE_RELEASE(m_SkyboxTex);
	SAFE_RELEASE(m_SkyboxSRV);
	SAFE_RELEASE(cloudTex1);
	SAFE_RELEASE(cloud1SRV);
	SAFE_RELEASE(cloudTex2);
	SAFE_RELEASE(cloud2SRV);
	SAFE_RELEASE(m_CloudVB);
	SAFE_RELEASE(m_SkydomeLayout);
	SAFE_RELEASE(m_CloudIB);
	//for(int i = 0; i < m_VertexCount; i++)
	//SAFE_RELEASE(m_skyPlane[i]);
}
HRESULT Skybox::RenderSkybox(ID3D11Device* pdevice, const CFirstPersonCamera& cam, ID3D11RenderTargetView* LightBW)
{
	HRESULT hr;
	D3DXMATRIX viewProj = (*cam.GetViewMatrix()) * (*cam.GetProjMatrix());
	D3DXMATRIX invViewProj;
	D3DXMatrixInverse(&invViewProj, NULL, &viewProj);
	D3DXVECTOR3 tL;
	D3DXVec3TransformCoord(&tL, &D3DXVECTOR3(-1,1,1), &invViewProj);
	D3DXVECTOR3 tR;
	D3DXVec3TransformCoord(&tR, &D3DXVECTOR3(1,1,1), &invViewProj);
	D3DXVECTOR3 bL;
	D3DXVec3TransformCoord(&bL, &D3DXVECTOR3(-1,-1,1), &invViewProj);

	D3DXVECTOR3 down = bL - tL;
	D3DXVECTOR3 right = tR - tL;
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDTV = DXUTGetD3D11DepthStencilView();

	V(m_pEffect->GetVariableByName("SkyCubeImage")->AsShaderResource()->SetResource(m_SkyboxSRV));
	V(m_pEffect->GetVariableByName("CloudTex1")->AsShaderResource()->SetResource(cloud1SRV));
	V(m_pEffect->GetVariableByName("CloudTex2")->AsShaderResource()->SetResource(cloud2SRV));
	V(m_pEffect->GetVariableByName("g_SunRadius")->AsScalar()->SetFloat(m_Sun.Radius));
	V(m_pEffect->GetVariableByName("g_SunPos")->AsVector()->SetFloatVector(m_Sun.Position));
	V(m_pEffect->GetVariableByName("SunColor")->AsVector()->SetFloatVector(g_LightColor));
	V(m_pEffect->GetVariableByName("g_Eye")->AsVector()->SetFloatVector(*cam.GetEyePt()));
	V(m_pEffect->GetVariableByName("g_TopLeft")->AsVector()->SetFloatVector(tL));
	V(m_pEffect->GetVariableByName("g_Right")->AsVector()->SetFloatVector(right));
	V(m_pEffect->GetVariableByName("g_Down")->AsVector()->SetFloatVector(down));
	V(m_pEffect->GetVariableByName("g_ViewProj")->AsMatrix()->SetMatrix(viewProj));
	V(m_pEffect->GetVariableByName("cloudTranslation")->AsVector()->SetFloatVectorArray((float*)&textureTranslation[0],0,2));
	m_pEffect->GetVariableByName("g_CamUp")->AsVector()->SetFloatVector(*cam.GetWorldUp());
	m_pEffect->GetVariableByName("g_CamRight")->AsVector()->SetFloatVector(*cam.GetWorldRight());
	V(m_horizontColorEV->SetFloatVector(horizontColor));
	V(m_apexColorEV->SetFloatVector(apexColor));
	m_Context->IASetInputLayout( NULL ); 
	m_Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); 
	m_SkyPass->Apply(0, m_Context); 
	m_Context->Draw(1, 0);
	m_Context->OMSetRenderTargets(1, &LightBW, NULL);
	V(m_SkyboxTechnique->GetPassByName("drawSkyCubeVLS")->Apply(0, m_Context)); 
	m_Context->Draw(1, 0);

	//Sun
	//m_Context->OMSetRenderTargets(1, &pRTV , pDTV);
	const UINT offset = 0;
	UINT stride; stride= sizeof(SpriteVertex);
	//V(m_SkyboxTechnique->GetPassByName("sun")->Apply(0, m_Context)); 
	m_Context->IASetInputLayout( NULL ); 
	m_Context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_POINTLIST); 
	//m_Context->Draw(1, 0);
	//m_Context->OMSetRenderTargets(1, &LightBW, NULL);
	V(m_SkyboxTechnique->GetPassByName("sunBW")->Apply(0, m_Context)); 
	m_Context->Draw(1, 0);
	//Skyplane
	m_Context->OMSetRenderTargets(1, &pRTV , pDTV);
	stride = sizeof(uvVertex);
	V(m_SkyboxTechnique->GetPassByName("Clouds")->Apply(0, m_Context)); 
	m_Context->IASetVertexBuffers(0,1, &m_CloudVB, &stride, &offset);
	m_Context->IASetIndexBuffer(m_CloudIB, DXGI_FORMAT_R32_UINT, 0);
	m_Context->IASetInputLayout( m_SkydomeLayout ); 
	m_Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
	m_Context->DrawIndexed(m_IndexCount,0, 0);
	m_Context->OMSetRenderTargets(1, &LightBW, NULL);
	V(m_SkyboxTechnique->GetPassByName("CloudsBW")->Apply(0, m_Context)); 
	m_Context->DrawIndexed(m_IndexCount,0, 0);
	m_Context->OMSetRenderTargets(1, &pRTV , pDTV);

	return S_OK;
}


void Skybox::OnMove(double time, float elapsedTime)
{
	//Sun Simulaions
	D3DXMATRIX mTmp;
	D3DXMatrixRotationY(&mTmp,static_cast<float>(DEG2RAD(time)*elapsedTime*0.03*m_SunSpeed));
	D3DXVec4Transform(&g_LightDir, &g_LightDir,&mTmp);
	D3DXVec3Normalize((D3DXVECTOR3*)&g_LightDir, (D3DXVECTOR3*)&g_LightDir); // Normalize the light direction for constant light Speed
	D3DXMatrixRotationZ(&mTmp,static_cast<float>(std::abs(DEG2RAD(time)*elapsedTime*0.1f*m_SunSpeed)));
	D3DXVec4Transform(&g_LightDir, &g_LightDir,&mTmp);
	D3DXVec3Normalize((D3DXVECTOR3*)&g_LightDir, (D3DXVECTOR3*)&g_LightDir); // Normalize the light direction for constant light Speed
	m_DeltaSun = ((D3DXVECTOR3)(g_LightDir)*m_SunDistance).y -m_Sun.Position.y;
	m_Sun.Position = (D3DXVECTOR3)(g_LightDir)*m_SunDistance;

	float sunHeight = min(1, max(-1,m_Sun.Position.y/(m_SunDistance*0.7)));

	if(sunHeight > 0){
		if(m_DeltaSun > 0){
			D3DXColorLerp(&horizontColor, &dayColor[0].first, &dayColor[1].first, sunHeight);
			D3DXColorLerp(&apexColor, &dayColor[0].second, &dayColor[1].second, sunHeight);
		}
		else
		{
			D3DXColorLerp(&horizontColor, &dayColor[1].first, &dayColor[2].first, 1-sunHeight);
			D3DXColorLerp(&apexColor, &dayColor[1].second, &dayColor[2].second, 1-sunHeight);
		}
		D3DXColorLerp(&g_LightColor, &D3DXCOLOR(1,0.8,0,1), &m_Sun.Color, sunHeight);
	}
	else
	{
		D3DXColorLerp(&g_LightColor, &D3DXCOLOR(1,0.8,0,1), &D3DXCOLOR(0,0,0,1), sunHeight);
		if(m_DeltaSun > 0){
			D3DXColorLerp(&horizontColor, &dayColor[0].first, &dayColor[3].first, sunHeight*-1);
			D3DXColorLerp(&apexColor, &dayColor[0].second, &dayColor[3].second, sunHeight*-1);
		}
		else
		{
			D3DXColorLerp(&horizontColor, &dayColor[2].first, &dayColor[3].first, sunHeight*-1);
			D3DXColorLerp(&apexColor, &dayColor[2].second, &dayColor[3].second, sunHeight*-1);
		}
	}
		textureTranslation[0] += translationSpeed[0];
		textureTranslation[1] += translationSpeed[1];

		//textureTranslation[0].x > 1.0f ? textureTranslation[0].x -= 1.0f:NULL;
		//textureTranslation[0].y > 1.0f ? textureTranslation[0].y -= 1.0f:NULL;
		//textureTranslation[1].x > 1.0f ? textureTranslation[1].x -= 1.0f:NULL;
		//textureTranslation[1].y > 1.0f ? textureTranslation[1].y -= 1.0f:NULL;
}