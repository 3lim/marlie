#include "SpriteRenderer.h"
#include <vector>
#include <set>
#include <fstream>
#include "SDKmisc.h"

using namespace std;
SpriteRenderer::SpriteRenderer(const std::vector<std::pair<std::string,int>>& textureFilenames) : m_textureFilenames(textureFilenames), 
																				m_pEffect(NULL),
																				m_pInputLayout(NULL),
																				m_pVertexBuffer(NULL),
																				m_spriteCountMax(1024)
{
}

SpriteRenderer::~SpriteRenderer()
{

}

HRESULT SpriteRenderer::ReloadShader(ID3D11Device* pDevice)
{
	HRESULT hr;

	WCHAR path[MAX_PATH];
	DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"SpriteRenderer.fxo");
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));    
	assert(m_pEffect->IsValid());
		
	return hr;
}

void SpriteRenderer::ReleaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::CreateResources(ID3D11Device* pDevice)
{
	HRESULT hr;
	hr = S_OK;

	D3D11_BUFFER_DESC bd;

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = m_spriteCountMax * sizeof(struct SpriteVertex);
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V_RETURN(pDevice->CreateBuffer(&bd,NULL,&m_pVertexBuffer));

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",    0, DXGI_FORMAT_R32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTUREINDEX",      0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ASIZE",      0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "APROGRESS",      0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "OPACITY",      0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC pd;
	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->GetDesc(&pd);
	V_RETURN(pDevice->CreateInputLayout(layout, sizeof(layout)/sizeof(layout[0]), pd.pIAInputSignature,pd.IAInputSignatureSize, &m_pInputLayout));
	
	typedef pair<D3D11_TEXTURE2D_DESC,pair<std::vector<D3D11_SUBRESOURCE_DATA>,std::vector<std::vector<unsigned char>>>> tD_type;
	map<int,pair<vector<tD_type>,vector<int>>> tD;
	
	m_textureOffsets.clear();
	m_textureOffsets.resize(m_textureFilenames.size());

	m_spriteSRV.clear();
	m_spriteTex.clear();
	m_spriteSRV.resize(MAXSIZE);
	m_spriteTex.resize(MAXSIZE);

	auto it=m_textureFilenames.begin();
	for(unsigned int i=0;i<m_textureFilenames.size();i++,it++)
	{
		tD_type t;
		bool sRgb;

		LoadNtxArrayFromFiles(it->first,0,it->second-1,&t.first,t.second.second,t.second.first,sRgb);

		for(unsigned int j=0;j<t.first.ArraySize*t.first.MipLevels;j++)
		{
			t.second.first[j].pSysMem = &t.second.second[j][0];
		}
		tD[t.first.Height*t.first.Width].first.push_back(t);
		tD[t.first.Height*t.first.Width].second.push_back(i);
	}

	unsigned int textureCount=0;
	unsigned int resCount=1;
	for(auto iter=tD.begin();iter!=tD.end()&&resCount<=MAXSIZE;iter++,resCount++)
	{
		auto it = *iter;
		if(resCount==MAXSIZE&&tD.size()>MAXSIZE)
		{
			iter++;
			for(;iter!=tD.end();iter++)
			{
				it.second.first.insert(it.second.first.end(),iter->second.first.begin(),iter->second.first.end());
				it.second.second.insert(it.second.second.end(),iter->second.second.begin(),iter->second.second.end());
			}
			iter--;
		}

		unsigned int count=0;
		for(unsigned int i=0;i<it.second.first.size();i++) count+=it.second.first[i].first.ArraySize;

		std::vector<D3D11_SUBRESOURCE_DATA> subresData;
		std::vector<std::vector<unsigned char>> texData;
		D3D11_TEXTURE2D_DESC maxMipmap = it.second.first[0].first;
		subresData.resize(count);
		texData.resize(count);

		unsigned int lastCount = textureCount;
		for(unsigned int i=0;i<it.second.first.size();i++)
		{
			int mipLevel = (int)ceil(log10(it.second.first[i].first.Width/(float)maxMipmap.Width)/log10(2.f));

			for(unsigned int j=0;j<it.second.first[i].first.ArraySize;j++,textureCount++)
			{
				texData[textureCount-lastCount] = it.second.first[i].second.second[j*it.second.first[i].first.MipLevels+mipLevel];
				subresData[textureCount-lastCount] = it.second.first[i].second.first[j*it.second.first[i].first.MipLevels+mipLevel];
				subresData[textureCount-lastCount].pSysMem = &texData[textureCount-lastCount][0];
			}

			m_textureOffsets[it.second.second[i]] = textureCount - it.second.first[i].first.ArraySize;
		}
		
		maxMipmap.ArraySize = count;
		maxMipmap.MipLevels = 1;

		V_RETURN(pDevice->CreateTexture2D(&maxMipmap,&subresData[0],&m_spriteTex[resCount-1]));
		V_RETURN(pDevice->CreateShaderResourceView(m_spriteTex[resCount-1],NULL,&m_spriteSRV[resCount-1]));

	}
	return hr;
}

void SpriteRenderer::ReleaseResources()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexBuffer);
	
	for(int i=0;i<MAXSIZE;i++)
	{
		SAFE_RELEASE(m_spriteTex[i]);
		SAFE_RELEASE(m_spriteSRV[i]);
	}
}

void SpriteRenderer::RenderSprites(ID3D11Device* pDevice, std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera)
{
	ID3D11DeviceContext* dc;
	pDevice->GetImmediateContext(&dc);

	D3D11_BOX box = {0,0,0,sprites.size()*sizeof(struct SpriteVertex),1,1};
	dc->UpdateSubresource(m_pVertexBuffer,0,&box,&sprites[0],sizeof(struct SpriteVertex),0);

	ID3D11Buffer* vb[] = {m_pVertexBuffer,};
	UINT strides[] = {sizeof(struct SpriteVertex),};
	UINT offsets[] = {0,};
	dc->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc->IASetVertexBuffers(0,1,vb,strides,offsets);
	dc->IASetInputLayout(m_pInputLayout);

	D3DXMATRIX vp = (*camera.GetViewMatrix()) * (*camera.GetProjMatrix());
	m_pEffect->GetVariableByName("g_ViewProjection")->AsMatrix()->SetMatrix(vp);
	
	m_pEffect->GetVariableByName("g_CamUp")->AsVector()->SetFloatVector(*camera.GetWorldUp());
	m_pEffect->GetVariableByName("g_CamRight")->AsVector()->SetFloatVector(*camera.GetWorldRight());
	
	m_pEffect->GetVariableByName("g_Textures")->AsShaderResource()->SetResourceArray(&m_spriteSRV[0],0,MAXSIZE);

	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0,dc);

	dc->Draw(sprites.size(),0);

	SAFE_RELEASE(dc);
}

unsigned int SpriteRenderer::GetTextureOffset(int textureIndex)
{
	return m_textureOffsets[textureIndex];
}

int SpriteRenderer::GetAnimationSize(int textureIndex)
{
	return m_textureFilenames[textureIndex].second-1;
}