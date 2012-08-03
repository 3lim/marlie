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
	m_FilenameNtxNormal    (filename_ntx_normal),
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
	D3D11_SUBRESOURCE_DATA id;
	D3D11_BUFFER_DESC bd;
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
	T3d::ReadFromFile(m_FilenameT3d.c_str(),vertexBufferData,indexBufferData);

	//TODO: Set appropriate values in "id" and "bd" and
	//      use V_RETURN( pd3dDevice->CreateBuffer(...) ) to create "m_VertexBuffer"
	//      from "vertexBufferData"
	//HINT: recall the creation of the terrain vertex buffer in assignment 3

	id.pSysMem = &vertexBufferData[0];
	id.SysMemPitch = sizeof(struct T3dVertex);
	id.SysMemSlicePitch = 0;

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = vertexBufferData.size() * sizeof(struct T3dVertex);
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V_RETURN( pd3dDevice->CreateBuffer(&bd,&id,&m_VertexBuffer) );

	//TODO: Set appropriate values in "id" and "bd" and
	//      use V_RETURN( pd3dDevice->CreateBuffer(...) ) to create "m_IndexBuffer"
	//      from "indexBufferData". Also set "m_IndexCount" to the size of 
	//      "indexBufferData".
	//HINT: Recall the creation of the terrain index buffer in assignment 3.
	id.pSysMem = &indexBufferData[0];
	id.SysMemPitch = sizeof(uint32_t);
	id.SysMemSlicePitch = 0;

	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth = sizeof(uint32_t) * indexBufferData.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V_RETURN( pd3dDevice->CreateBuffer(&bd,&id,&m_IndexBuffer) );
	m_IndexCount = indexBufferData.size();

	//TODO: Create the textures "m_DiffuseTex", "m_SpecularTex" and "m_GlowTex"
	//      as well as the SRV's  "m_DiffuseSRV", "m_SpecularSRV" and "m_GlowSRV"
	//      from "m_FilenameNtxDiffuse", "m_FilenameNtxSpecular" and "m_FilenameNtxGlow".
	//HINT: For each texture use the following four function calls:
	//      - Mesh::LoadFile(...)
	//      - V_RETURN( LoadNtx(...) );
	//      - V_RETURN( pd3dDevice->CreateTexture2D(...) );
	//      - V_RETURN( pd3dDevice->CreateShaderResourceView(...) );
	//      Recall the creation of the terrain texture resources in assignment 4.
	if(strcmp(m_FilenameNtxDiffuse.c_str(),"-")!=0)
	{
		Mesh::LoadFile(m_FilenameNtxDiffuse.c_str(),ntxFileContents);
		V_RETURN( LoadNtx(ntxFileContents,&tex2DDesc,textureData,subresourceData,sRgb) );
		V_RETURN( pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&m_DiffuseTex) );
		V_RETURN( pd3dDevice->CreateShaderResourceView(m_DiffuseTex,NULL,&m_DiffuseSRV) );
	}
	
	if(strcmp(m_FilenameNtxSpecular.c_str(),"-")!=0)
	{
		Mesh::LoadFile(m_FilenameNtxSpecular.c_str(),ntxFileContents);
		V_RETURN( LoadNtx(ntxFileContents,&tex2DDesc,textureData,subresourceData,sRgb) );
		V_RETURN( pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&m_SpecularTex) );
		V_RETURN( pd3dDevice->CreateShaderResourceView(m_SpecularTex,NULL,&m_SpecularSRV) );
	}
	
	if(strcmp(m_FilenameNtxGlow.c_str(),"-")!=0)
	{
		Mesh::LoadFile(m_FilenameNtxGlow.c_str(),ntxFileContents);
		V_RETURN( LoadNtx(ntxFileContents,&tex2DDesc,textureData,subresourceData,sRgb) );
		V_RETURN( pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&m_GlowTex) );
		V_RETURN( pd3dDevice->CreateShaderResourceView(m_GlowTex,NULL,&m_GlowSRV) );
	}

	if(strcmp(m_FilenameNtxNormal.c_str(),"-")!=0)
	{
		Mesh::LoadFile(m_FilenameNtxNormal.c_str(),ntxFileContents);
		V_RETURN( LoadNtx(ntxFileContents,&tex2DDesc,textureData,subresourceData,sRgb) );
		V_RETURN( pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&m_NormalTex) );
		V_RETURN( pd3dDevice->CreateShaderResourceView(m_NormalTex,NULL,&m_NormalSRV) );
	}

	// END: Assignment 5.2.3

	return S_OK;
}

void Mesh::ReleaseResources()
{
	// BEGIN: Assignment 5.2.4

	//TODO: use the SAFE_RELEASE macro to realease all D3D11 resources that are created in CreateResources()
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);
	SAFE_RELEASE(m_DiffuseTex);
	SAFE_RELEASE(m_DiffuseSRV);
	SAFE_RELEASE(m_SpecularTex);
	SAFE_RELEASE(m_SpecularSRV);
	SAFE_RELEASE(m_GlowTex);
	SAFE_RELEASE(m_GlowSRV);
	SAFE_RELEASE(m_NormalTex);
	SAFE_RELEASE(m_NormalSRV);

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