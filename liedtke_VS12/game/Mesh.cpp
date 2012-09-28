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
    m_VertexBuffer(NULL), m_InstanceBuffer(NULL),
	m_IndexCount(0),
	m_DiffuseTex(NULL), m_DiffuseSRV(NULL),
	m_SpecularTex(NULL), m_SpecularSRV(NULL),
	m_GlowTex(NULL), m_GlowSRV(NULL),
	m_NormalTex(NULL), m_NormalSRV(NULL),
	m_pMeshInstanceList(NULL)
{
	m_MeshInstanceMatrices[MAX_MESH_INSTANCES];
}

Mesh::~Mesh(void)
{
}

HRESULT Mesh::CreateInstanceLayout(ID3D11Device* pd3dDevice, 
	ID3DX11EffectPass* pass, ID3D11InputLayout** t3dInputLayout)
								{
	HRESULT hr;

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
	D3DX11_PASS_DESC pd;
	V_RETURN(pass->GetDesc(&pd));
	V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, pd.pIAInputSignature,
			  pd.IAInputSignatureSize, t3dInputLayout ) );

	return S_OK;
}

HRESULT Mesh::CreateResources(ID3D11Device* pd3dDevice)
{	
	HRESULT hr;

	// BEGIN: Assignment 5.2.3

	//Some variables that we will need
	D3D11_SUBRESOURCE_DATA VertexSubData, IndexSubData, InstanceSubData;
	D3D11_BUFFER_DESC VertexBufferDesc, IndexBufferDesc, InstanceBufferDesc;
	D3D11_TEXTURE2D_DESC tex2DDesc;
	std::vector<uint8_t> ntxFileContents;
	std::vector<std::vector<uint8_t>> textureData;
	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData;
	bool sRgb;

	//Read mesh
	std::vector<T3dVertex> vertexBufferData;
	std::vector<uint32_t> indexBufferData;
	MeshInstanceType* instances;
	std::vector<D3DXVECTOR3> vertecies;

	T3d::ReadFromFile(m_FilenameT3d.c_str(), vertexBufferData, indexBufferData);
	m_IndexCount = indexBufferData.size();
	m_VertexCount = vertexBufferData.size();
	vertecies = readVerticesFromStream(&vertexBufferData);
	V(D3DXComputeBoundingSphere(&vertecies[0], vertecies.size(), 0, &m_centerVertex, &m_sphereRadius));

	VertexSubData.pSysMem = &vertexBufferData[0];
	VertexSubData.SysMemPitch = sizeof(T3dVertex); // Stride
	VertexSubData.SysMemSlicePitch = 0;

	//D3D11_BUFFER_DESC bd;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.ByteWidth = sizeof(T3dVertex)*vertexBufferData.size();
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	V_RETURN(pd3dDevice->CreateBuffer(&VertexBufferDesc, &VertexSubData, &m_VertexBuffer)); 

	//Instance buffer
	instances = new MeshInstanceType[1];
	instances[0].position = D3DXVECTOR3(0,0,0);
	InstanceSubData.pSysMem = instances;
	InstanceSubData.SysMemPitch = sizeof(MeshInstanceType);

	//D3D11_BUFFER_DESC bufferDesc;
	InstanceBufferDesc.Usage           = D3D11_USAGE_DEFAULT;
	InstanceBufferDesc.ByteWidth       = 1*InstanceSubData.SysMemPitch; //== numInd *sizeof(unsigned int)
	InstanceBufferDesc.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
	InstanceBufferDesc.CPUAccessFlags  = 0;
	InstanceBufferDesc.MiscFlags       = 0;
	V_RETURN(pd3dDevice->CreateBuffer(&InstanceBufferDesc, &InstanceSubData, &m_InstanceBuffer));
	delete[] instances;
	instances = 0;

	IndexSubData.pSysMem = &indexBufferData[0];
	IndexSubData.SysMemPitch = sizeof(uint32_t);

	//D3D11_BUFFER_DESC bufferDesc;
	IndexBufferDesc.Usage           = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth       = m_IndexCount*IndexSubData.SysMemPitch; //== numInd *sizeof(unsigned int)
	IndexBufferDesc.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags  = 0;
	IndexBufferDesc.MiscFlags       = 0;
	V_RETURN(pd3dDevice->CreateBuffer(&IndexBufferDesc, &IndexSubData, &m_IndexBuffer));
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

std::vector<D3DXVECTOR3> Mesh::readVerticesFromStream(std::vector<T3dVertex>* input)
{
	std::vector<D3DXVECTOR3> result;
	result.resize(input->size());
	for(int i = 0; i < input->size(); i++)
		result[i] = (*input)[i].position;
	return result;

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
	SAFE_RELEASE(m_InstanceBuffer);
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