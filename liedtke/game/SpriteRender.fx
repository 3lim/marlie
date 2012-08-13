//Shader resources

Texture2DArray g_SpriteDiffuseArray[15];

//Constant buffers

cbuffer cbConstant{
};

cbuffer cbChangesEveryFrame
{
	matrix g_ViewProjection;
	float3 g_CameraUp;
	float3 g_CameraRight;
};

//Structs
struct SpriteVertex{
	float4 Pos : POSITION;
	float Radius : RADIUS;
	int TexIndex : TEXTUREINDEX;
	float t		: ANIMATIONTIME;
	float alpha	: ALPHABLENDING;
	float4 Color : BLENDINGCOLOR;
};

struct PSVertex{
	float4 Pos : SV_POSITION;
	float3 TexPos : TEXCOORD;
	int TexIndex : TEXTUREINDEX;
	float t		: ANIMATIONTIME;
	float alpha	: ALPHABLENDING;
	float4 Color : BLENDINGCOLOR;
};

//Sampler
SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

//State
BlendState BSBlendOver 
{ 
    BlendEnable[0]    = TRUE; 
    SrcBlend[0]       = SRC_ALPHA; 
    SrcBlendAlpha[0]  = ONE; 
    DestBlend[0]      = INV_SRC_ALPHA; 
    DestBlendAlpha[0] = INV_SRC_ALPHA; 
}; 

//Shaders
SpriteVertex SpriteVS(SpriteVertex input) { 
	//output.Pos = mul(input.Pos, g_ViewProjection);
	return input;
} 
float4 SpritePS(PSVertex input /*: SV_Target*/) : SV_Target0 { 
	//return float4(input.TexPos, 1);
	float3 dims;
	float4 color;
	switch(input.TexIndex)
	{
		case 0:
			g_SpriteDiffuseArray[0].GetDimensions(dims.x, dims.y, dims.z);
			color = g_SpriteDiffuseArray[0].Sample(samAnisotropic, float3(input.TexPos.xy, dims.z*input.t));
		break;
		case 1:
			g_SpriteDiffuseArray[1].GetDimensions(dims.x, dims.y, dims.z);
			color = g_SpriteDiffuseArray[1].Sample(samAnisotropic, float3(input.TexPos.xy, dims.z*input.t));
		break;
		case 2://Animated
			g_SpriteDiffuseArray[2].GetDimensions(dims.x, dims.y, dims.z);
			color = g_SpriteDiffuseArray[2].Sample(samAnisotropic, float3(input.TexPos.xy, int(dims.z*input.t)));
		break;
		case 3://Animated
			g_SpriteDiffuseArray[3].GetDimensions(dims.x, dims.y, dims.z);
			color = g_SpriteDiffuseArray[3].Sample(samAnisotropic, float3(input.TexPos.xy, int(dims.z*input.t)));
		break;
		case 4://Animated
			g_SpriteDiffuseArray[4].GetDimensions(dims.x, dims.y, dims.z);
			color= g_SpriteDiffuseArray[4].Sample(samAnisotropic, float3(input.TexPos.xy, int(dims.z*input.t)));
		break;
		case 5:
			g_SpriteDiffuseArray[5].GetDimensions(dims.x, dims.y, dims.z);
			color = g_SpriteDiffuseArray[5].Sample(samAnisotropic, float3(input.TexPos.xy, dims.z*input.t));
		break;
	default:
			return float4(1,1,1,0);
	}
	color.rgb += input.Color.rgb*input.Color.a;
	return float4(color.rgb, color.w*input.alpha);
}	

[maxvertexcount(4)]
void SpriteGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream){
	SpriteVertex input = vertex[0];
	PSVertex v;
	v.Color = input.Color;
	v.TexIndex = input.TexIndex;
	v.TexPos.z = input.TexIndex;
	v.t = input.t;
	v.alpha = input.alpha;
	float3 offsetX = input.Radius*g_CameraRight;
	float3 offsetY = input.Radius*g_CameraUp;
	v.Pos = float4(input.Pos.xyz+offsetX+offsetY,1);
	v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(0,0);
	stream.Append(v);
	v.Pos = float4(input.Pos.xyz+offsetX-offsetY,1);
	v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(0,1);
	stream.Append(v);
	v.Pos = float4(input.Pos.xyz-offsetX+offsetY,1);
	v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(1,0);
	stream.Append(v);
	v.Pos = float4(input.Pos.xyz-offsetX-offsetY,1);
	v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(1,1);
	stream.Append(v);
	stream.RestartStrip();
	
}

[maxvertexcount(4)]
void GuiGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream){
	SpriteVertex input = vertex[0];
	PSVertex v;
	v.Color = input.Color;
	v.TexIndex = input.TexIndex;
	v.TexPos.z = input.TexIndex;
	v.t = input.t;
	v.alpha = input.alpha;
	float3 offsetX = float3(input.Radius,0,0)*9/16;
	float3 offsetY = float3(0,input.Radius,0);
	v.Pos = float4(input.Pos.xy+offsetX+offsetY,0,1);
	//v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(0,0);
	stream.Append(v);
	v.Pos = float4(input.Pos.xy+offsetX-offsetY,0,1);
	//v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(0,1);
	stream.Append(v);
	v.Pos = float4(input.Pos.xy-offsetX+offsetY,0,1);
	//v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(1,0);
	stream.Append(v);
	v.Pos = float4(input.Pos.xy-offsetX-offsetY,0,1);
	//v.Pos = mul(v.Pos, g_ViewProjection);
	v.TexPos.xy = float2(1,1);
	stream.Append(v);
	stream.RestartStrip();
	
}

//Rasterizer States
RasterizerState rsCullNone {
	CullMode = None; 
};
RasterizerState rsCullBack {
	CullMode = Back; 
};

//DepthStates
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
//Techniques
technique11 Render
{
	pass P0_Sprite
	{
		SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
		SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
		SetPixelShader(CompileShader(ps_4_0, SpritePS()));

		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass P1_SpriteGUI
	{
		SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
		SetGeometryShader(CompileShader(gs_4_0, GuiGS()));
		SetPixelShader(CompileShader(ps_4_0, SpritePS()));

		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}