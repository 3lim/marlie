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
	apexColor(D3DXCOLOR(0.0,0.12,0.8,1))
{
	dayColor[0] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.9, 0.18, 0.12,1), D3DXCOLOR(0.08, 0.2, 0.1 ,1));
	dayColor[1] = pair<D3DXCOLOR, D3DXCOLOR>(D3DXCOLOR(0.34, 0.52, 1,1), D3DXCOLOR(0.08, 0.02, 0.8 ,1));

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

HRESULT Skybox::CreateResources(ID3D11Device* pDevice)
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
	//Sonne
	m_Sun.Opacity = 1;
	//nun im onMove
	m_Sun.Position = (D3DXVECTOR3)(g_LightDir)*m_SunDistance;
	m_Sun.Radius = 100.0f;
	m_Sun.TextureIndex = 0;
	m_Sun.AnimationProgress = 0;
	m_Sun.Color = g_LightColor;

	return S_OK;
}

void Skybox::ReleaseResources()
{
	SAFE_RELEASE(m_SkyboxTex);
	SAFE_RELEASE(m_SkyboxSRV);
}
HRESULT Skybox::RenderSkybox(ID3D11Device* pdevice, const CFirstPersonCamera& cam)
{
	HRESULT hr;
	D3DXMATRIX viewProj = (*cam.GetViewMatrix()) * (*cam.GetProjMatrix());
	D3DXMatrixInverse(&viewProj, NULL, &viewProj);
	D3DXVECTOR3 tL;
	D3DXVec3TransformCoord(&tL, &D3DXVECTOR3(-1,1,1), &viewProj);
	D3DXVECTOR3 tR;
	D3DXVec3TransformCoord(&tR, &D3DXVECTOR3(1,1,1), &viewProj);
	D3DXVECTOR3 bL;
	D3DXVec3TransformCoord(&bL, &D3DXVECTOR3(-1,-1,1), &viewProj);
	
	D3DXVECTOR3 down = bL - tL;
	D3DXVECTOR3 right = tR - tL;

	V(m_pEffect->GetVariableByName("SkyCubeImage")->AsShaderResource()->SetResource(m_SkyboxSRV));
	V(m_pEffect->GetVariableByName("g_Eye")->AsVector()->SetFloatVector(*cam.GetEyePt()));
	V(m_pEffect->GetVariableByName("g_TopLeft")->AsVector()->SetFloatVector(tL));
	V(m_pEffect->GetVariableByName("g_Right")->AsVector()->SetFloatVector(right));
	V(m_pEffect->GetVariableByName("g_Down")->AsVector()->SetFloatVector(down));
	V(m_horizontColorEV->SetFloatVector(horizontColor));
	V(m_apexColorEV->SetFloatVector(apexColor));
	m_Context->IASetInputLayout( NULL ); 
	m_Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); 
	m_SkyPass->Apply(0, m_Context); 
	m_Context->Draw(1, 0);

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
	m_Sun.Position = (D3DXVECTOR3)(g_LightDir)*m_SunDistance;
	float sunHeight = m_Sun.Position.y/m_SunDistance;
	D3DXColorLerp(&horizontColor, &dayColor[0].first, &dayColor[1].first, sunHeight);
	D3DXColorLerp(&apexColor, &dayColor[0].second, &dayColor[1].second, sunHeight);
}