Texture2D aux1Buffer;
Texture2D aux2Buffer;
Texture2D source;
float4 g_LightColor;
float3 g_LightPosition;
matrix g_WorldViewProj;
float g_density = 0.3;
int NUM_SAMPLES = 80;
float g_decay = 0.8;
float g_weight = 1;
float g_exposure = 1.4;
int BLURSAMPLES = 16;
struct QuadVertex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	BorderColor = float4(0,0,0,1);
};


BlendState AdditiveBlending
{ 
	BlendEnable[0] = TRUE;
	SrcBlend[0] = ONE;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = ONE;
	DestBlendAlpha[0] = ONE;
	BlendOp = Add;
	BlendOpAlpha = Add;
};
BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};
RasterizerState rsCullNone {
	CullMode = None; 
};
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};


//FUNCTION
half4 Blur(float2 texCoord, int range ,texture2D tex) {
    float4 c = tex.Sample(samLinear, texCoord);
	float2  dimensions;
	tex.GetDimensions(dimensions.x, dimensions.y);
	float2 d = 1/dimensions;
    // this loop will be unrolled by compiler and the constants precalculated:
    for(int i=0; i<range; i++) {
		float2 offset = -d;
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(0,-d.y);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(d.x,-d.y);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(d.x,0);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(d.x,d.y);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(0,d.y);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(-d.x,d.y);
    	c += tex.Sample(samLinear, texCoord +offset);
		offset = float2(-d.x,0);
    	c += tex.Sample(samLinear, texCoord +offset);
    }
    c /= range*9;
    return c;
} 



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
float4 LightScatteringPS(QuadVertex v) : SV_TARGET0
{
	float4 lightScreenPos = mul(float4(g_LightPosition,0), g_WorldViewProj);
	lightScreenPos.xy /= lightScreenPos.w;
	lightScreenPos.x += 0.45f;//BUT WHY?

	float2 deltaTexCoord = v.Tex-lightScreenPos.xy;
		deltaTexCoord  *= 1.f/NUM_SAMPLES*g_density;
	float illuminationDecay = 1.0f;
	float3 result = aux1Buffer.Sample(samLinear, v.Tex);
		for(int i = 0; i < NUM_SAMPLES; i++)
		{
			v.Tex -= deltaTexCoord;
			float4 sample =  aux1Buffer.Sample(samLinear, v.Tex);
				sample *= illuminationDecay*g_weight;
			result += sample;
			illuminationDecay *= g_decay;
		}
	return float4(result * g_exposure,result.r * g_exposure);
}

float4 AdditiveBlendPS(QuadVertex v) : SV_TARGET0
{
	float4 color = Blur(v.Tex,1, aux2Buffer);//.Sample(samLinear, v.Tex);
		return float4(color.rgb, 1);
}

technique11 VolumetricLightScattering
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
        SetPixelShader(CompileShader(ps_4_0, LightScatteringPS()));
        
		SetRasterizerState(rsCullNone);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
        SetPixelShader(CompileShader(ps_4_0, AdditiveBlendPS()));
        
		SetRasterizerState(rsCullNone);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}