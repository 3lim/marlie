#include "Util.fx"
//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D Source; //Input Image
//Texture2D g_SATimg; //OutputImage

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	uint Samples;
};
cbuffer cbPass
{
	int2 PassOffset;
};

struct Vertex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct Texel
{
	float4 Pos : SV_POSITION;
	float2 Tex  : TEXCOORD0;

	float2 TexSAT1 : TEXCOORD1;
	float2 TexSAT2 : TEXCOORD2;
};
	

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samBorderPoint
{
    AddressU = Border;
    AddressV = Border;
    Filter = MIN_MAG_MIP_POINT;
    BorderColor = float4(0, 0, 0, 1);
};


//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsScissorin {
    CullMode = None;
    FillMode = Solid;
    ScissorEnable = true;
    MultisampleEnable = false;
};



//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
    DepthEnable = false;
    DepthWriteMask = 0;
    StencilEnable = false;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

//Texel satVS(Vertex input)
//{
//	Texel Out;
//	Out.Pos = input.Pos;
//	Out.Tex = input.Tex;
//	Out.TexSAT1 = input.Tex + float2(input.Tex.x-1/imgSize.x ,input.Tex.y);
//	Out.TexSAT2 = input.Tex + float2(-0.5f/imgSize.x, 0.5f/imgSize.y);
//	return Out;
//}

uint dummy(uint id : SV_VertexId) : POINT
{
	return id;
}

[maxvertexcount(4)]
void QuadGS(point uint vertex[1] : POINT, inout TriangleStream<Vertex> stream)
{
	Vertex qv;
	qv.Pos = float4(-1,1, 1, 1);
	qv.Tex = float2(0,0);
	stream.Append(qv);
	qv.Pos = float4(-1,-1, 1, 1);
	qv.Tex = float2(0,1);
	stream.Append(qv);
	qv.Pos = float4(1,1,1,1);
	qv.Tex = float2(1,0);
	stream.Append(qv);
	qv.Pos = float4(1,-1,1,1);
	qv.Tex = float2(1,1);
	stream.Append(qv);
	stream.RestartStrip();

}

//Recursive Double
float4 generateSAT(Vertex In) : SV_Target0
{
	float2 dimension;
	Source.GetDimensions(dimension.x, dimension.y); 
	float2 currentOffset = float2(PassOffset)/dimension;// (1, 0)/dimension oder (0, 1)/dimension
	float4 result;
	for(int i = 0; i < Samples; i++)
	{
		//result = float4(In.Tex.xy,0,1);
		result += Source.SampleLevel(samBorderPoint, In.Tex - i*currentOffset, 0);
	}
	return result;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 SAT
{
	//Write SAT
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, dummy()));
        SetGeometryShader(CompileShader(gs_4_0, QuadGS()));//Fullscreen Quad
        SetPixelShader(CompileShader(ps_4_0, generateSAT()));
        
        SetRasterizerState(rsScissorin);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

}

