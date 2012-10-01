#pragma once
#include <DXUT.h>
#include "Effects11\Inc\d3dx11effect.h"
#include <map>
#include <set>
#include "SDKmisc.h"
#include <fstream>
#include <vector>
#include <DXUT.h>
#include "DXUT\Optional\DXUTcamera.h"
#include "FrustumCulling.h"
#include "Macros.h"

using namespace std;

struct DataC
{
	float x,y,z,w;
	DataC(float a, float b, float c, float d)
	{
		x=a;
		y=b;
		z=c;
		w=d;
	}

	bool operator<(DataC o) const
	{
		return x*y<o.x*o.y;
	}

	bool operator==(DataC o) const
	{
		return x==o.x&&y==o.y&&z==o.z&&w==o.w;
	}
};
struct VertexOut
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
};
class TerraGen
{
public:

	TerraGen(void);

	~TerraGen(void);

	HRESULT CreateResources(ID3D11Device* pDevice);

	HRESULT ReleaseResources();

	HRESULT ReloadShader(ID3D11Device* pDevice);

	void OnFrameMove(CFirstPersonCamera* camera, FrustumCulling* frustumCulling, float dTime, float time);

	void OnFrameRender(ID3D11Device* pDevice);

	void CreateChunk(DataC data, ID3D11Device* pDevice);
private:
	static const unsigned int chunkDimX = 1024;
	static const unsigned int chunkDimY = 1024;
	float HEIGHT;
	float HEIGHTSCALE;
	ID3DX11Effect* shader;
	ID3D11ShaderResourceView* heightMapSRV;
	ID3D11RenderTargetView* heightMapRTV;
	ID3D11Texture2D* heightMapTex;
	D3D11_VIEWPORT heightMapVP;
	map<DataC,ID3D11Buffer*> vertexBuffers;
	ID3D11InputLayout* inputLayout;
	FrustumCulling* fc;
	set<DataC> chunksToContain;
	CFirstPersonCamera* lc;
	D3DXMATRIX w;
	D3DXMATRIX vp;
	ID3D11Buffer* indexBuffer;

	void GenChunks(D3DXVECTOR3 pos, D3DXVECTOR3* next, UINT count, UINT dist);
	bool CheckPoint(D3DXVECTOR3 p);
};

