#include "TerrainRenderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include "ntx\NTX.h"
#include "Macros.h"
#include "SDKmisc.h"

using namespace std;

vector<unsigned short> TerrainRenderer::g_TerrainHeights;

TerrainRenderer::TerrainRenderer(CHAR path[], float width, float depth, float height, bool spinning, float spinningSpeed) :
	m_TerrainNumVertices(3),
	m_TerrainNumTriangles(1),
	//m_TerrainPath(string(path)),
	m_TerrainWidth(width),
	m_TerrainDepth(depth),
	m_TerrainHeight(height),
	m_TerrainSpinning(spinning),
	m_TerrainSpinSpeed(spinningSpeed),
	m_pEffect(NULL),
	stride(0),
	offset(0)
{
	vbs[0] = NULL;
	m_TerrainPath= std::string(path);

	m_trunk.trunkResolution = 1;
}


TerrainRenderer::~TerrainRenderer(void)
{
}

HRESULT TerrainRenderer::CreateResources(ID3D11Device* pDevice)
{
	HRESULT hr;

	errno_t error;
	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;
	//   // Find the terrain file
	wss.str(L""); wss << m_TerrainPath.c_str();
	V(DXUTFindDXSDKMediaFileCch( path, MAX_PATH, wss.str().c_str()));
	if (hr != S_OK) {
		ss.str();
		ss << "Could not find '" << m_TerrainPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "Missing file", MB_ICONERROR | MB_OK);
		return hr;
	}
	// open the terrain file
	FILE* file;
	error = _wfopen_s(&file, path, TEXT("rb"));
	if (file == nullptr) {
		ss.str();
		ss << "could not open '" << m_TerrainPath << "'";
		MessageBoxA(NULL, ss.str().c_str(), "file error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// read the terrain header
	{
		auto r = fread (&m_TerrainHeader, sizeof (m_TerrainHeader), 1, file);//TODO g_t.. -> PtfH...
		if (r != 1) {
			MessageBoxA(NULL, "could not read the header.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}
	}

	// check the magic number
	if (m_TerrainHeader.magicNumber != 0x00da) {
		MessageBoxA (NULL, "the magic number is incorrect.",
			"invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// check the version
	if (m_TerrainHeader.version != 1) {
		MessageBoxA (NULL, "the header version is incorrect.",
			"invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
	// calculate the terrain resoultion from the height size
	m_TerrainResolution = (int)sqrt(m_TerrainHeader.heightSize / 2.0); // assume a square terrain
	m_TerrainNumVertices = m_TerrainResolution * m_TerrainResolution;
	m_TerrainNumTriangles = 2 * (m_TerrainResolution - 1) * (m_TerrainResolution - 1) ;
	assert((m_TerrainHeader.heightSize / 2) == m_TerrainNumVertices);
	m_TerrainVertexCount = m_TerrainNumTriangles*3;
	// read the terrain heights
	g_TerrainHeights.resize(m_TerrainNumVertices);
	{
		//Kontrolliert, ob die Werte in das array passen
		if (m_TerrainHeader.heightSize != (int)::fread (&g_TerrainHeights[0], sizeof(byte), m_TerrainHeader.heightSize, file)) {
			MessageBoxA(NULL, "error while reading height data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}
	m_trunk.squareLength = m_TerrainResolution/m_trunk.trunkResolution;
	m_trunk.vertexCount = m_trunk.squareLength*m_trunk.squareLength;
	m_trunk.trunksCount = m_trunk.trunkResolution*m_trunk.trunkResolution;

	// read the terrain color texture for diffuse lighting
	std::vector<unsigned char> terraindiffusentx;
	terraindiffusentx.resize(m_TerrainHeader.colorSize);
	{
		const auto requestedsize = terraindiffusentx.size();
		if (requestedsize != fread (&terraindiffusentx[0], sizeof(unsigned char), requestedsize, file)) {
			MessageBoxA (NULL, "error while reading color data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}
	// read the terrain normals texture
	std::vector<unsigned char> terrainNormalNtx;
	terrainNormalNtx.resize(m_TerrainHeader.normalSize);
	{
		const auto requestedsize = terrainNormalNtx.size();
		if (requestedsize != fread (&terrainNormalNtx[0], sizeof(unsigned char), requestedsize, file)) {
			MessageBoxA (NULL, "error while reading normal data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}

	fclose(file);

	// Create the vertex buffer for the terrain
	D3D11_SUBRESOURCE_DATA id;
	id.pSysMem = &g_TerrainHeights[0];
	id.SysMemPitch = sizeof(unsigned short);
	id.SysMemSlicePitch = 0;
	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = m_TerrainNumVertices * id.SysMemPitch;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;	
	pDevice->CreateBuffer(&bd, &id, &m_TerrainHeightBuf);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R16_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_TerrainResolution * m_TerrainResolution;
	V_RETURN(pDevice->CreateShaderResourceView(m_TerrainHeightBuf, &srvDesc, &m_TerrainHeightSRV));
	// END: Assignment 4.2.6

	for(size_t y = 0; y < m_TerrainResolution; y+=m_trunk.squareLength)
		for(size_t x = 0; x < m_TerrainResolution; x+=m_trunk.squareLength)
		{
			Trunk t;
			t.TopLeftX = x;
			t.TopLeftY = y;
			vector<unsigned short> trunkHeights;
			for(size_t i = 0; i < m_trunk.squareLength; i++){
				UINT begin =  y*m_TerrainResolution+x+i*m_trunk.squareLength;
				UINT end = begin +m_trunk.squareLength;
				trunkHeights.insert(trunkHeights.begin()+i*(m_trunk.squareLength), g_TerrainHeights.begin() + begin, g_TerrainHeights.begin() + end);
			}
			//for(int trunkY = y; trunkY < y+ m_trunk.squareLength; trunkY++)
			//for(int trunkX = x; trunkX < x+m_trunk.squareLength; trunkX++)
			//{
			//	trunkHeights.push_back(g_TerrainHeights[trunkY*m_TerrainResolution+trunkX]);
			//}
			//for(int i = 0; i < g_TerrainHeights.size(); i++)
			//	if(trunkHeights[1024] != g_TerrainHeights[1024])
			//		hr = -1;
			D3D11_SUBRESOURCE_DATA id;
			id.pSysMem = &trunkHeights[0];
			id.SysMemPitch = sizeof(unsigned short);
			id.SysMemSlicePitch = 0;
			D3D11_BUFFER_DESC bd;
			bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.ByteWidth = m_trunk.vertexCount * id.SysMemPitch;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;
			bd.Usage = D3D11_USAGE_DEFAULT;	
			pDevice->CreateBuffer(&bd, &id, &t.vbs);
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R16_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = m_trunk.vertexCount;
			V_RETURN(pDevice->CreateShaderResourceView(t.vbs, &srvDesc, &t.srv));
			m_TrunksBuffer.push_back(t);
		}



		D3D11_TEXTURE2D_DESC tex2DDesc;
		std::vector<std::vector<unsigned char>> textureData;
		std::vector<D3D11_SUBRESOURCE_DATA> subresourceData;
		bool sRgb;
		V(LoadNtx(terraindiffusentx, &tex2DDesc, textureData, subresourceData, sRgb));
		if(FAILED(hr))
			return hr;
		V(pDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &m_TerrainDiffuseTex));
		if(FAILED(hr))
			return hr;
		V(pDevice->CreateShaderResourceView(m_TerrainDiffuseTex, NULL, &m_TerrainDiffuseSRV));  

		D3D11_TEXTURE2D_DESC heightDesc;
		vector<vector<float>> heightData;
		heightData.resize(1);
		heightData[0].resize(m_TerrainResolution*m_TerrainResolution);
		for(int i=0;i<m_TerrainResolution*m_TerrainResolution;i++)
		{
			heightData[0][i] = g_TerrainHeights[i]/(float)UINT16_MAX;
		}
		vector<D3D11_SUBRESOURCE_DATA> heightSubres;
		heightSubres.resize(1);
		heightSubres[0].pSysMem = &heightData[0][0];
		heightSubres[0].SysMemPitch = sizeof(float) * m_TerrainResolution;

		heightDesc.ArraySize=1;
		heightDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		heightDesc.CPUAccessFlags = 0;
		heightDesc.Format = DXGI_FORMAT_R32_FLOAT;
		heightDesc.Height = m_TerrainResolution;
		heightDesc.MipLevels = 1;
		heightDesc.MiscFlags = 0;
		heightDesc.SampleDesc.Count = 1;
		heightDesc.SampleDesc.Quality = 0;
		heightDesc.Usage = D3D11_USAGE_DEFAULT;
		heightDesc.Width = m_TerrainResolution;
		pDevice->CreateTexture2D(&heightDesc,&heightSubres[0],&heightTex);
		pDevice->CreateShaderResourceView(heightTex,NULL,&heightSRV);


		// END: Assignment 3.2.5
		// BEGIN: Assigment 4.2.7
		D3D11_TEXTURE2D_DESC normal2DDesc;
		std::vector<std::vector<unsigned char>> normalData;
		subresourceData.empty();
		V(LoadNtx(terrainNormalNtx, &normal2DDesc, normalData, subresourceData, sRgb));
		if(FAILED(hr))
			return hr;
		pDevice->CreateTexture2D(&normal2DDesc, &subresourceData[0], &m_TerrainNormalTex);
		if(FAILED(hr))
			return hr;
		pDevice->CreateShaderResourceView(m_TerrainNormalTex, NULL, &m_TerrainNormalSRV);  


		return S_OK;
}

void TerrainRenderer::ReleaseResources()
{
	SAFE_RELEASE(m_TerrainDiffuseTex);
	SAFE_RELEASE(m_TerrainDiffuseSRV);
	SAFE_RELEASE(m_TerrainNormalTex);
	SAFE_RELEASE(m_TerrainNormalSRV);
	SAFE_RELEASE(m_TerrainHeightBuf);
	SAFE_RELEASE(m_TerrainHeightSRV);
	SAFE_RELEASE(heightTex);
	SAFE_RELEASE(heightSRV);
	for(auto it = m_TrunksBuffer.begin(); it != m_TrunksBuffer.end(); it++)
	{
		SAFE_RELEASE(it->vbs);
		SAFE_RELEASE(it->srv);
	}
}

HRESULT TerrainRenderer::ReloadShader(ID3D11Device* pDevice)
{
	assert(pDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"TerrainRenderer.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));    
	assert(m_pEffect->IsValid());

	SAFE_GET_TECHNIQUE(m_pEffect, "Render", m_RenderET);
	SAFE_GET_TECHNIQUE(m_pEffect, "Shadow", m_ShadowET);

	SAFE_GET_PASS(m_RenderET, "P0", m_RenderTerrain);
	SAFE_GET_PASS(m_ShadowET, "VSM", m_ShadowTerrain);

	SAFE_GET_RESOURCE(m_pEffect, "g_ShadowMap", m_ShadowMapEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Height", m_TerrainHeightEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Normal", m_TerrainNormalEV);
	SAFE_GET_RESOURCE(m_pEffect, "g_Diffuse", m_TerrainDiffuseEV);

	SAFE_GET_MATRIX(m_pEffect, "g_World", m_WorldEV);

	SAFE_GET_SCALAR(m_pEffect, "g_TerrainRes", m_TerrainResEV);
	SAFE_GET_SCALAR(m_pEffect, "g_TerrainQuadRes", m_TerrainQuadsEV);

	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", m_ViewProjectionEV);
	SAFE_GET_VECTOR(m_pEffect, "g_LightDir", m_LightDirEV);
	SAFE_GET_MATRIX(m_pEffect, "g_LightViewProjMatrix", m_LightViewProjMatrixEV);

	return S_OK;
}

void TerrainRenderer::ReleaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

void TerrainRenderer::Deinit()
{
}

D3DXVECTOR4 objLightDir;
D3DXMATRIX terrainWorldLightViewProj;
D3DXMATRIX terrainViewProj;
void TerrainRenderer::setEffectVariables()
{
	m_TerrainNormalEV->SetResource(m_TerrainNormalSRV);
	m_TerrainDiffuseEV->SetResource(m_TerrainDiffuseSRV);
	m_TerrainHeightEV->SetResource(m_TerrainHeightSRV/*m_TrunksBuffer[0].srv*/);

	m_WorldEV->SetMatrix(m_World);

	m_TerrainResEV->SetInt(m_TerrainResolution);
	m_TerrainQuadsEV->SetInt(m_TerrainResolution-1);
	terrainViewProj = m_World**g_ViewProj;
	terrainWorldLightViewProj = m_World**g_LightViewProjMatrix;
	m_ViewProjectionEV->SetMatrix(terrainViewProj);
	D3DXMATRIX invWorld;
	D3DXMatrixInverse(&invWorld, NULL, &m_World);
	D3DXVec4Transform(&objLightDir, g_LightDir, &invWorld);
	D3DXVec3Normalize((D3DXVECTOR3*)&objLightDir, (D3DXVECTOR3*)&objLightDir); 
	m_LightDirEV->SetFloatVector(objLightDir);
	m_LightViewProjMatrixEV->SetMatrix(terrainWorldLightViewProj);
}

void TerrainRenderer::OnMove(double time, float elapsedTime)
{
	D3DXMATRIX mTmp;
	// Set origin to (0.5, 0.5, 0)
	D3DXMatrixTranslation(&m_World, -0.5f, -0.5f, 0.f); // Assume: x, y and z are in [0,1]

	// Scale to terrain extents
	D3DXMatrixScaling(&mTmp, m_TerrainWidth, m_TerrainDepth, m_TerrainHeight);
	m_World *= mTmp;

	// Since "up" is z-axis in object space, but y-axis in world space, we rotate around the x-axis
	D3DXMatrixRotationX( &mTmp, DEG2RAD( -90.0f ) );
	m_World *= mTmp;

	if( m_TerrainSpinning ) {
		D3DXMatrixRotationY( &mTmp, m_TerrainSpinSpeed * DEG2RAD((float)time) );
		m_World *= mTmp; // Rotate around world-space "up" axis
	}
}

void TerrainRenderer::RenderTerrain(ID3D11Device* pDevice, RenderableTexture* shadowMap)
{
	ID3D11DeviceContext* pd3dImmediateContext;
	pDevice->GetImmediateContext(&pd3dImmediateContext);

	m_ShadowMapEV->SetResource(shadowMap->GetShaderResource());
	setEffectVariables();
	m_pEffect->GetVariableByName("g_TrunkRes")->AsScalar()->SetInt(m_trunk.squareLength);
	m_pEffect->GetVariableByName("g_TrunkQuadRes")->AsScalar()->SetInt(m_trunk.squareLength-1);
	m_TerrainResEV->SetInt(m_trunk.squareLength);
	m_TerrainQuadsEV->SetInt(m_trunk.squareLength-1);
	for(int i = 0; i < m_trunk.trunksCount; i++)
	{
		m_TerrainHeightEV->SetResource(m_TrunksBuffer[i].srv);
		m_pEffect->GetVariableByName("offsetIndex")->AsScalar()->SetInt(m_TrunksBuffer[i].TopLeftY*m_TerrainResolution+ m_TrunksBuffer[i].TopLeftX);
		// Apply the rendering pass in order to submit the necessary render state changes to the device
		//m_RenderTerrain->Apply(0, pd3dImmediateContext);
		// Set input layout

		pd3dImmediateContext->IASetInputLayout( NULL );
		// Bind the terrain vertex buffer to the input assembler stage 
		pd3dImmediateContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
		// Tell the input assembler stage which primitive topology to use
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->Draw(m_TerrainVertexCount/m_trunk.trunksCount, 0);
	}
	//pd3dImmediateContext->OMSetRenderTargets(0, NULL, dsv);
	//m_pEffect->GetTechniqueByName("Render")->GetPassByName("P2")->Apply(0, pd3dImmediateContext);
	//pd3dImmediateContext->Draw(m_TerrainVertexCount/16, 0);
	/*
	//	D3DXMATRIX reflectM = D3DXMATRIX(1.f,0.f,0.f,0.f,0.f,-1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f);
	/*
	D3DXPLANE reflectP = D3DXPLANE(0,1,0,1);
	D3DXMatrixReflect(&reflectM,&reflectP);*/
	//	m_ViewProjectionEV->SetMatrix(terrainViewProj * reflectM);
	//	pd3dImmediateContext->OMSetRenderTargets(1, &reflectionRTV, NULL);
	//	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0, pd3dImmediateContext);
	//pd3dImmediateContext->Draw(m_TerrainVertexCount, 0);

	m_ShadowMapEV->SetResource(0);
	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0, pd3dImmediateContext);

	SAFE_RELEASE(pd3dImmediateContext);
}
void TerrainRenderer::ShadowTerrain(ID3D11Device* pDevice)
{
	ID3D11DeviceContext* pd3dImmediateContext;
	pDevice->GetImmediateContext(&pd3dImmediateContext);
	setEffectVariables();
	// Apply the rendering pass in order to submit the necessary render state changes to the device
	m_ShadowTerrain->Apply(0 , pd3dImmediateContext);
	// Set input layout
	pd3dImmediateContext->IASetInputLayout( NULL );
	// Bind the terrain vertex buffer to the input assembler stage 
	pd3dImmediateContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
	// Tell the input assembler stage which primitive topology to use
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Draw
	pd3dImmediateContext->Draw(m_TerrainVertexCount, 0);
	SAFE_RELEASE(pd3dImmediateContext);
}

float TerrainRenderer::getHeightAtPoint(float x, float y) {
	return getHeightAtPoint(static_cast<int>(min(m_TerrainResolution - 1, (0.5f + x / m_TerrainWidth) * m_TerrainResolution)), static_cast<int>(min(m_TerrainResolution - 1, (0.5f - y / m_TerrainDepth) * m_TerrainResolution)));
}

float TerrainRenderer::getHeightAtPoint(int TerrainX, int TerrainY) {
	return m_TerrainHeight * g_TerrainHeights[(int)(max(0,min(TerrainX, m_TerrainResolution-1)) + m_TerrainResolution * max(0,min(TerrainY, m_TerrainResolution-1)))] / (float)UINT16_MAX;
}

D3DXVECTOR3 TerrainRenderer::getNormalAtPoint(float x, float y, size_t size)
{
	int midX = static_cast<int>(min(m_TerrainResolution - 1, (0.5f + x / m_TerrainWidth) * m_TerrainResolution));
	int midY = static_cast<int>(min(m_TerrainResolution - 1, (0.5f - y / m_TerrainDepth) * m_TerrainResolution));

	return D3DXVECTOR3(getHeightAtPoint(midX-size, midY) - getHeightAtPoint(midX+size, midY), getHeightAtPoint(midX, midY-size) - getHeightAtPoint(midX, midY+size), 1.f);
}

