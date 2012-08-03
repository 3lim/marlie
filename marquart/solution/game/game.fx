//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D   g_Diffuse; // Material albedo for diffuse lighting
Buffer<float> g_Height;
Texture2D g_Normal;
Texture2D g_Specular; // Material albedo/color for specular reflections 
Texture2D g_Glow; // Material albedo/color for self-emission
TextureCube g_Skybox;
Texture2D g_ShadowMap;


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	float4  g_LightDir; // Object space
	int g_TerrainRes;
	int g_TerrainQuadRes;
};

cbuffer cbChangesEveryFrame
{
	matrix  g_World;
	matrix  g_WorldViewProjection;
	float   g_Time;
	matrix g_WorldView; //WorldView transformation 
	matrix g_WorldViewNormals; //WorldView transformation for normals 
	float4 g_LightDirView; //Light direction in view space
	float3 g_Eye;
	float3 g_TopLeft;
	float3 g_Right;
	float3 g_Down;
	matrix g_mLightViewProj;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct PosNorTex
{
	float4 Pos : SV_POSITION;
	float4 Nor : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PosTexLi
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float   Li : LIGHT_INTENSITY;
};

struct PosTex
{
	float4 Pos : SV_POSITION;
	float4 lightPos : LIGHTPOSITION;
	float2 Tex : TEXCOORD;
};

struct T3dVertexVSIn 
{ 
	float3 Pos : POSITION; //Position in object space 
	float2 Tex : TEXCOORD; //Texture coordinate 
	float3 Nor : NORMAL; //Normal in object space 
	float3 Tan : TANGENT; //Tangent in object space (not used in Ass. 5) 
}; 

struct T3dVertexPSIn 
{ 
	float4 Pos : SV_POSITION; //Position in clip space 
	float4 lightPos : LIGHTPOSITION;
	float2 Tex : TEXCOORD; //Texture coordinate 
	float3 PosView : VIEWPOSITION; //Position in view space 
	float3 NorView : NORMAL; //Normal in view space 
	float3 TanView : TANGENT; //Tangent in view space (not used in Ass. 5) 
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

SamplerState samLinearClamp
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
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

RasterizerState rsDefault {
};

RasterizerState rsCullFront {
	CullMode = Front;
};

RasterizerState rsCullBack {
	CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

RasterizerState rsLineAA {
	CullMode = None; 
	AntialiasedLineEnable = true;
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

PosTexLi SimpleVS(PosNorTex Input) {
	PosTexLi output = (PosTexLi) 0;

	// Transform position from object space to homogenious clip space
	output.Pos = mul(Input.Pos, g_WorldViewProjection);

	// Pass trough normal and texture coordinates
	output.Tex = Input.Tex;

	// Calculate light intensity
	float3 n = normalize(mul(Input.Nor, g_World).xyz); // Assume orthogonal matrix
	output.Li = dot(n, g_LightDir.xyz);
		
	return output;
}

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

	output.lightPos = mul(pos,mul(g_World,g_mLightViewProj));

	output.Pos = mul(pos,g_WorldViewProjection);

	return output;
}

T3dVertexPSIn MeshVS(T3dVertexVSIn Input) 
{ 
	T3dVertexPSIn output = (T3dVertexPSIn) 0; 

	output.Pos = mul(float4(Input.Pos,1),g_WorldViewProjection);
	output.Tex = Input.Tex;
	output.PosView = mul(float4(Input.Pos,1),g_WorldView).xyz;
	output.NorView = normalize(mul(float4(Input.Nor,0),g_WorldViewNormals).xyz);
	output.TanView = normalize(mul(float4(Input.Tan,0),g_WorldViewNormals).xyz);
	output.lightPos = mul(float4(Input.Pos,1),mul(g_WorldView,g_mLightViewProj));
	return output; 
}

void QuadVS(uint id : SV_VertexID, out float4 Pos : SV_POSITION, out float2 Tex : TEXCOORD)
{
	Pos = float4(id%2==0?-1:1, id/2==0?-1:1, 1, 1);
	Tex = float2(id%2==0? 0:1, id/2==0? 1:0);
}

float4 SimplePS(PosTexLi Input) : SV_Target0 {
	// Perform lighting in object space, so that we can use the input normal "as it is"
	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
	return float4(matDiffuse.rgb * Input.Li, 1);
}

float4 TerrainPS(PosTex Input) : SV_Target0 {
	float4 cLightAmbient = float4(1,1,1,1);
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

	return i * matDiffuse * shadowFactor + 0.05 * i * matDiffuse * cLightAmbient * (1.f-shadowFactor);
}

float4 MeshPS(T3dVertexPSIn Input) : SV_Target0 { 
	float4 output = (float4)0; //output color 
	
	float4 mDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
	float4 mSpecular = g_Specular.Sample(samAnisotropic, Input.Tex);
	float4 mGlow = g_Glow.Sample(samAnisotropic, Input.Tex);	
	float3 nT;
	nT.xy = g_Normal.Sample(samAnisotropic, Input.Tex).xy*2-1;
	nT.z = sqrt(saturate(1 - nT.x*nT.x - nT.y*nT.y));
	float4 cLight = float4(1,1,1,1);
	float4 cLightAmbient = float4(1,1,1,1);
	float3 N = normalize(Input.NorView);
	float3 T = normalize(Input.TanView - N * dot(N,Input.TanView));
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
	uint2 dimensions;
	g_ShadowMap.GetDimensions(dimensions.x,dimensions.y);
	float2 t = frac( lPos.xy * dimensions.x );
	float shadowFactor = lerp(lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,lPos.xy,lPos.z-0.01f),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y),lPos.z-0.01f),t.x),
						lerp(g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x+1.f/dimensions.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),g_ShadowMap.SampleCmpLevelZero(depthMap,float2(lPos.x,lPos.y+1.f/dimensions.y),lPos.z-0.01f),t.x),t.y);

	output = (0.5 * mDiffuse * saturate(dot(n,l)) * cLight + 0.7 * mSpecular * pow(saturate(dot(r,v)),1) * cLight + 0.5 * mGlow + 0.2 * mDiffuse * cLightAmbient) * shadowFactor + 0.05 * mDiffuse * cLightAmbient * (1.f-shadowFactor);
	return output; 
}

float4 SkyboxPS(in float4 Pos : SV_POSITION, in float2 Tex : TEXCOORD) : SV_Target0
{
	float3 res = normalize((g_TopLeft + Tex.x * g_Right + Tex.y * g_Down) - g_Eye);
	return g_Skybox.Sample(samAnisotropic,res);
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
	pass P1_Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
		
		SetRasterizerState(rsCullBack);
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
	pass P1_Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MeshPS()));
		
		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass P2_Skybox
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, SkyboxPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
