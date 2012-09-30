Texture2D heightData;
Texture2D textures[4];

cbuffer changePerChunk
{
	float3 quadWorldPos;
	uint2 dims;
};

cbuffer changePerFrame
{
	matrix worldViewProj;
	matrix lightViewProj;
};

cbuffer simplexData
{
	float F2;
	float G2;
	uint p[256];
	uint perm[512];
	uint permMod12[512];
	int3 grad3[12];
};

cbuffer constant
{
	float heightScale;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

DepthStencilState NoDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};

struct vertexOut
{
	float3 wPos : POSITION;
	float3 normal : NORMAL;
};

int fastfloor(float x) {
    int xi = (int)x;
    return x<xi ? xi-1 : xi;
}

float simplexNoise(float2 pos)
{
	float n0, n1, n2; 
    float s = (pos.x+pos.y)*F2;
    int i = fastfloor(pos.x+s);
    int j = fastfloor(pos.y+s);
    float t = (i+j)*G2;
    float X0 = i-t;
    float Y0 = j-t;
    float x0 = pos.x-X0;
    float y0 = pos.y-Y0;
    int i1, j1;
    if(x0>y0) {i1=1; j1=0;}
    else {i1=0; j1=1;}
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0 + 2.0 * G2;
    float y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;
    int gi0 = permMod12[ii+perm[jj]];
    int gi1 = permMod12[ii+i1+perm[jj+j1]];
    int gi2 = permMod12[ii+1+perm[jj+1]];

    float t0 = 0.5 - x0*x0-y0*y0;
    if(t0<0) n0 = 0.0;
    else {
      t0 *= t0;
      n0 = t0 * t0 * dot(grad3[gi0].xy, float2(x0, y0));
    }
    float t1 = 0.5 - x1*x1-y1*y1;
    if(t1<0) n1 = 0.0;
    else {
      t1 *= t1;
      n1 = t1 * t1 * dot(grad3[gi1].xy, float2(x1, y1));
    }
    float t2 = 0.5 - x2*x2-y2*y2;
    if(t2<0) n2 = 0.0;
    else {
      t2 *= t2;
      n2 = t2 * t2 * dot(grad3[gi2].xy, float2(x2, y2));
    }

    return 70.0 * (n0 + n1 + n2);
}

struct PosTex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

void terraGen_VS(in uint vId : SV_VertexID, out uint output : POINT)
{
	output = vId;
}

[maxvertexcount(4)]
void terraGen_GS(point uint vertex[1] : POINT, inout TriangleStream<PosTex> stream)
{
	PosTex output;
	output.Pos = float4(-1,1, 0.1, 1);
	output.Tex = float2(0,0);
	stream.Append(output);
	output.Pos = float4(-1,-1, 0.1, 1);
	output.Tex = float2(0,1);
	stream.Append(output);
	output.Pos = float4(1,1,0.1,1);
	output.Tex = float2(1,0);
	stream.Append(output);
	output.Pos = float4(1,-1,0.1,1);
	output.Tex = float2(1,1);
	stream.Append(output);
	stream.RestartStrip();
}

float terraGen_PS(in PosTex input) : SV_Target0
{
	return simplexNoise((quadWorldPos.xy + input.Tex*dims)*0.0005f)*0.5;
}

vertexOut genVertices_VS(uint vId : SV_VertexID)
{
	vertexOut output = (vertexOut) 0;
	
	uint posX = vId % dims.x;
	uint posY = vId / dims.x;

	output.wPos.x = posX/(float)dims.x;
	output.wPos.y = posY/(float)dims.x;
	output.wPos.z = heightData.SampleLevel(samAnisotropic,(float2(posX,posY)-quadWorldPos.xy)/(float)dims,0);
		
	output.normal.x = heightData.SampleLevel(samAnisotropic,(float2(posX,posY)-quadWorldPos.xy+int2(-1,0))/(float)dims,0) - heightData.SampleLevel(samAnisotropic,(output.wPos.xy-quadWorldPos.xy+int2(1,0))/dims,0);
	output.normal.y = heightData.SampleLevel(samAnisotropic,(float2(posX,posY)-quadWorldPos.xy+int2(0,-1))/(float)dims,0) - heightData.SampleLevel(samAnisotropic,(output.wPos.xy-quadWorldPos.xy+int2(0,1))/dims,0);
	output.normal.z = 1.f;

	output.normal.xy *= heightScale;
	output.normal = normalize(output.normal);

	return output;
}

[maxvertexcount(1)]
void genVertices_GS(point vertexOut input[1], inout PointStream<vertexOut> stream)  
{
	stream.Append(input[0]);
}

struct RenderPS_IN
{
	float4 pos : SV_POSITION;
	float4 lightPos : LIGHTPOSITION;
	float3 normal : NORMAL;
	float absHeight : HEIGHT;
};

RenderPS_IN RenderVS(vertexOut input)
{
	RenderPS_IN output = (RenderPS_IN)0;
	output.absHeight = input.wPos.y;
	
	output.pos.xyz = input.wPos.xzy;
	output.pos.w = 1.f;
	output.pos = mul(output.pos,worldViewProj);
	output.lightPos = mul(float4(input.wPos,1),lightViewProj);
	output.normal = input.normal;

	return output;
}

float4 RenderPS(RenderPS_IN input) : SV_Target0
{
	float steepness = 1-input.normal.z;

	return float4(saturate(steepness),0.2,0.7,1);
}

technique11 Generate
{
	pass DrawTexture
	{
		SetVertexShader(CompileShader(vs_4_0, terraGen_VS()));
		SetGeometryShader(CompileShader(gs_4_0, terraGen_GS()));
		SetPixelShader(CompileShader(ps_4_0, terraGen_PS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(NoDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass GenerateVertices
	{
		SetVertexShader(CompileShader(vs_4_0, genVertices_VS()));
		SetGeometryShader(ConstructGSWithSO(CompileShader(gs_4_0, genVertices_GS()),"POSITION.xyz;NORMAL.xyz",NULL,NULL,NULL,0));
		SetPixelShader(NULL);
	}
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, RenderVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, RenderPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}

