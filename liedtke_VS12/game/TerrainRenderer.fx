#include "Util.fx"
#include "Filter.fx"
//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Buffer<float> g_Height;
Texture2D g_Normal;
Texture2D g_Diffuse; // Material albedo for diffuse lighting
Texture2D g_ShadowMap;
//use VSMap for VarianceShading
float4 cLightAmbient = float4(1,1,1,1);
matrix  g_World;
float g_VSMMinVariance = 0.000001;
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
	float2 g_ShadowTexSize;
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
SamplerState samPSVSM
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
};
SamplerState samDepthMap
{
	Filter = MIN_MAG_MIP_LINEAR;
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
	float dx = 1.f / g_TerrainRes;
	int2 coords;
	coords.x = quadIdx % g_TerrainQuadRes;
	coords.y = quadIdx / g_TerrainQuadRes;
	
	if(inQuadIdx==1||inQuadIdx==4) coords.x++;
	if(inQuadIdx==2||inQuadIdx==3) coords.y++;
	if(inQuadIdx==5) {coords.x++;coords.y++;}
	
	output.Tex.x = coords.x * dx;
	output.Tex.y = coords.y *dx;

	float4 pos;
	pos.x = output.Tex.x;
	pos.y = output.Tex.y;
	pos.z = g_Height[coords.y*g_TerrainRes + coords.x];
	pos.w = 1;

	output.lightPos = mul(pos,g_LightViewProjMatrix);

	output.Pos = mul(pos,g_ViewProjection);

	return output;
}
float4 TerrainPS(PosTex Input, out float4 vlsMap : SV_TARGET1) : SV_Target0 {
	float3 n;
	vlsMap = 0;
	n.xy = g_Normal.Sample(samAnisotropic, Input.Tex).xy*2-1;
	n.z = sqrt(saturate(1 - n.x*n.x - n.y*n.y));

	float3 lPos = Input.lightPos.xyz/Input.lightPos.w;
	lPos.y = -lPos.y;
	lPos.xy = lPos.xy/2.f+0.5f;
	uint2 dimensions;
	float i = saturate(dot(g_LightDir.xyz, n));
	float shadowFactor;
	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);

		//OLD Shading
	//g_VSMap.GetDimensions(dimensions.x,dimensions.y);
	//float2 t = frac( lPos.xy * dimensions.x );
	//float shadowFactor = lerp(lerp(g_VSMap.SampleCmpLevelZero(depthMap,lPos.xy,lPos.z-0.01f),g_VSMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y),lPos.z-0.01f),t.x),
	//					lerp(g_VSMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),g_VSMap.SampleCmpLevelZero(depthMap,float2(lPos.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),t.x),t.y);
	//
	

	//VSM Shading + BilinearFiltering
	//float2 dtdx = ddx(lPos);
	//float2 dtdy = ddy(lPos);
	float2 moments = TexturePCF(g_ShadowMap, lPos.xy, int2(10,10)).xy +GetFPBias();//g_ShadowMap.Sample(samPSVSM, lPos.xy).rg +GetFPBias();
	//VSM Shading + PCF
	//float4 shadowTex =g_ShadowMap.Sample(samPSVSM, lPos.xy).rg+GetFPBias();
	float depth = lPos.z; //distant to Lightsource

 //   float2 TexelSize = 1 / g_ShadowTexSize;
 //   float2 MinFilterWidth = float2(4,4);
	//float2 MaxFilterWidth = float2(32,32);
 //   // Compute the filter tile information
 //   // Don't clamp the filter area since we have constant-time filtering!
 //   float2 Size;
 //   float2 CoordsUL = GetFilterTile(tc, dx, dy, g_ShadowTexSize,
 //                                   MinFilterWidth, MaxFilterWidth, Size);

 //   // Compute bilinear weights and coordinates
 //   float4 BilWeights;
 //   float2 BilCoordsUL = GetBilCoordsAndWeights(CoordsUL, g_ShadowTexSize, BilWeights);
 //   float4 Tile = BilCoordsUL.xyxy + float4(0, 0, Size.xy);
 //   
 //   // Read the moments and compute a Chebyshev upper bound
 //   float ShadowContrib = BilinearChebyshev(Tile, BilWeights, Distance, g_VSMMinVariance);

	shadowFactor = ChebyshevUpperBound(moments, depth, g_VSMMinVariance);
	shadowFactor = ReduceLightBleeding(shadowFactor, 0.14);

	return i * matDiffuse * shadowFactor 
		+ 0.05 * i * matDiffuse * cLightAmbient * (1.f-shadowFactor);
	
}

float4 TerrainBWPS(void) : SV_Target0
{
	return float4(0,0,0,1);
}
//returns r = depth, g = moment2 of VSM Shading, b = SummendArea information
float2 VSM_DepthPS(PosTex Input) : SV_TARGET {
	float bias = 0.3;
	return ComputeMoments(Input.Pos.z)-GetFPBias();//z value ist kein besonders guter tiefen wert aber muss für den anfang genügen
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
	pass P1//BW result for Light Scattering
	{
		SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TerrainBWPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
