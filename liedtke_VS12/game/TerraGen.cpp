#include "TerraGen.h"

HRESULT TerraGen::CreateResources(ID3D11Device* pDevice)
{
	HRESULT hr;
	ReloadShader(pDevice);

	shader->GetConstantBufferByName("simplexData")->GetMemberByName("F2")->AsScalar()->SetFloat(0.5*(sqrt(3.0)-1.0));
	shader->GetConstantBufferByName("simplexData")->GetMemberByName("G2")->AsScalar()->SetFloat((3.0-sqrt(3.0))/6.0);
	
	const int p[256] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

	shader->GetConstantBufferByName("simplexData")->GetMemberByName("p")->AsScalar()->SetIntArray(p,0,256);

	int perm[512];
    int permMod12[512];
    for(int i=0; i<512; i++)
    {
      perm[i]=p[i & 255];
      permMod12[i] = (perm[i] % 12);
    }

	shader->GetConstantBufferByName("simplexData")->GetMemberByName("perm")->AsScalar()->SetIntArray(perm,0,512);
	shader->GetConstantBufferByName("simplexData")->GetMemberByName("F2")->AsScalar()->SetIntArray(permMod12,0,512);
	
	D3DXVECTOR3 grad3[] = {D3DXVECTOR3(1,1,0),D3DXVECTOR3(-1,1,0),D3DXVECTOR3(1,-1,0),D3DXVECTOR3(-1,-1,0),
                                 D3DXVECTOR3(1,0,1),D3DXVECTOR3(-1,0,1),D3DXVECTOR3(1,0,-1),D3DXVECTOR3(-1,0,-1),
                                 D3DXVECTOR3(0,1,1),D3DXVECTOR3(0,-1,1),D3DXVECTOR3(0,1,-1),D3DXVECTOR3(0,-1,-1)};
	shader->GetConstantBufferByName("simplexData")->GetMemberByName("grad3")->AsVector()->SetFloatVectorArray((const float*)&grad3,0,12);

	heightMapVP = D3D11_VIEWPORT();
	heightMapVP.Height = chunkDimY;
	heightMapVP.Width = chunkDimX;
	heightMapVP.TopLeftX = 0;
	heightMapVP.TopLeftY = 0;
	heightMapVP.MaxDepth = 1.f;
	heightMapVP.MinDepth = 0.f;

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC pd;
	shader->GetTechniqueByName("Render")->GetPassByName("P0")->GetDesc(&pd);
	V_RETURN(pDevice->CreateInputLayout(layout, sizeof(layout)/sizeof(layout[0]), pd.pIAInputSignature,pd.IAInputSignatureSize, &inputLayout));
	
	shader->GetConstantBufferByName("constant")->GetMemberByName("heightScale")->AsScalar()->SetFloat(HEIGHTSCALE);
	
	D3D11_TEXTURE2D_DESC tDesc;
	tDesc.ArraySize = 1;
	tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tDesc.CPUAccessFlags = 0;
	tDesc.Format = DXGI_FORMAT_R32_FLOAT;
	tDesc.Height = chunkDimX;
	tDesc.MipLevels = 1;
	tDesc.MiscFlags = 0;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_DEFAULT;
	tDesc.Width = chunkDimY;

	pDevice->CreateTexture2D(&tDesc,NULL,&heightMapTex);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = tDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	pDevice->CreateRenderTargetView(heightMapTex,&rtvDesc,&heightMapRTV);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = tDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	pDevice->CreateShaderResourceView(heightMapTex, &srvDesc, &heightMapSRV);

	vector<unsigned int> indexData;
	indexData.resize((chunkDimX-1)*(chunkDimY-1)*6);

	D3D11_SUBRESOURCE_DATA indexSubres;
	D3D11_BUFFER_DESC bufferDesc =
	{
		(chunkDimX-1)*(chunkDimY-1)*6*sizeof(unsigned int),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0,
		0
	};
	for(int i=0;i<(chunkDimX-1)*(chunkDimY-1)*6;i++)
	{
		int quadId = i/6+i/(6*(chunkDimX-1));
		int inQuad = i%6;

		indexData[i] = quadId;
		if(inQuad==1||inQuad==4||inQuad==5) indexData[i] += 1;
		if(inQuad==2||inQuad==3||inQuad==5) indexData[i] += chunkDimX;
	}
	indexSubres.pSysMem = &indexData[0];
	indexSubres.SysMemPitch = sizeof(unsigned int);
	pDevice->CreateBuffer( &bufferDesc, &indexSubres, &indexBuffer );
	return hr;
}

HRESULT TerraGen::ReloadShader(ID3D11Device* pDevice)
{
	HRESULT hr;

	WCHAR path[MAX_PATH];
	DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"TerraGen.fxo");
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &shader));    
	assert(shader->IsValid());
		
	return hr;
}

void TerraGen::CreateChunk(DataC data, ID3D11Device* pDevice)
{
	ID3D11DeviceContext* pContext;
	pDevice->GetImmediateContext(&pContext);
	ID3D11RenderTargetView* oldRTV;
	ID3D11DepthStencilView* oldDSV;
	D3D11_VIEWPORT oldVP;
	UINT one = 1;

	float color[4] = {0.f,0.f,0.f,0.f};
	pContext->ClearRenderTargetView(heightMapRTV,color);
	pContext->OMGetRenderTargets(1,&oldRTV,&oldDSV);
	pContext->RSGetViewports(&one,&oldVP);
	pContext->OMSetRenderTargets(1,&heightMapRTV,NULL);
	pContext->RSSetViewports(1,&heightMapVP);
	pContext->IASetInputLayout(NULL);
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3DXVECTOR3 quadWorldPos = D3DXVECTOR3(data.x,HEIGHT,data.y);
	shader->GetConstantBufferByName("changePerChunk")->GetMemberByName("quadWorldPos")->AsVector()->SetFloatVector(quadWorldPos);
	int dims[2] = {chunkDimX,chunkDimY};
	shader->GetConstantBufferByName("changePerChunk")->GetMemberByName("dims")->AsVector()->SetIntVector(dims);
	
	shader->GetTechniqueByName("Generate")->GetPassByName("DrawTexture")->Apply(0,pContext);
	pContext->Draw(1,0);

	pContext->OMSetRenderTargets(1,&oldRTV,oldDSV);
	pContext->RSSetViewports(1,&oldVP);
	
	shader->GetVariableByName("heightData")->AsShaderResource()->SetResource(heightMapSRV);
	ID3D11Buffer *chunkBuffer;
	D3D11_BUFFER_DESC bufferDesc =
	{
		(chunkDimX)*(chunkDimY)*sizeof(VertexOut),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_STREAM_OUTPUT|D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	pDevice->CreateBuffer( &bufferDesc, NULL, &chunkBuffer );
	UINT offset[1] = {0,};

	pContext->SOSetTargets(1,&chunkBuffer,offset);
	shader->GetTechniqueByName("Generate")->GetPassByName("GenerateVertices")->Apply(0,pContext);
	pContext->Draw(chunkDimX*chunkDimY,0);
	shader->GetVariableByName("heightData")->AsShaderResource()->SetResource(0);
	shader->GetTechniqueByName("Generate")->GetPassByName("GenerateVertices")->Apply(0,pContext);
	string name = "TerraGen_VertexBuffer";
	chunkBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name), (const void*)&name);
	vertexBuffers[data] = chunkBuffer;
	pContext->SOSetTargets(0,NULL,NULL);
	SAFE_RELEASE(pContext);
}

void TerraGen::OnFrameMove(CFirstPersonCamera* camera, FrustumCulling* frustumCulling, float dTime, float time)
{
	//chunksToContain.clear();
	fc = frustumCulling;
	lc = camera;
	vp = *lc->GetViewMatrix() * *lc->GetProjMatrix();
	const D3DXVECTOR3* camPos = camera->GetEyePt();
	const float chunkX = ((int)camPos->x/chunkDimX)*chunkDimX;
	const float chunkY = ((int)camPos->z/chunkDimY)*chunkDimY;
	GenChunks(D3DXVECTOR3(chunkX,HEIGHT,chunkY),NULL,0,0);
	
	D3DXVECTOR3 next1[1] = {D3DXVECTOR3(chunkX,HEIGHT,chunkY+2*chunkDimY)};
	D3DXVECTOR3 next3[3] = {D3DXVECTOR3(chunkX-2*chunkDimX,HEIGHT,chunkY+chunkDimY),D3DXVECTOR3(chunkX-2*chunkDimX,HEIGHT,chunkY+2*chunkDimY),D3DXVECTOR3(chunkX-chunkDimX,HEIGHT,chunkY+2*chunkDimY)};
	
	GenChunks(D3DXVECTOR3(chunkX-chunkDimX,HEIGHT,chunkY+chunkDimY),next3,3,0);
	GenChunks(D3DXVECTOR3(chunkX,HEIGHT,chunkY+chunkDimY),next1,1,0);
	
	next3[0] = D3DXVECTOR3(chunkX+2*chunkDimX,HEIGHT,chunkY+chunkDimY);
	next3[1] = D3DXVECTOR3(chunkX+2*chunkDimX,HEIGHT,chunkY+2*chunkDimY);
	next3[2] = D3DXVECTOR3(chunkX+chunkDimX,HEIGHT,chunkY+2*chunkDimY);
	GenChunks(D3DXVECTOR3(chunkX+chunkDimX,HEIGHT,chunkY+chunkDimY),next3,3,0);

	next1[0] = D3DXVECTOR3(chunkX-2*chunkDimX,HEIGHT,chunkY);
	GenChunks(D3DXVECTOR3(chunkX-chunkDimX,HEIGHT,chunkY),next1,1,0);

	next1[0] = D3DXVECTOR3(chunkX+2*chunkDimX,HEIGHT,chunkY);
	GenChunks(D3DXVECTOR3(chunkX+chunkDimX,HEIGHT,chunkY),next1,1,0);

	next3[0] = D3DXVECTOR3(chunkX-2*chunkDimX,HEIGHT,chunkY-chunkDimY);
	next3[1] = D3DXVECTOR3(chunkX-2*chunkDimX,HEIGHT,chunkY-2*chunkDimY);
	next3[2] = D3DXVECTOR3(chunkX-chunkDimX,HEIGHT,chunkY-2*chunkDimY);
	GenChunks(D3DXVECTOR3(chunkX-chunkDimX,HEIGHT,chunkY-chunkDimY),next3,3,0);
	
	next1[0] = D3DXVECTOR3(chunkX,HEIGHT,chunkY-2*chunkDimY);
	GenChunks(D3DXVECTOR3(chunkX,HEIGHT,chunkY-chunkDimY),next1,1,0);
	
	next3[0] = D3DXVECTOR3(chunkX+2*chunkDimX,HEIGHT,chunkY-chunkDimY);
	next3[1] = D3DXVECTOR3(chunkX+2*chunkDimX,HEIGHT,chunkY-2*chunkDimY);
	next3[2] = D3DXVECTOR3(chunkX+chunkDimX,HEIGHT,chunkY-2*chunkDimY);
	GenChunks(D3DXVECTOR3(chunkX+chunkDimX,HEIGHT,chunkY-chunkDimY),next3,3,0);
}

void TerraGen::GenChunks(D3DXVECTOR3 pos, D3DXVECTOR3* next, UINT count, UINT dist)
{
	if(CheckPoint(pos)||CheckPoint(pos+D3DXVECTOR3(chunkDimX,0,0))||CheckPoint(pos+D3DXVECTOR3(0,0,chunkDimY))||CheckPoint(pos+D3DXVECTOR3(chunkDimX,0,chunkDimY))
		||CheckPoint(pos+D3DXVECTOR3(0,HEIGHTSCALE,0))||CheckPoint(pos+D3DXVECTOR3(chunkDimX,HEIGHTSCALE,0))||CheckPoint(pos+D3DXVECTOR3(0,0,chunkDimY))||CheckPoint(pos+D3DXVECTOR3(chunkDimX,HEIGHTSCALE,chunkDimY)
		))
	{
		chunksToContain.insert(DataC(pos.x,pos.z,chunkDimX,chunkDimY));

		if(count==1)
		{
			D3DXVECTOR3 nnext[1] = {next[0]+next[0]-pos};
			GenChunks(next[0],nnext,1,dist+1);
		}
		else if(count==3)
		{
			D3DXVECTOR3 nnext1[1] = {next[0]+next[0]-pos};
			D3DXVECTOR3 nnext2[3] = {next[1]+next[0]-pos,next[1]+next[0]-pos+next[2]-pos,next[1]+next[2]-pos};
			D3DXVECTOR3 nnext3[1] = {next[2]+next[2]-pos};
			GenChunks(next[0],nnext1,1,dist+1);
			GenChunks(next[1],nnext2,3,dist+1);
			GenChunks(next[2],nnext3,1,dist+1);
		}
	}
}

void TerraGen::OnFrameRender(ID3D11Device* pDevice)
{
	ID3D11DeviceContext* pContext;
	pDevice->GetImmediateContext(&pContext);
		
	for(auto i = chunksToContain.begin();i!=chunksToContain.end();i++)
	{
		if(vertexBuffers.find(*i)==vertexBuffers.end()) CreateChunk(*i,pDevice);


		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->IASetInputLayout(inputLayout);

		ID3D11Buffer* buffer[1] = {vertexBuffers.at(*i)};
		UINT offsets[1] = {0};
		UINT strides[1] = {sizeof(VertexOut)};
		pContext->IASetVertexBuffers(0,1,buffer,strides,offsets);
		pContext->IASetIndexBuffer(indexBuffer,DXGI_FORMAT_R32_UINT,0);

		shader->GetConstantBufferByName("changePerChunk")->GetMemberByName("quadWorldPos")->AsVector()->SetFloatVector(D3DXVECTOR3(i->x,HEIGHT,i->y));
		D3DXMATRIX wvp;
		D3DXMatrixTranslation(&wvp,i->x/(float)chunkDimX,HEIGHT,i->y/(float)chunkDimY);
		wvp = wvp * w;
		wvp	*= vp;
		shader->GetConstantBufferByName("changePerFrame")->GetMemberByName("worldViewProj")->AsMatrix()->SetMatrix(wvp);
		shader->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0,pContext);

		pContext->DrawIndexed((chunkDimX-1)*(chunkDimY-1)*6,0,0);
	}
	SAFE_RELEASE(pContext);
}

HRESULT TerraGen::ReleaseResources()
{
	for(auto i=vertexBuffers.begin();i!=vertexBuffers.end();i++)
	{
		SAFE_RELEASE(i->second);
	}

	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(heightMapRTV);
	SAFE_RELEASE(heightMapSRV);
	SAFE_RELEASE(heightMapTex);

	return S_OK;
}

TerraGen::TerraGen()
{
	HEIGHT = 0.f;
	HEIGHTSCALE = 400.f;	
	
	D3DXMatrixScaling(&w, chunkDimX, chunkDimY, HEIGHTSCALE);
}

inline bool TerraGen::CheckPoint(D3DXVECTOR3 p)
{
	D3DXVECTOR4 projP;
	D3DXVec3Transform(&projP,&p,&(*lc->GetViewMatrix() * *lc->GetProjMatrix()));
	return fc->IsPointInFrustum((D3DXVECTOR3)projP);
}