#include "Mesh.h"

#include "T3d.h"
#include "ntx/NTX.h"

Mesh::Mesh(const char* filename_t3d,
           const char* filename_ntx_diffuse,
           const char* filename_ntx_specular,
           const char* filename_ntx_glow,
		   const char* filename_ntx_normal)
  : m_FilenameT3d        (filename_t3d),
	m_FilenameNtxDiffuse (filename_ntx_diffuse),
	m_FilenameNtxSpecular(filename_ntx_specular),
	m_FilenameNtxGlow    (filename_ntx_glow),
	m_FilenameNtxNormal  (filename_ntx_normal),
	//Default values for all other member variables
    m_VertexBuffer(NULL), m_IndexBuffer(NULL),
	m_IndexCount(0),
	m_DiffuseTex(NULL), m_DiffuseSRV(NULL),
	m_SpecularTex(NULL), m_SpecularSRV(NULL),
	m_GlowTex(NULL), m_GlowSRV(NULL),
	m_NormalTex(NULL), m_NormalSRV(NULL)
{
}

Mesh::~Mesh(void)
{
}

HRESULT Mesh::CreateResources(ID3D11Device* pd3dDevice)
{	
	HRESULT hr;

	// BEGIN: Assignment 5.2.3

	//Some variables that we will need
	D3D11_SUBRESOURCE_DATA idV;
	D3D11_BUFFER_DESC bdV;
	D3D11_SUBRESOURCE_DATA idI;
	D3D11_BUFFER_DESC bdI;
	D3D11_TEXTURE2D_DESC tex2DDesc;
	std::vector<uint8_t> ntxFileContents;
	std::vector<std::vector<uint8_t>> textureData;
	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData;
	bool sRgb;

	//Read mesh
	std::vector<T3dVertex> vertexBufferData;
	std::vector<uint32_t> indexBufferData;
	//TODO: Use T3d::ReadFromFile() to read the contents of the t3d file "m_FilenameT3d"
	//      To obtain a char*, use the c_str() method of std::string.
	T3d::ReadFromFile(m_FilenameT3d.c_str(), vertexBufferData, indexBufferData);
	m_IndexCount = indexBufferData.size();

	//TODO: Set appropriate values in "id" and "bd" and
	//      use V_RETURN( pd3dDevice->CreateBuffer(...) ) to create "m_VertexBuffer"
	//      from "vertexBufferData"
	//HINT: recall the creation of the terrain vertex buffer in assignment 3
		//D3D11_SUBRESOURCE_DATA id;
	idV.pSysMem = &vertexBufferData[0];
	idV.SysMemPitch = sizeof(T3dVertex); // Stride
	idV.SysMemSlicePitch = 0;

	//D3D11_BUFFER_DESC bd;
	bdV.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdV.ByteWidth = sizeof(T3dVertex)*vertexBufferData.size();
	bdV.CPUAccessFlags = 0;
	bdV.MiscFlags = 0;
	bdV.Usage = D3D11_USAGE_DEFAULT;
	V_RETURN(pd3dDevice->CreateBuffer(&bdV, &idV, &m_VertexBuffer)); 
	//Set Buffer


	//TODO: Set appropriate values in "id" and "bd" and
	//      use V_RETURN( pd3dDevice->CreateBuffer(...) ) to create "m_IndexBuffer"
	//      from "indexBufferData". Also set "m_IndexCount" to the size of 
	//      "indexBufferData".
	//HINT: Recall the creation of the terrain index buffer in assignment 3.
	//D3D11_SUBRESOURCE_DATA initD;
	idI.pSysMem = &indexBufferData[0];
	idI.SysMemPitch = sizeof(&indexBufferData[0]);

	//D3D11_BUFFER_DESC bufferDesc;
	bdI.Usage           = D3D11_USAGE_DEFAULT;
	bdI.ByteWidth       = indexBufferData.size()*idI.SysMemPitch; //== numInd *sizeof(unsigned int)
	bdI.BindFlags       = D3D11_BIND_INDEX_BUFFER;
	bdI.CPUAccessFlags  = 0;
	bdI.MiscFlags       = 0;
	V_RETURN(pd3dDevice->CreateBuffer(&bdI, &idI, &m_IndexBuffer));
	//set Buffer
	

	//TODO: Create the textures "m_DiffuseTex", "m_SpecularTex" and "m_GlowTex"
	//      as well as the SRV's  "m_DiffuseSRV", "m_SpecularSRV" and "m_GlowSRV"
	//      from "m_FilenameNtxDiffuse", "m_FilenameNtxSpecular" and "m_FilenameNtxGlow".
	//HINT: For each texture use the following four function calls:
	//      - Mesh::LoadFile(...)
	//      - V_RETURN( LoadNtx(...) );
	//      - V_RETURN( pd3dDevice->CreateTexture2D(...) );
	//      - V_RETURN( pd3dDevice->CreateShaderResourceView(...) );
	//      Recall the creation of the terrain texture resources in assignment 4.
	if(m_FilenameNtxDiffuse.compare("-") != 0){
	Mesh::LoadFile(m_FilenameNtxDiffuse.c_str(), ntxFileContents);
	LoadNtx(ntxFileContents, &tex2DDesc, textureData, subresourceData, sRgb);
	V_RETURN(pd3dDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &m_DiffuseTex));
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_DiffuseTex, NULL, &m_DiffuseSRV));
	}
	if(m_FilenameNtxSpecular.compare("-") != 0){
	Mesh::LoadFile(m_FilenameNtxSpecular.c_str(), ntxFileContents);
	LoadNtx(ntxFileContents, &tex2DDesc, textureData, subresourceData, sRgb);
	V_RETURN(pd3dDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &m_SpecularTex));
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_SpecularTex, NULL, &m_SpecularSRV));
	}
	if(m_FilenameNtxGlow.compare("-") != 0){
	Mesh::LoadFile(m_FilenameNtxGlow.c_str(), ntxFileContents);
	LoadNtx(ntxFileContents, &tex2DDesc, textureData, subresourceData, sRgb);
	V_RETURN(pd3dDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &m_GlowTex));
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_GlowTex, NULL, &m_GlowSRV));
	// END: Assignment 5.2.3
	}
	if(m_FilenameNtxNormal.compare("-") != 0){
	Mesh::LoadFile(m_FilenameNtxNormal.c_str(), ntxFileContents);
	LoadNtx(ntxFileContents, &tex2DDesc, textureData, subresourceData, sRgb);
	V_RETURN(pd3dDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &m_NormalTex));
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_NormalTex, NULL, &m_NormalSRV));
	}
	return S_OK;
}

void Mesh::ReleaseResources()
{
	// BEGIN: Assignment 5.2.4

	//TODO: use the SAFE_RELEASE macro to realease all D3D11 resources that are created in CreateResources()
	SAFE_RELEASE(m_DiffuseTex);
	SAFE_RELEASE(m_DiffuseSRV);
	SAFE_RELEASE(m_SpecularTex);
	SAFE_RELEASE(m_SpecularSRV);
	SAFE_RELEASE(m_GlowTex);
	SAFE_RELEASE(m_GlowSRV);
	SAFE_RELEASE(m_NormalTex);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);
	// END: Assignment 5.2.4
}

HRESULT Mesh::LoadFile(const char * filename, std::vector<uint8_t>& data)
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