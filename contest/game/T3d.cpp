#include "T3d.h"

#include <sstream>
#include "SDKmisc.h"

using namespace std;

struct T3dHeader {
	int16_t magicNumber; // Must be 0x003D 
	int16_t version;     // Must be 1 
	int32_t verticesSize;  // vertex buffer data size 
	int32_t indicesSize;   // index buffer data size 
}; // Sizes are always in bytes

HRESULT T3d::ReadFromFile(const char* filename, std::vector<T3dVertex>& vertexBufferData, 
                                        std::vector<uint32_t>& indexBufferData)
{
	HRESULT hr;
    WCHAR path[MAX_PATH];
    stringstream ss;
    wstringstream wss;

    // Find the terrain file
    wss.str(L""); wss << filename;
    V(DXUTFindDXSDKMediaFileCch( path, MAX_PATH, wss.str().c_str()));
	if (hr != S_OK) {
        ss.str();
        ss << "Could not find '" << filename << "'";
        MessageBoxA (NULL, ss.str().c_str(), "Missing file", MB_ICONERROR | MB_OK);
		return hr;
	}

    // Open the terrain file
    FILE* file;
	/*errno_t error =*/ _wfopen_s(&file, path, L"rb");
	if (file == nullptr) {
        ss.str();
        ss << "Could not open '" << filename << "'";
        MessageBoxA (NULL, ss.str().c_str(), "File error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

    // Read the terrain header
	T3dHeader header;
	{
		auto r = fread (&header, sizeof (T3dHeader), 1, file);
		if (r != 1) {
			MessageBoxA (NULL, "Could not read the header.",
				"Invalid t3d file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}
	}

    // Check the magic number
	if (header.magicNumber != 0x003D) {
		MessageBoxA (NULL, "The magic number is incorrect.",
			"Invalid t3d file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

    // Check the version
	if (header.version != 1) {
		MessageBoxA (NULL, "The header version is incorrect.",
			"Invalid t3d file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	//Read vertex buffer
	vertexBufferData.resize(header.verticesSize / sizeof(T3dVertex));
	fread(&vertexBufferData[0], sizeof(T3dVertex), vertexBufferData.size(), file);

	//Read index buffer
	indexBufferData.resize(header.indicesSize / sizeof(uint32_t));
	fread(&indexBufferData[0], sizeof(uint32_t), indexBufferData.size(), file);

	fclose(file);

	return S_OK;
}


HRESULT T3d::CreateT3dInputLayout(ID3D11Device* pd3dDevice, 
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
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
	D3DX11_PASS_DESC pd;
	V_RETURN(pass->GetDesc(&pd));
	V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, pd.pIAInputSignature,
			  pd.IAInputSignatureSize, t3dInputLayout ) );

	return S_OK;
}