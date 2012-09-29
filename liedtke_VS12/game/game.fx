#include "Util.fx"
//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

float cA = 0.1; //Ambient beleuchtung
Texture2D   g_Diffuse; // Material albedo for diffuse lighting
Buffer<float> g_Height;
Texture2D g_Normal;
Texture2D g_Specular;
Texture2D g_Glow;
TextureCube g_SkyBoxTex;
Texture2D g_ShadowMap;
Texture2D g_ShadowMapVSM;
Texture2DMS<float4,4> g_VLSMap;
//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	float4 g_LightColor;
	int g_TerrainRes;
	int g_TerrainQuadRes;
};

cbuffer cbChangesEveryFrame
{
	matrix	g_World; //für jedes Object anders
    float4  g_LightDir; // Object space
    matrix  g_WorldViewProjection;
    float   g_Time;
	matrix g_Proj;
	matrix g_WorldView;
	matrix g_WorldViewNormals;
	float4 g_LightDirView;
	matrix g_LightProjectionMatrix;
	matrix g_LightViewMatrix;
	matrix g_LightWorldViewProjMatrix;
};

cbuffer cbUserChanges
{
	bool objectOfCamera;
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
    float2 Tex : TEXCOORD;
	float4 LightPos : TEXCOORD1;
};

struct T3dVertexVSIn {
	float3 Pos : POSITION; //in object space
	float2 Tex : TEXCOORD; //Texture Coordinate
	float3 Nor : NORMAL; // in object Space
	float3 Tan : TANGENT; //in object Space
};

struct T3dVertexPSIn {
	float4 Pos : SV_Position; //in clip space
	float2 Tex : TEXCOORD;
	float3 PosView : POSITION; //in viewSpace
	float3 NorView : NORMAL; //Normal in view Space
	float3 TanView : TANGENT; //in ViewSpace
	float4 LightPos : TEXCOORD1;
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
SamplerState samEnvironment
{
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
SamplerState bilinearSampler
{
    Filter = min_mag_mip_linear;
    AddressU = MIRROR;
    AddressV = MIRROR;	
};
SamplerState pointSampler
{
    Filter = min_mag_mip_point;
    AddressU = MIRROR;
    AddressV = MIRROR;	
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


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------


T3dVertexPSIn ShadowMeshVS(T3dVertexVSIn input) {
	T3dVertexPSIn output = (T3dVertexPSIn) 0;
	output.Pos = mul(float4(input.Pos,1), mul(g_World, g_LightWorldViewProjMatrix));
	return output;
}

PosTex ShadowTerrainVS(uint VertexID : SV_VertexID) {
	PosTex output = (PosTex)0;
	uint quadIdx = VertexID/6;
	uint inQuadIdx = VertexID%6;
	int2 coords;
	coords.x = quadIdx%(g_TerrainRes-1); 
	coords.y = quadIdx/(g_TerrainRes-1);
	if(inQuadIdx == 1)
		coords.x++;
	if(inQuadIdx == 2)
		coords.y++;
	if(inQuadIdx == 3)
		coords.y++;
	if(inQuadIdx ==4)
		coords.x++;
	if(inQuadIdx == 5){
		coords.y++;
		coords.x++;
	}
	output.Tex.x = (float)coords.x/(g_TerrainRes-1);
	output.Tex.y = (float)coords.y/(g_TerrainRes-1);
	float height = g_Height[coords.x + coords.y*(g_TerrainRes)];
	output.Pos.x = output.Tex.x;
	output.Pos.y = output.Tex.y;
	output.Pos.z = height;
	output.Pos.w = 1;
	output.Pos = mul(output.Pos, mul(g_World, g_LightWorldViewProjMatrix));
	return output;
}


PosTex TerrainVS(uint VertexID : SV_VertexID) {
	PosTex output = (PosTex)0;
	uint quadIdx = VertexID/6;
	uint inQuadIdx = VertexID%6;
	int2 coords;
	coords.x = quadIdx%(g_TerrainRes-1); 
	coords.y = quadIdx/(g_TerrainRes-1);
	if(inQuadIdx == 1)
		coords.x++;
	if(inQuadIdx == 2)
		coords.y++;
	if(inQuadIdx == 3)
		coords.y++;
	if(inQuadIdx ==4)
		coords.x++;
	if(inQuadIdx == 5){
		coords.y++;
		coords.x++;
	}
	output.Tex.x = (float)coords.x/(g_TerrainRes-1);
	output.Tex.y = (float)coords.y/(g_TerrainRes-1);
	float height = g_Height[coords.x + coords.y*(g_TerrainRes)];
	output.Pos.x = output.Tex.x;
	output.Pos.y = output.Tex.y;
	output.Pos.z = height;
	output.Pos.w = 1;
	output.LightPos =mul(output.Pos, mul(g_World, g_LightWorldViewProjMatrix));
	output.Pos = mul(output.Pos, mul(g_World, g_WorldViewProjection));
	return output;
}

float4 TerrainPS(PosTex Input) : SV_Target0 {
	int isLighted = 1;
	Input.LightPos.xyz /= Input.LightPos.w;
	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
	//if(Input.LightPos.x < -1.0f || Input.LightPos.x > 1.0f ||
	//   Input.LightPos.y < -1.0f || Input.LightPos.y > 1.0f ||
	//   Input.LightPos.z < -1.0f || Input.LightPos.z > 1.0f)
	//   return float4(cA*g_LightColor.rgb*matDiffuse.rgb,0);
	Input.LightPos.x = Input.LightPos.x/2 +0.5 ;
	Input.LightPos.y = Input.LightPos.y/2+ 0.5;
	Input.LightPos.y *= -1;
	Input.LightPos.z -= 0.001;
	float shadowDepth = g_ShadowMap.Sample(samAnisotropic, Input.LightPos.xy).r;
	float3 n = g_Normal.Sample(samAnisotropic, Input.Tex).rgb;
	n.xy = n.xy*2-1;
	n.z = sqrt(1- n.x*n.x-n.y*n.y);
	float i = saturate(dot(n, -g_LightDir.xyz));
	if(Input.LightPos.z > shadowDepth) //wenn im Schatten
	  return float4(cA/2*g_LightColor.rgb*matDiffuse.rgb, 1);
	return float4(i*matDiffuse.rgb*g_LightColor.rgb*(1-cA)
		//g_LightDirView.xyz
		//n.xyz
		//matDiffuse.rgb*g_LightColor*(1-cA)
		+ cA*g_LightColor.rgb*matDiffuse.rgb //Ambient Shading
		, 1);

}

T3dVertexPSIn MeshVS(T3dVertexVSIn Input) {
	T3dVertexPSIn output = (T3dVertexPSIn) 0;
	//
	if(objectOfCamera)
	output.Pos = mul(float4(Input.Pos,1), mul(g_World, g_Proj));
	else
	output.Pos = mul(float4(Input.Pos,1), mul(g_World, g_WorldViewProjection));
	output.Tex = Input.Tex;
	float4 posView = mul(float4(Input.Pos,1), mul(g_World, g_WorldView));
	output.PosView = posView.xyz;
	output.NorView.xyz = normalize(mul(float4(Input.Nor, 0), mul(g_World, g_WorldViewNormals)).xyz);
	output.TanView = normalize(mul(float4(Input.Tan, 0), mul(g_World, g_WorldViewNormals)).xyz);
	output.LightPos = mul(float4(Input.Pos,1), mul(g_World, g_LightWorldViewProjMatrix));
	return output;
}

float4 MeshPS(T3dVertexPSIn Input) : SV_Target0 {
	float cD = 0.7, cS = 0.3, cG = 0.6;
	int isLighted = 1;
	Input.LightPos.xyz /= Input.LightPos.w;
	//if(Input.LightPos.x < -1.0f || Input.LightPos.x > 1.0f ||
	//   Input.LightPos.y < -1.0f || Input.LightPos.y > 1.0f ||
	//   Input.LightPos.z < -1.0f || Input.LightPos.z > 1.0f)
	//   return float4(1,0,0,0);
	Input.LightPos.x = Input.LightPos.x/2 + 0.5;
	Input.LightPos.y = Input.LightPos.y/-2+0.5;
	Input.LightPos.z -= 0.001;
	float shadowDepth = g_ShadowMap.Sample(samAnisotropic, Input.LightPos.xy).r;
	float4 spec = g_Specular.Sample(samAnisotropic, Input.Tex);
	float4 glow = g_Glow.Sample(samAnisotropic, Input.Tex);
	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
	float3 n = normalize(Input.NorView);
	float3 t = Input.TanView;
	t = normalize(t-n*dot(n,t));
	float3 b = +cross(t,n);
	float3 normal = g_Normal.Sample(samAnisotropic, Input.Tex).rgb;
	normal.xy = normal.xy*2-1;
	normal.z = sqrt(1- normal.x*normal.x-normal.y*normal.y);
	normal = normalize(normal);
	float3x3 tbn = float3x3( t.x, b.x, n.x,
					t.y, b.y, n.y,
					t.z, b.z, n.z);
	if(normal.x != 0 && normal.y != 0 && normal.z != 0)
		n = mul(tbn,normal.xyz);
	float4 colLight = float4(0.8,0.8,1,1);
	float4 colLightAmbient= float4(1,1,1,1);
	float4 l = -g_LightDirView;
	float3 r = reflect(-l.xyz, n);
	float3 v = normalize(Input.PosView);
	if(Input.LightPos.z > shadowDepth && objectOfCamera == false)
	  return cA/2*matDiffuse*g_LightColor;
	return cD*matDiffuse*saturate(dot(n,l.xyz))*g_LightColor
		+ cS*spec*pow(saturate(dot(r,v.xyz)),20)*g_LightColor
		+ cA*matDiffuse*g_LightColor
		+ cG*glow;
}



float4 VS_Billboard( void ) : SV_POSITION
{
	return float4(0,0,0,0);
}
[maxvertexcount(6)]
void GS_Billboard( point float4 s[1] : SV_POSITION, inout TriangleStream<PosTex> triStream )
{
	PosTex p;
	p.LightPos = float4(0,0,0,0);
	
	p.Pos = float4(-1,-1,0,1);
	p.Tex = float2(0,1);
	triStream.Append(p);
	
	p.Pos = float4(-1,-0.5,0,1);
	p.Tex = float2(0,0);
	triStream.Append(p);

	p.Pos = float4(-0.00,-1,0,1);
	p.Tex = float2(2,1);
	triStream.Append(p);

	p.Pos = float4(-0.00,-0.5,0,1);
	p.Tex = float2(2,0);
	triStream.Append(p);
}
float4 PS_Billboard( PosTex input ) : SV_TARGET
{		
	return g_ShadowMap.Sample(samAnisotropic, input.Tex);	
}
float4 SATPS_Billboard( PosTex input ) : SV_TARGET
{		

	float2 size = float2(2,2);
	float2 dim;
	g_ShadowMap.GetDimensions(dim.x, dim.y);
	size = size/dim;
	float4 SatCoord = float4(input.Tex.x-size.x,input.Tex.y-size.y,input.Tex.x+size.x,input.Tex.y+size.y);
	//float4 SatCoord = input.Tex.xyxy + float4(0, 0, size.xy);
	//return float4(sampleSAT(g_ShadowMap, SatCoord, dim).r,0,1);
	int texID = (input.Tex.x > 1.f);
	int samples=0;
	g_VLSMap.GetDimensions(dim.x, dim.y,samples);
	return float4( 
		//(1-texID)*(g_ShadowMap.Sample(samAnisotropic, float3(input.Tex,0)))+
		(1-texID)*(g_VLSMap.Load(int3(input.Tex.x * dim.x, input.Tex.y * dim.y, 0), 0))+
		//(1-texID)*(samSAT(g_ShadowMap, input.Tex, float2(2,2)).xxx+GetFPBias().xxx)+ //Displays SatImg als ursprüngliches Bild
		(g_ShadowMapVSM.Sample(samAnisotropic, float3(input.Tex,0)).xxx+GetFPBias().xxx)*texID,1);
}



//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
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

	
}
technique11 Shadow
{
    pass P0_ShadowTerrain
    {
        SetVertexShader(CompileShader(vs_4_0, ShadowTerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);
        
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
	pass P1_ShadowMesh
	{
        SetVertexShader(CompileShader(vs_4_0, ShadowMeshVS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);
        
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}

technique10 RenderBillboard
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Billboard() ) );
        SetGeometryShader( CompileShader( gs_4_0, GS_Billboard() ) );
        SetPixelShader( CompileShader( ps_4_0, PS_Billboard() ) );	
		SetRasterizerState( rsCullBack );		
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
	//sample SAT with size 1 to display
    pass P1
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Billboard() ) );
        SetGeometryShader( CompileShader( gs_4_0, GS_Billboard() ) );
        SetPixelShader( CompileShader( ps_4_0, SATPS_Billboard() ) );	
		SetRasterizerState( rsCullBack );		
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

