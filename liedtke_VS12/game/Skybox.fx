Texture2D g_Sun;
TextureCube SkyCubeImage;
Texture2D CloudTex1;
Texture2D CloudTex2;
float cloudBrightness = 1.8;
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
	float2 cloudTranslation[3];
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
	DestBlendAlpha[0] = ONE;
	BlendOp[0] = ADD;
	BlendOpAlpha[0] = ADD;

	BlendEnable[1] = TRUE;
	SrcBlend[1] = SRC_ALPHA;
	SrcBlendAlpha[1] = ONE;
	DestBlend[1] = INV_SRC_ALPHA;
	DestBlendAlpha[1] = INV_SRC_ALPHA;
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

float4 SkyboxPS(in QuadVertex input/*, out float4 finalColor : SV_TARGET0*/, out float4  vlsColor : SV_TARGET1) : SV_TARGET0
{
	float3 pixPosition = normalize((g_TopLeft + input.Tex.x * g_Right + input.Tex.y * g_Down) - g_Eye);
	float height = pixPosition.y;
	height*=2;
	height = abs(height);
	float4 finalColor;
	//float4 SkyBackColor = lerp(horizontColor, apexColor, height);
	//float4 Cloud1 = CloudTex1.Sample(samAnisotropic, pixPosition);
	//float4 Cloud2 = CloudTex2.Sample(samAnisotropic, pixPosition.xy+cloudTranslation[1])*0;
	//return float4(SkyBackColor.rgb+Cloud2.rgb*Cloud2.a*cloudBrightness+Cloud1.rgb*Cloud1.a*cloudBrightness,1);
	//return SkyCubeImage.Sample(samAnisotropic, pixPosition)+(SkyBackColor.a - 0.3);
	finalColor = lerp(horizontColor, apexColor, height);
	vlsColor = finalColor*0.02;
	return finalColor;
}

float4 SunPS(SunVertex Input, out float4 vlsColor : SV_TARGET1) : SV_Target0 
{
	float dist = length (Input.tex - float2 (0.5f, 0.5f)); //get the distance form the center of the point-sprite
	float alpha = saturate(sign (0.5f - dist*SUNSIZEFACTOR));
	float4 glow = SunColor;
		glow.a = saturate((0.5f - dist))*saturate((0.5f - dist)*0.36);
	vlsColor = float4(SunColor.rgb*1.1,alpha)+glow;
	return vlsColor;//wird vom Light Scattering übernommen
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


void CloudPS(QuadVertex input, out float4 finalColor : SV_TARGET0, out float4 vlsColor : SV_TARGET1)
{
	float4 textureColor1;
	float4 textureColor2;
    textureColor1 = CloudTex1.Sample(samAnisotropic, input.Tex + cloudTranslation[0]);
    textureColor2 = CloudTex2.Sample(samAnisotropic, input.Tex + cloudTranslation[1]);
	finalColor = lerp(textureColor1, textureColor2, 0.5f);
	finalColor.a *=CloudTex1.Sample(samAnisotropic, (float2(1,1) - input.Tex)*0.7 + cloudTranslation[2]).a*cloudBrightness+0.4;
	finalColor.rgb = 1;
	////SquareFade
	//float fadeX = input.Plane.x*2-1;//;
	//fadeX = fadeX*fadeX;
	//fadeX = 1-fadeX;
	//float fadeY = ((input.Plane.y*2-1));//*(input.Plane.y*2-1));//;
	//fadeY = fadeY*fadeY;
	//fadeY = 1-fadeY;
	//float fade = saturate(1-fadeX*fadeY*10);
	//CirccleFade
	float fade = saturate(length(float2(input.Plane.x*2-1, input.Plane.y*2-1)*0.7));
	finalColor = lerp(finalColor, float4(0,0,0,0), fade);//Fade out horizont
	vlsColor = float4(0,0,0,clamp(finalColor.a*2,0,0.96));
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
	pass sun
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, SunGS()));
		SetPixelShader(CompileShader(ps_4_0, SunPS()));
		
		SetRasterizerState(rsCullNone);
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
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
}