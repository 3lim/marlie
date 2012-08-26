#pragma once

#include <string>
#include <vector>
#include <map>

#include <DXUT.h>
#include <DXUTcamera.h>

#include "Effects11/Inc/d3dx11effect.h"
#include "ntx\NTX.h"
#include "Mesh.h"


struct SpriteVertex
{
	D3DXVECTOR3 Position; // world-space position (sprite center)
	float Radius;         // world-space radius (= half side length of the sprite quad)
	unsigned int TextureIndex;     // which texture to use (out of SpriteRenderer::m_spriteSRV)
	int AnimationSize;
	float AnimationProgress;
	float Opacity;
	D3DXCOLOR Color;
};

class SpriteRenderer
{
public:
	// Constructor: Create a SpriteRenderer with the given list of textures.
	// The textures are *not* be created immediately, but only when CreateResources is called!
	SpriteRenderer(const std::vector<std::pair<std::string,int>>& textureFilenames);
	// Destructor does nothing. ReleaseResources and ReleaseShader must be called first!
	~SpriteRenderer();

	// Load/reload the effect. Must be called once before CreateResources!
	HRESULT ReloadShader(ID3D11Device* pDevice);
	// Release the effect again.
	void ReleaseShader();

	// Create all required D3D resources (textures, buffers, ...).
	// ReloadShader must be called first!
	HRESULT CreateResources(ID3D11Device* pDevice);
	// Release D3D resources again.
	void ReleaseResources();

	// Render the given sprites. They must already be sorted into back-to-front order.
	void RenderSprites(ID3D11Device* pDevice, const CFirstPersonCamera& camera);
	void RenderGUI(ID3D11Device* pDevice, const CFirstPersonCamera& camera);
	unsigned int GetTextureOffset(int textureIndex);

	int GetAnimationSize(int textureIndex);

	static const unsigned int MAXSIZE = 5;
	static std::vector<SpriteVertex> g_GUISprites;
	static std::vector<SpriteVertex> g_SpritesToRender;
private:

	std::vector<std::pair<std::string,int>> m_textureFilenames;
	std::vector<int> m_textureOffsets;

	// Rendering effect (shaders and related GPU state). Created/released in Reload/ReleaseShader.
	ID3DX11Effect* m_pEffect;

	// Sprite textures and corresponding shader resource views.
	std::vector<ID3D11Texture2D*>          m_spriteTex;
	std::vector<ID3D11ShaderResourceView*> m_spriteSRV;

	// Maximum number of allowed sprites, i.e. size of the vertex buffer.
	size_t m_spriteCountMax;
	// Vertex buffer for sprite vertices, and corresponding input layout.
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_pInputLayout;
};
