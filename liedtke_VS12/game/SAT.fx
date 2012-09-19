//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D g_ShadowMap; //Input Image
//Texture2D g_SATimg; //OutputImage

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
float2 imgSize;
float range; //range
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

SamplerState samPoint
{
    Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
	BorderColor = float4(0,0,0,1);
};


//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault {
	CullMode = None; 	
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
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
    DepthFunc = LESS;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

Texel satVS(Vertex input)
{
	Texel Out;
	Out.Pos = input.Pos;
	Out.Tex = input.Tex;
	Out.TexSAT1 = input.Tex + float2(input.Tex.x-1/imgSize.x ,input.Tex.y);
	Out.TexSAT2 = input.Tex + float2(-0.5f/imgSize.x, 0.5f/imgSize.y);
	return Out;
}

uint dummy(uint id : VERTEXID) : POINT
{
	return id;
}
[maxvertexcount(4)]
void GS_x (point uint vertex[1] : POINT, inout TriangleStream<Texel> stream)
{
	float dx = 1.f/imgSize.x;
	Texel o;
	o.Pos = float4(-1,1,0.1,1);
	o.Tex = float2(0,0);
	o.TexSAT1 = o.Tex + 0.5/imgSize;
	o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	stream.Append(o);
	o.Pos = float4(-1,-1,0.1,1);
	o.Tex = float2(0,1);
	o.TexSAT1 = o.Tex + 0.5/imgSize;
	o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	stream.Append(o);
	o.Pos = float4(1,1,0.1,1);
	o.Tex = float2(1,0);
	o.TexSAT1 = o.Tex + 0.5/imgSize;
	o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	stream.Append(o);
	o.Pos = float4(1,-1,0.1,1);
	o.Tex = float2(1,1);
	o.TexSAT1 = o.Tex + 0.5/imgSize;
	o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	stream.Append(o);
	stream.RestartStrip();
	//o.Tex = float2(-1,0);
	//o.TexSAT1 = o.Tex + 0.5/imgSize;
	//o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	//stream.Append(o);
	//o.Tex = float2(0,1);
	//o.TexSAT1 = o.Tex + 0.5/imgSize;
	//o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	//stream.Append(o);
	//o.Tex = float2(1,0);
	//o.TexSAT1 = o.Tex + 0.5/imgSize;
	//o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	//stream.Append(o);
	//o.Tex = float2(1,1);
	//o.TexSAT1 = o.Tex + 0.5/imgSize;
	//o.TexSAT2 = o.Tex + float2(0.5/imgSize.x, -0.5/imgSize.y);
	//stream.Append(o);
	//stream.RestartStrip();

}


float4 horizontal(Texel input) : SV_Target0 {
	float dx = 1.f/imgSize.x;
	float4 result;
	int maxLoop =input.Tex.x/dx;
	//for(int i = 0; i <= 512*input.Tex.x; i+= 1)//schrittweise von links nach rechts alle Pixel aufsumieren
	//{
	//	result += g_ShadowMap.Sample(samPoint, float2(i*dx, input.Tex.y));
	//}
	//return float4(input.Tex,g_ShadowMap.Sample(samPoint, float2(input.Tex.x - dx, input.Tex.y)).r,1); 
	float2 r= float2(range,0);
	//[loop]
	//for(int i = 0; i < imgSize.x; i = i+1)
	//{
	result= (g_ShadowMap.Sample(samPoint, input.Tex) + g_ShadowMap.Sample(samPoint, (input.Tex.x - dx)));
	//	//+ g_ShadowMap.Sample(samPoint, input.Pos+1*pow(r, float2(i,1)))
	//	//+ g_ShadowMap.Sample(samPoint, input.Pos+2*pow(r, float2(i,1))); // 2 = r
	//}
	return result;
}

float4 vertical(Texel input) : SV_Target0 {
	//[loop]
	//for(int i = 0; i < imgSize.x; i++)
	//{

	//}
	float dy = 1.f/imgSize.y;
	float4 result;
	//for(int i = 0; i <= imgSize.y; i+= 1)//schrittweise von links nach rechts alle Pixel aufsumieren
	//{
	//	result +=  g_ShadowMap.Sample(samPoint, float2(i*dy, input.Tex.y));
	//}
	//float2 r= float2(range,0);
	//[loop]
	//for(int i = 0; i < imgPasses.y; i = i+1)
	//{
	//result= g_ShadowMap.Sample(samPoint, input.Pos)
	//	+ g_ShadowMap.Sample(samPoint, input.Pos+1*pow(r, float2(1,i))) // (0,r)^(1,i) == (0,r^i)
	//	+ g_ShadowMap.Sample(samPoint, input.Pos+2*pow(r, float2(1,i))); // 2 = r
	//}
	result= (g_ShadowMap.Sample(samPoint, input.Tex) + g_ShadowMap.Sample(samPoint, (input.Tex.y - dy)));
	return result;
}

float4 satPS(Texel input) : SV_TARGET0
{
	return float4(0.2, 0.3,0,1);
	return g_ShadowMap.Sample(samPoint, input.TexSAT1) + g_ShadowMap.Sample(samPoint, input.TexSAT2)-0.5f;
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 SAT
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, dummy()));
        SetGeometryShader(CompileShader(gs_4_0, GS_x()));
        SetPixelShader(CompileShader(ps_4_0, horizontal()));
        
        SetRasterizerState(rsDefault);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    pass P1
    {
        SetVertexShader(CompileShader(vs_4_0, dummy()));
        SetGeometryShader(CompileShader(gs_4_0, GS_x()));
        SetPixelShader(CompileShader(ps_4_0, vertical()));
        
        SetRasterizerState(rsDefault);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

}

