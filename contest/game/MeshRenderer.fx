#include "Util.fx"
#include "Filter.fx"
float    g_VSMMinVariance = 0.000001;         // Minimum variance for VSM
//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

float4 cLightAmbient = float4(1,1,1,1);
float cA = 0.1; //Ambient beleuchtung
Texture2D g_ShadowMap;
	Texture2D Diffuse; // Material albedo for diffuse lighting
	Texture2D Normal;
	Texture2D Specular;
	Texture2D Glow;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	float4 g_LightColor;
};

cbuffer cbChangesEveryMesh
{
	//matrix world has to be replaced with InstanceWorld
	matrix mLightViewProj;
	matrix mViewProj;
	matrix mProj;
	matrix mView;
	matrix mViewInv;
	matrix mViewProjInv;
	matrix mProjInv;
	matrix mNormals;
};

cbuffer cbChangesEveryFrame
{
	float4 g_LightDirView;
	matrix g_LightViewProjMatrix; // = g_mLightViewProj
};

struct T3dVertexVSIn 
{ 
	float3 Pos : POSITION; //Position in object space 
	float2 Tex : TEXCOORD; //Texture coordinate 
	float3 Nor : NORMAL; //Normal in object space 
	float3 Tan : TANGENT; //Tangent in object space (not used in Ass. 5) 
    matrix InstanceWorld : POSITION1; //Instance Data
    matrix InstanceWorldInv : INVPOSITION; //Instance Data
}; 

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState samPSVSM
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
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
RasterizerState rsCullBack {
	CullMode = Back;
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
// Shaders
//--------------------------------------------------------------------------------------

struct T3dVertexPSIn 
{ 
	float4 Pos : SV_POSITION; //Position in clip space 
	float4 lightPos : LIGHTPOSITION;
	float2 Tex : TEXCOORD; //Texture coordinate 
	float3 PosView : VIEWPOSITION; //Position in view space 
	float3 NorView : NORMAL; //Normal in view space 
	float3 TanView : TANGENT; //Tangent in view space (not used in Ass. 5) 
};


T3dVertexPSIn MeshVS(T3dVertexVSIn Input) 
{ 
	T3dVertexPSIn output = (T3dVertexPSIn) 0; 

	//matrix WorldViewProjection = mViewProj*Input.InstanceWorld;
	matrix WorldViewNormals = Input.InstanceWorldInv*mNormals;
	//matrix(Input.InstanceWorld[0][0],0,0,Input.InstanceWorld[0][0],0,Input.InstanceWorld[2][2],0,0,0,0,Input.InstanceWorld[3][3],0,Input.InstanceWorld[0][3],Input.InstanceWorld[1][3],Input.InstanceWorld[2][3],1)
	float4 worldPos = mul(float4(Input.Pos,1),Input.InstanceWorld);

	output.Pos = mul(worldPos, mViewProj);
	output.Tex = Input.Tex;
	//output.PosView = mul(float4(Input.Pos,1), WorldView).xyz;
	output.PosView = mul(worldPos, mView).xyz;
	//output.NorView = normalize(mul(float4(Input.Nor,0), WorldViewNormals).xyz);
	output.NorView = normalize(mul(mul(float4(Input.Nor,0),  Input.InstanceWorld), mProjInv).xyz);
	output.TanView = normalize(mul(mul(float4(Input.Tan,0),  Input.InstanceWorld), mProjInv).xyz);
	//output.lightPos = mul(float4(Input.Pos,1), mul(WorldView, g_LightViewProjMatrix));
	//output.lightPos = mul(float4(output.PosView,1), g_LightViewProjMatrix);
	output.lightPos =mul(float4(worldPos.xyz,1), g_LightViewProjMatrix);
	return output; 
}

float4 MeshPS(T3dVertexPSIn Input, out float4 vlsMap : SV_TARGET1) : SV_Target0 { 
	float4 output = (float4)0; //output color 
	vlsMap = 0;
	float4 mDiffuse = Diffuse.Sample(samAnisotropic, Input.Tex);
	float4 mSpecular = Specular.Sample(samAnisotropic, Input.Tex);
	float4 mGlow = Glow.Sample(samAnisotropic, Input.Tex);	
	float3 nT;
	nT.xy = Normal.Sample(samAnisotropic, Input.Tex).xy*2-1;
	nT.z = sqrt(saturate(1 - nT.x*nT.x - nT.y*nT.y));
	//float4 cLight = float4(1,1,1,1);
	float3 N = Input.NorView;
	float3 T = normalize(Input.TanView - N * dot(N,Input.TanView));
	//float3 T = Input.TanView;
	float3 B = cross(T,N);
	float3x3 TBN = {T.x,B.x,N.x,
						T.y,B.y,N.y,
						T.z,B.z,N.z};
	float4 n = float4(mul(TBN,nT),0);
	float4 l = g_LightDirView;
	float4 r = reflect(-l,n);
	float4 v = float4(normalize(-Input.PosView),0);

	float3 lPos = Input.lightPos.xyz/Input.lightPos.w;
	lPos.y = -lPos.y;
	lPos.xy = lPos.xy/2.f+0.5f;
//	lPos.z += -0.0001;
		uint2 dimensions;
	g_ShadowMap.GetDimensions(dimensions.x,dimensions.y);
	float2 t = frac( lPos.xy * dimensions.x );
	float shadowFactor;
	//float shadowFactor = lerp(lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,lPos.xy,lPos.z),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y),lPos.z),t.x),
	//					lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y+1.f/dimensions.y),lPos.z),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x,lPos.y+1.f/dimensions.y),lPos.z),t.x)
	//					,t.y);
	//float shadowFactor = lerp(
	//					lerp(
	//						g_ShadowMap.SampleCmpLevelZero(depthMap,lPos.xy,lPos.z),
	//						g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y),lPos.z),
	//						t.x),
	//					lerp(
	//						g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y+1.f/dimensions.y),lPos.z),
	//						g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x,lPos.y+1.f/dimensions.y),lPos.z),
	//						t.x),
	//					t.y);

	//VSM Shading + PCF
	//float4 shadowTex =g_ShadowMap.Sample(samPSVSM, lPos.xy).rg+GetFPBias();
	float2 moments = TexturePCF(g_ShadowMap, lPos.xy, int2(5,5)).xy +GetFPBias();//g_ShadowMap.Sample(samPSVSM, lPos.xy).rg +GetFPBias();
	float depth = lPos.z; 
	shadowFactor = ChebyshevUpperBound(moments, depth, g_VSMMinVariance);
	shadowFactor = ReduceLightBleeding(shadowFactor, 0.3);
	output = (0.5 * mDiffuse * saturate(dot(n,l)) * g_LightColor
		+ 0.7 * mSpecular * pow(saturate(dot(r,v)),20) * g_LightColor
		+ 0.5 * mGlow + 0.2 * mDiffuse * cLightAmbient) * shadowFactor 
		+ 0.05 * mDiffuse * cLightAmbient * (1.f-shadowFactor);
	return output; 
}
float4 MeshBWPS(T3dVertexPSIn Input) : SV_Target0 { 
	return float4(0,0,0,1); 
}
float2 VSM_DepthPS(T3dVertexPSIn Input) : SV_TARGET {
	return ComputeMoments(Input.Pos.z)-GetFPBias();
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Shadow
{
	//VSM
	pass Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, VSM_DepthPS()));
		
		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
technique11 Render
{
	pass Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MeshPS()));
		
		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}