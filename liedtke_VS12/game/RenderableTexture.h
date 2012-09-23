#pragma once
#include <DXUT.h>
#include "SDKmisc.h"
//modified from GPU GEMS 3


class RenderableTexture
{
public:
	RenderableTexture(ID3D11Device* d3dDevice, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, const DXGI_SAMPLE_DESC* sampleDesc = 0);
	 RenderableTexture(ID3D11Device* d3dDevice, ID3D11Texture2D* textureArray,
                      unsigned int index);

	~RenderableTexture(void);

  unsigned int GetWidth() const { return m_Width; }
  unsigned int GetHeight() const { return m_Height; }
  unsigned int GetMipLevels() const { return m_MipLevels; }
  DXGI_FORMAT GetFormat() const { return m_Format; }
  bool IsArray() const { return m_Array; }
  unsigned int GetArrayIndex() const { return m_ArrayIndex; }

  ID3D11Texture2D * GetTexture() { return m_Texture; }
  ID3D11RenderTargetView * GetRenderTarget() { return m_RenderTarget; }
  ID3D11ShaderResourceView * GetShaderResource() { return m_ShaderResource; }


private:
	 // Not implemented
  RenderableTexture(const RenderableTexture &);

  unsigned int                        m_Width;
  unsigned int                        m_Height;
  unsigned int                        m_MipLevels;
  DXGI_FORMAT                         m_Format;
  bool                                m_Array;
  unsigned int                        m_ArrayIndex;

  ID3D11Texture2D*                    m_Texture;
  ID3D11RenderTargetView*             m_RenderTarget;
  ID3D11ShaderResourceView*           m_ShaderResource;

};

