TextureCube SkyCubeImage;


cbuffer cbChangesEveryFrame
{
	float3 g_TopLeft;
	float3 g_Down;
	float3 g_Right;
	float3 g_Eye;
	float4 horizontColor;
	float4 apexColor;
};


struct QuadVertex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------
RasterizerState rsCullNone {
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
BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};
	

uint QuadVS(uint id : SV_VERTEXID) : POINT
{
	return id;
}

[maxvertexcount(4)]
void QuadGS(point uint vertex[1] : POINT, inout TriangleStream<QuadVertex> stream)
{
	QuadVertex qv;
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


float4 SkyboxPS(in QuadVertex input) : SV_Target0
{
	float3 pixPosition = normalize((g_TopLeft + input.Tex.x * g_Right + input.Tex.y * g_Down) - g_Eye);
	float height = pixPosition.y;
	if(height < 0)
		height = 0;
	float4 SkyBackColor = lerp(horizontColor, apexColor, height);
	return SkyBackColor;
	return SkyCubeImage.Sample(samAnisotropic, pixPosition)+(SkyBackColor.a - 0.3);
}

technique11 tSkybox
{
	pass drawSkyCube
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
		SetPixelShader(CompileShader(ps_4_0, SkyboxPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}

}