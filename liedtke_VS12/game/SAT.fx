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
float2 imgPasses; //log r(x|y);
float range; //range
};


struct inTex
{
	float2 Pos  : TEXCOORD;
};
	

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT ;
    AddressU = Clamp;
    AddressV = Clamp;
};


//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault {
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
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};



float4 horizontal(inTex input) :SV_TARGET0{
	//[loop]
	//for(int i = 0; i < imgSize.x; i++)
	//{

	//}
	float2 r= float2(range,0);
	float4 result;
	[loop]
	for(int i = 0; i < imgPasses.x; i = i+1)
	{
	result= g_ShadowMap.Sample(samPoint, input.Pos);
		//+ g_ShadowMap.Sample(samPoint, input.Pos+1*pow(r, float2(i,1)))
		//+ g_ShadowMap.Sample(samPoint, input.Pos+2*pow(r, float2(i,1))); // 2 = r
	}
	result = float4(0,0,0,1);
	return result;
}

float4 vertical(inTex input) :SV_TARGET0 {
	//[loop]
	//for(int i = 0; i < imgSize.x; i++)
	//{

	//}
	float2 r = float2(0,range);
	float4 result;
	[loop]
	for(int i = 0; i < imgPasses.y; i = i+1)
	{
	result= g_ShadowMap.Sample(samPoint, input.Pos)
		+ g_ShadowMap.Sample(samPoint, input.Pos+1*pow(r, float2(1,i))) // (0,r)^(1,i) == (0,r^i)
		+ g_ShadowMap.Sample(samPoint, input.Pos+2*pow(r, float2(1,i))); // 2 = r
	}
	return result;
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 SAT
{
    pass P0
    {
        SetVertexShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, horizontal()));
        
        SetRasterizerState(rsDefault);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    pass P1
    {
        SetVertexShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, vertical()));
        
        SetRasterizerState(rsDefault);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

}

