#pragma once

#include <cstdint>
#include <vector>
#include <DXUT.h>
#include "Effects11/Inc/d3dx11effect.h"

//C++ struct for t3d vertex buffer
struct T3dVertex {
	D3DXVECTOR3 position;
	D3DXVECTOR2 texCoord;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent;
};

// Corresponding struct for usage in your effect file
//
//struct T3dVertexVSIn
//{
//    float3 Pos : POSITION; //Position in object space
//    float2 Tex : TEXCOORD; //Texture coordinate
//    float3 Nor : NORMAL;   //Normal in object space
//	  float3 Tan : TANGENT;  //Tangent in object space
//};

class T3d
{
public:
	static HRESULT ReadFromFile(const char* filename, std::vector<T3dVertex>& vertexBufferData, 
                                                      std::vector<uint32_t>& indexBufferData);

	static HRESULT CreateT3dInputLayout(ID3D11Device* pd3dDevice, 
		ID3DX11EffectPass* pass, ID3D11InputLayout** t3dInputLayout);
};

