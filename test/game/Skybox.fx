Texture2D g_Sun;
TextureCube SkyCubeImage;
Texture2D CloudTex1;
Texture2D CloudTex2;
float cloudBrightness = 0.85;
int SUNSIZEFACTOR = 6;

cbuffer cbChangesEveryFrame
{
	matrix g_ViewProj;
	float4 SunColor;
	float3 g_SunPos;
	float  g_SunRadius;
	float3 g_TopLeft;
	float3 g_Down;
	float3 g_Right;
	float3 g_Eye;
	float4 horizontColor;
	float4 apexColor;
	float2 cloudTranslation[2];
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

struct SunVertex
{
	float4 pos : SV_Position;
	float4 center : CENTERPOS;
	float2 tex : TEXCOORD;
};



struct QuadVertex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float2 Plane : PLANECOORD;
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = MIRROR;
	AddressV = MIRROR;
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
DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};


//--------------------------------------------------------------------------------------
// BlendStates
//--------------------------------------------------------------------------------------
BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};
BlendState EnableBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend[0] = SRC_ALPHA;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = INV_SRC_ALPHA;
	DestBlendAlpha[0] = INV_SRC_ALPHA;
};
BlendState SubtractBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend[0] = ONE;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = ONE;
	DestBlendAlpha[0] = ONE;
	BlendOp = ADD;
	BlendOpAlpha = ADD;
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
	qv.Plane = float2(0,0);
	stream.Append(qv);
	qv.Pos = float4(-1,-1, 1, 1);
	qv.Tex = float2(0,1);
	qv.Plane = float2(0,1);
	stream.Append(qv);
	qv.Pos = float4(1,1,1,1);
	qv.Tex = float2(1,0);
	qv.Plane = float2(1,0);
	stream.Append(qv);
	qv.Pos = float4(1,-1,1,1);
	qv.Tex = float2(1,1);
	qv.Plane = float2(1,1);
	stream.Append(qv);
	stream.RestartStrip();

}

[maxvertexcount(4)]
void SunGS( point uint vertex[1] : POINT, inout TriangleStream<SunVertex> Stream )
{
	SunVertex output;
	output.center = mul(float4(g_SunPos,1),g_ViewProj);
	output.pos =  mul(float4(g_SunPos-(g_CamRight+g_CamUp)*g_SunRadius*SUNSIZEFACTOR,1),g_ViewProj);
	output.tex = float2(0,0);
	Stream.Append(output);
	output.pos =  mul(float4(g_SunPos-(g_CamRight-g_CamUp)*g_SunRadius*SUNSIZEFACTOR,1),g_ViewProj);
	output.tex = float2(0,1);
	Stream.Append(output);
	output.pos =  mul(float4(g_SunPos+(g_CamRight-g_CamUp)*g_SunRadius*SUNSIZEFACTOR,1),g_ViewProj);
	output.tex = float2(1,0);
	Stream.Append(output);
	output.pos =  mul(float4(g_SunPos+(g_CamRight+g_CamUp)*g_SunRadius*SUNSIZEFACTOR,1),g_ViewProj);
	output.tex = float2(1,1);
	Stream.Append(output);
}

float4 SkyboxPS(in QuadVertex input) : SV_Target0
{
	float3 pixPosition = normalize((g_TopLeft + input.Tex.x * g_Right + input.Tex.y * g_Down) - g_Eye);
	float height = pixPosition.y;
	height*=2;
	height = abs(height);
	//float4 SkyBackColor = lerp(horizontColor, apexColor, height);
	//float4 Cloud1 = CloudTex1.Sample(samAnisotropic, pixPosition);
	//float4 Cloud2 = CloudTex2.Sample(samAnisotropic, pixPosition.xy+cloudTranslation[1])*0;
	//return float4(SkyBackColor.rgb+Cloud2.rgb*Cloud2.a*cloudBrightness+Cloud1.rgb*Cloud1.a*cloudBrightness,1);
	//return SkyCubeImage.Sample(samAnisotropic, pixPosition)+(SkyBackColor.a - 0.3);
	return lerp(horizontColor, apexColor, height);
}
float4 SkyboxVLSPS(in QuadVertex input) : SV_Target0
{
	float3 pixPosition = normalize((g_TopLeft + input.Tex.x * g_Right + input.Tex.y * g_Down) - g_Eye);
	float height = pixPosition.y;
	height*=2;
	height = abs(height);
	//float4 SkyBackColor = lerp(horizontColor, apexColor, height);
	//float4 Cloud1 = CloudTex1.Sample(samAnisotropic, pixPosition);
	//float4 Cloud2 = CloudTex2.Sample(samAnisotropic, pixPosition.xy+cloudTranslation[1])*0;
	//return float4(SkyBackColor.rgb+Cloud2.rgb*Cloud2.a*cloudBrightness+Cloud1.rgb*Cloud1.a*cloudBrightness,1);
	//return SkyCubeImage.Sample(samAnisotropic, pixPosition)+(SkyBackColor.a - 0.3);
	return lerp(horizontColor, apexColor, height)*0.02;
}

float4 SunPS(SunVertex Input) : SV_Target0 
{
	float dist = length (Input.tex - float2 (0.5f, 0.5f)); //get the distance form the center of the point-sprite
	float alpha = saturate(sign (0.5f - dist*SUNSIZEFACTOR));
	return float4(SunColor.rgb,alpha);
}

float4 SunBWPS(SunVertex Input) : SV_Target0 
{
	float dist = length (Input.tex - float2 (0.5f, 0.5f)); //get the distance form the center of the point-sprite
	float alpha = saturate(sign (0.5f - dist*SUNSIZEFACTOR));
	float4 glow = SunColor;
		glow.a = saturate((0.5f - dist))*saturate((0.5f - dist)*0.36);
	return float4(SunColor.rgb*1.1,alpha)+glow;
}

QuadVertex SkydomeVS(QuadVertex input)
{
	QuadVertex output;
	output.Pos = mul(input.Pos,g_ViewProj);
	output.Plane = input.Plane;
	output.Tex = input.Tex;
	return output;
}

[maxvertexcount(1)]
void SkydomeGS(point QuadVertex vertex[1], inout TriangleStream<QuadVertex> stream)
{
	QuadVertex p =vertex[0];
	stream.Append(p);
}


float4 CloudPS(QuadVertex input) : SV_Target0
{
	float4 textureColor1;
	float4 textureColor2;
	float4 finalColor;
    textureColor1 = CloudTex1.Sample(samAnisotropic, input.Tex + cloudTranslation[0]);
    textureColor2 = CloudTex2.Sample(samAnisotropic, input.Tex + cloudTranslation[1]);
	finalColor = lerp(textureColor1, textureColor2, 0.5f);
	finalColor = finalColor * cloudBrightness;
	float fadeX = input.Plane.x*2-1;//;
	fadeX = fadeX*fadeX;
	fadeX = 1-fadeX;
	float fadeY = ((input.Plane.y*2-1));//*(input.Plane.y*2-1));//;
	fadeY = fadeY*fadeY;
	fadeY = 1-fadeY;
	float fade = saturate(1-fadeX*fadeY*10);
	//return float4(fade,fade,fade,1);
	return lerp(finalColor, float4(0,0,0,0), fade);//Fade out horizont
}
float4 CloudBWPS(QuadVertex input) : SV_Target0
{
	float4 textureColor1;
	float4 textureColor2;
	float4 finalColor;
    textureColor1 = CloudTex1.Sample(samAnisotropic, input.Tex + cloudTranslation[0]);
    textureColor2 = CloudTex2.Sample(samAnisotropic, input.Tex + cloudTranslation[1]);
	finalColor = lerp(textureColor1, textureColor2, 0.5f);
	finalColor = finalColor * cloudBrightness;
	float fadeX = input.Plane.x*2-1;//;
	fadeX = fadeX*fadeX;
	fadeX = 1-fadeX;
	float fadeY = ((input.Plane.y*2-1));//*(input.Plane.y*2-1));//;
	fadeY = fadeY*fadeY;
	fadeY = 1-fadeY;
	float fade = saturate(1-fadeX*fadeY*10);
	//return float4(fade,fade,fade,1);
	float4 color = lerp(finalColor, float4(0,0,0,0), fade);
	//float4 color = CloudTex2.Sample(samAnisotropic, input.Tex + cloudTranslation[0])*cloudBrightness;
	//return float4(0,1,1,1);
	return float4(0,0,0,clamp(color.a*2,0,0.96));//Fade out horizont
}

technique11 tSkybox
{
	pass sun
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, SunGS()));
		SetPixelShader(CompileShader(ps_4_0, SunPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass sunBW
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, SunGS()));
		SetPixelShader(CompileShader(ps_4_0, SunBWPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass drawSkyCube
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
		SetPixelShader(CompileShader(ps_4_0, SkyboxPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass drawSkyCubeVLS
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
		SetPixelShader(CompileShader(ps_4_0, SkyboxVLSPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}

	pass Clouds
	{
		SetVertexShader(CompileShader(vs_4_0, SkydomeVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CloudPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass CloudsBW
	{
		SetVertexShader(CompileShader(vs_4_0, SkydomeVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CloudBWPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}