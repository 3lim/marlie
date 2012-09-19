Texture2DArray g_Textures[5];

cbuffer cbChangesEveryFrame
{
	matrix g_ViewProjection;
	float3 g_CamUp;
	float3 g_CamRight;
};

struct SpriteVertex
{
	float3 pos : POSITION;
	float radius : RADIUS;
	uint texindex : TEXTUREINDEX;
	uint animationSize : ASIZE;
	float animationProgress : APROGRESS;
	float opacity : OPACITY;
};

struct PSVertex
{
	float4 pos : SV_Position;
	uint texindex : TEXTUREINDEX;
	float2 texcoord : TEXCOORD;
	uint animationSize : ASIZE;
	float animationProgress : APROGRESS;
	float opacity : OPACITY;
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

BlendState BSBlendOver 
{ 
	BlendEnable[0] = TRUE;
	SrcBlend[0] = SRC_ALPHA;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = INV_SRC_ALPHA;
	DestBlendAlpha[0] = INV_SRC_ALPHA;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

SpriteVertex SpriteVS(SpriteVertex Input) 
{
	return Input;
}

float4 SpritePS(PSVertex Input) : SV_Target0 
{
	uint3 s1;
	g_Textures[0].GetDimensions(s1.x,s1.y,s1.z);
	uint3 s2;
	g_Textures[1].GetDimensions(s2.x,s2.y,s2.z);
	uint3 s3;
	g_Textures[2].GetDimensions(s3.x,s3.y,s3.z);
	uint3 s4;
	g_Textures[3].GetDimensions(s4.x,s4.y,s4.z);
	
	if(Input.texindex<s1.z)
	{
		return g_Textures[0].Sample(samAnisotropic,float3(Input.texcoord.x,Input.texcoord.y,Input.texindex + ceil(Input.animationProgress * (float)Input.animationSize))) * Input.opacity;
	}
	else if(Input.texindex<s1.z+s2.z)
	{
		return g_Textures[1].Sample(samAnisotropic,float3(Input.texcoord.x,Input.texcoord.y,Input.texindex - s1.z + ceil(Input.animationProgress * (float)Input.animationSize))) * Input.opacity;
	}
	else if(Input.texindex<s1.z+s2.z+s3.z)
	{
		return g_Textures[2].Sample(samAnisotropic,float3(Input.texcoord.x,Input.texcoord.y,Input.texindex - (s1.z+s2.z) + ceil(Input.animationProgress * (float)Input.animationSize))) * Input.opacity;
	}
	else if(Input.texindex<s1.z+s2.z+s3.z+s4.z)
	{
		return g_Textures[3].Sample(samAnisotropic,float3(Input.texcoord.x,Input.texcoord.y,Input.texindex - (s1.z+s2.z+s3.z) + ceil(Input.animationProgress * (float)Input.animationSize))) * Input.opacity;
	}
	else
	{
		return g_Textures[4].Sample(samAnisotropic,float3(Input.texcoord.x,Input.texcoord.y,Input.texindex - (s1.z+s2.z+s3.z+s4.z) + ceil(Input.animationProgress * (float)Input.animationSize))) * Input.opacity;
	}
}

[maxvertexcount(4)]
void SpriteGS( point SpriteVertex vertex[1], inout TriangleStream<PSVertex> Stream )
{
	PSVertex output;
	SpriteVertex input = vertex[0];
	output.texindex = input.texindex;
	output.animationSize = input.animationSize;
	output.animationProgress = input.animationProgress;
	output.opacity = input.opacity;

	output.pos = mul(float4(input.pos-(g_CamRight+g_CamUp)*input.radius,1),g_ViewProjection);
	output.texcoord = float2(0,1);
	Stream.Append(output);
	output.pos = mul(float4(input.pos-(g_CamRight-g_CamUp)*input.radius,1),g_ViewProjection);
	output.texcoord = float2(0,0);
	Stream.Append(output);
	output.pos = mul(float4(input.pos+(g_CamRight-g_CamUp)*input.radius,1),g_ViewProjection);
	output.texcoord = float2(1,1);
	Stream.Append(output);
	output.pos = mul(float4(input.pos+(g_CamRight+g_CamUp)*input.radius,1),g_ViewProjection);
	output.texcoord = float2(1,0);
	Stream.Append(output);
}

technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
