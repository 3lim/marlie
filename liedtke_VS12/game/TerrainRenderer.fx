//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Buffer<float> g_Height;
Texture2D g_Normal;
Texture2D g_Diffuse; // Material albedo for diffuse lighting
Texture2D g_ShadowMap;
float4 cLightAmbient = float4(1,1,1,1);
matrix  g_World;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	int g_TerrainRes;
	int g_TerrainQuadRes;
};

cbuffer cbChangesEveryFrame
{
	matrix  g_ViewProjection;
	float4 g_LightDir;
	matrix g_LightViewProjMatrix;
};


struct PosTex
{
	float4 Pos : SV_POSITION;
	float4 lightPos : LIGHTPOSITION;
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

SamplerComparisonState depthMap
{
	Filter = COMPARISON_MIN_MAG_MIP_POINT;
	ComparisonFunc = LESS;
	AddressU = Clamp;
	AddressV = Clamp;
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


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
PosTex TerrainVS(uint VertexID : SV_VertexID)
{
	PosTex output = (PosTex) 0;

	uint quadIdx = VertexID / 6;
	uint inQuadIdx = VertexID % 6;

	int2 coords;
	coords.x = quadIdx % g_TerrainQuadRes;
	coords.y = quadIdx / g_TerrainQuadRes;
	
	if(inQuadIdx==1||inQuadIdx==4) coords.x++;
	if(inQuadIdx==2||inQuadIdx==3) coords.y++;
	if(inQuadIdx==5) {coords.x++;coords.y++;}
	
	output.Tex.x = coords.x * 1.f / g_TerrainRes;
	output.Tex.y = coords.y * 1.f / g_TerrainRes;

	float4 pos;
	pos.x = output.Tex.x;
	pos.y = output.Tex.y;
	pos.z = g_Height[coords.y*g_TerrainRes + coords.x];
	pos.w = 1;

	output.lightPos = mul(pos,g_LightViewProjMatrix);

	output.Pos = mul(pos,g_ViewProjection);

	return output;
}

float4 TerrainPS(PosTex Input) : SV_Target0 {
	float3 n;
	n.xy = g_Normal.Sample(samAnisotropic, Input.Tex).xy*2-1;
	n.z = sqrt(saturate(1 - n.x*n.x - n.y*n.y));

	float3 lPos = Input.lightPos.xyz/Input.lightPos.w;
	lPos.y = -lPos.y;
	lPos.xy = lPos.xy/2.f+0.5f;
	uint2 dimensions;
	g_ShadowMap.GetDimensions(dimensions.x,dimensions.y);
	float2 t = frac( lPos.xy * dimensions.x );
	float shadowFactor = lerp(lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,lPos.xy,lPos.z-0.01f),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y),lPos.z-0.01f),t.x),
						lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),t.x),t.y);
	
	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
	float i = saturate(dot(g_LightDir.xyz, n));

	return i * matDiffuse * shadowFactor 
		+ 0.05 * i * matDiffuse * cLightAmbient * (1.f-shadowFactor);
}
float2 ComputeMoments(float Depth)
{
	float2 Moments;
	Moments.x = Depth;

	float dx = ddx(Depth);
	float dy = ddy(Depth);
	Moments.y = Depth*Depth+0.25*(dx*dx+dy*dy);
	return Moments;
}
float2 VSM_DepthPS(PosTex Input) : SV_TARGET {
	return ComputeMoments(Input.Pos.z);
}
//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Shadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass VSM
	{
		SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, VSM_DepthPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
