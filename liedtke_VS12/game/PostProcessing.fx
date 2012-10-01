#include "Filter.fx"

Texture2DMS<float4,4> aux1Buffer;
Texture2DMS<float4,4> aux2Buffer;
Texture2D source;

cbuffer cbCommon
{
float4 g_LightColor;
float3 g_LightPosition;
matrix g_WorldViewProj;
}
cbuffer cbVLS
{
float g_density = 0.1;
int NUM_SAMPLES = 80;
float g_decay = .9;
float g_weight = 1.5;
float g_exposure = 1.5;
}
cbuffer cbBlur
{
float2 g_BlurDimension;
int g_BlurSamples;
}
Texture2D blurImg;

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
SamplerState samBilinear
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = MIN_MAG_LINEAR_MIP_POINT;
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
	float3 dim;
	aux1Buffer.GetDimensions(dim.x, dim.y, dim.z);
	float4 lightScreenPos = mul(float4(g_LightPosition.xyz,0), g_WorldViewProj);
	lightScreenPos.xy /= lightScreenPos.w;
	lightScreenPos.x += 0.5f;
	lightScreenPos.y *= -1;//0.5f;

	float2 deltaTexCoord = v.Tex-lightScreenPos.xy;
		deltaTexCoord  *= 1.f/NUM_SAMPLES*g_density;
	float illuminationDecay = 1.0f;
	float3 result = aux1Buffer.Load(int3(v.Tex*dim.xy,0),0);
		for(int i = 0; i < NUM_SAMPLES; i++)
		{
			v.Tex -= deltaTexCoord;
			float4 sample =  aux1Buffer.Load(int3(v.Tex*dim.xy,0),0);
				sample *= illuminationDecay*g_weight;
			result += sample;
			illuminationDecay *= g_decay;
		}
	return float4(result * g_exposure,result.r * g_exposure);
}

float4 AdditiveBlendBlurPS(QuadVertex v) : SV_TARGET0
{
	float3 dim;
	aux1Buffer.GetDimensions(dim.x, dim.y, dim.z);
	float4 color = aux2Buffer.Load(int3(v.Tex*dim.xy,0),0); // aux2Buffer.Sample(samLinear, v.Tex);
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
        SetPixelShader(CompileShader(ps_4_0, AdditiveBlendBlurPS()));
        
		SetRasterizerState(rsCullNone);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}


// Water pixel shader
// Copyright (C) Wojciech Toman 2009
Texture2D heightMap;
Texture2DMS<float4,4> depthMap;
Texture2DMS<float4,4> screen;
Texture2D normalMap;
Texture2D foamMap;
Texture2D reflectionMap;

// We need this matrix to restore position in world space
float4x4 matViewInverse;
float4x4 matProjInverse;

float4x4 matViewProj;

float terrainDim;
// Level at which water surface begins
float waterLevel = 20.0f;

// Position of the camera
float3 cameraPos;

// How fast will colours fade out. You can also think about this
// values as how clear water is. Therefore use smaller values (eg. 0.05f)
// to have crystal clear water and bigger to achieve "muddy" water.
float fadeSpeed = 0.15f;

// Timer
float timer;

// Normals scaling factor
float normalScale = 0.005f;

// R0 is a constant related to the index of refraction (IOR).
// It should be computed on the CPU and passed to the shader.
float R0 = 0.5f;

// Maximum waves amplitude
float maxAmplitude = 20.0f;

// Direction of the light
float3 lightDir = {0.0f, 1.0f, 0.0f};

// Colour of the sun
float3 sunColor = {1.0f, 1.0f, 1.0f};

// The smaller this value is, the more soft the transition between
// shore and water. If you want hard edges use very big value.
// Default is 1.0f.
float shoreHardness = 1.0f;

// This value modifies current fresnel term. If you want to weaken
// reflections use bigger value. If you want to empasize them use
// value smaller then 0. Default is 0.0f.
float refractionStrength = 0.0f;
//float refractionStrength = -0.3f;

// Modifies 4 sampled normals. Increase first values to have more
// smaller "waves" or last to have more bigger "waves"
float4 normalModifier = {2.0f, 4.0f, 8.0f, 16.0f};

// Strength of displacement along normal.
float displace = 1.7f;

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The fird value is at
// what height foam for waves appear (+ waterLevel).
float3 foamExistence = {0.65f, 1.35f, 0.5f};

float sunScale = 3.0f;

float4x4 matReflection =
{
	{0.5f, 0.0f, 0.0f, 0.5f},
	{0.0f, 0.5f, 0.0f, 0.5f},
	{0.0f, 0.0f, 0.0f, 0.5f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

float shininess = 0.7f;
float specular_intensity = 0.32;

// Colour of the water surface
float3 depthColour = {0.0078f, 0.5176f, 0.7f};
// Colour of the water depth
float3 bigDepthColour = {0.0039f, 0.00196f, 0.145f};
float3 extinction = {7.0f, 30.0f, 40.0f};			// Horizontal

// Water transparency along eye vector.
float visibility = 4.0f;

// Increase this value to have more smaller waves.
float2 scale = {0.005f, 0.005f};
float refractionScale = 0.005f;

// Wind force in x and z axes.
float2 wind = {-0.3f, 0.7f};

float3 PositionFromDepth(float2 vTexCoord)
{
	uint3 dim;
	depthMap.GetDimensions(dim.x, dim.y,dim.z);
    float z = depthMap.Load(vTexCoord*dim.xy,0).r;  
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    float4 vPositionVS = mul(vProjectedPos, matProjInverse);  
    return vPositionVS.xyz / vPositionVS.w;  
}

float3x3 compute_tangent_frame(float3 N, float3 P, float2 UV)
{
	float3 dp1 = ddx(P);
	float3 dp2 = ddy(P);
	float2 duv1 = ddx(UV);
	float2 duv2 = ddy(UV);
	
	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float2x3 inverseM = float2x3( cross( M[1], M[2] ), cross( M[2], M[0] ) );
	float3 T = mul(float2(duv1.x, duv2.x), inverseM);
	float3 B = mul(float2(duv1.y, duv2.y), inverseM);
	
	return float3x3(normalize(T), normalize(B), N);
}

float fresnelTerm(float3 normal, float3 eyeVec)
{
		float angle = 1.0f - saturate(dot(normal, eyeVec));
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return saturate(fresnel * (1.0f - saturate(R0)) + R0 - refractionStrength);
}

float4 waterPS(QuadVertex IN): SV_Target0
{
	uint3 dims;
	screen.GetDimensions(dims.x,dims.y,dims.z);
	float3 color2 = screen.Load(IN.Tex*dims.xy,0).rgb;
	float3 color = color2;
	
	float3 position = mul(float4(PositionFromDepth(IN.Tex), 1.0f), matViewInverse).xyz;
	float level = waterLevel;
	float depth = 0.0f;
	
	// If we are underwater let's leave out complex computations
	if(level >= cameraPos.y)
		return float4(color2, 1.0f);

	if(position.x>terrainDim||position.x<-terrainDim||position.z>terrainDim||position.z<-terrainDim) return float4(color2, 1.0f);
	
	if(position.y <= level + maxAmplitude)
	{
		float3 eyeVec = position - cameraPos;
		float diff = level - position.y;
		float cameraDepth = cameraPos.y - position.y;
		
		// Find intersection with water surface
		float3 eyeVecNorm = normalize(eyeVec);
		float t = (level - cameraPos.y) / eyeVecNorm.y;
		float3 surfacePoint = cameraPos + eyeVecNorm * t;
	
		eyeVecNorm = normalize(eyeVecNorm);

		
		float2 texCoord;
		for(int i = 0; i < 10; ++i)
		{
			texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1f) * scale + timer * 0.000005f * wind;
			float bias = heightMap.SampleLevel(samLinear,texCoord*0.15f+float(0.5).xx,0);
			bias *= 0.1f;
			level += bias * maxAmplitude;
			t = (level - cameraPos.y) / eyeVecNorm.y;
			surfacePoint = cameraPos + eyeVecNorm * t;
		}
		
		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		
		eyeVecNorm = normalize(cameraPos - surfacePoint);
		
		
		float normal1 = heightMap.Sample(samLinear,texCoord*0.15f+float(0.5).xx + float2(-1, 0)/terrainDim);
		float normal2 = heightMap.Sample(samLinear,texCoord*0.15f+float(0.5).xx + float2(1, 0)/terrainDim);
		float normal3 = heightMap.Sample(samLinear,texCoord*0.15f+float(0.5).xx + float2(0, -1)/terrainDim);
		float normal4 = heightMap.Sample(samLinear,texCoord*0.15f+float(0.5).xx + float2(0, 1)/terrainDim);
		
		float3 myNormal = normalize(float3((normal1 - normal2) * maxAmplitude,
										   normalScale,
										   (normal3 - normal4) * maxAmplitude));   
		
		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		float3x3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal0a = normalize(mul(2.0f * normalMap.Sample(samLinear, texCoord*0.15f+float(0.5).xx).rgb - 1.0f, tangentFrame));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal1a = normalize(mul(2.0f * normalMap.Sample(samLinear, texCoord*0.15f+float(0.5).xx).rgb - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal2a = normalize(mul(2.0f * normalMap.Sample(samLinear, texCoord*0.15f+float(0.5).xx).rgb - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal3a = normalize(mul(2.0f * normalMap.Sample(samLinear, texCoord*0.15f+float(0.5).xx).rgb - 1.0f, tangentFrame));
		
		float3 normal = normalize(normal0a * normalModifier.x + normal1a * normalModifier.y +
								  normal2a * normalModifier.z + normal3a * normalModifier.w);
		
		texCoord = IN.Tex.xy;
		texCoord.x += sin(timer * 0.002f + 3.0f * abs(position.y)) * (refractionScale * min(depth2, 1.0f));
		float3 refraction = screen.Load(texCoord*dims.xy,0).rgb;
		if(mul(float4(PositionFromDepth(texCoord).xyz, 1.0f), matViewInverse).y > level)
			refraction = color2;

		float4x4 matTextureProj = mul(matViewProj, matReflection);
				
		float3 waterPosition = surfacePoint.xyz;
		waterPosition.y -= (level - waterLevel);
		float4 texCoordProj = mul(float4(waterPosition, 1.0f), matTextureProj);
		
		float4 dPos;
		dPos.x = texCoordProj.x + displace * normal.x;
		dPos.z = texCoordProj.z + displace * normal.z;
		dPos.yw = texCoordProj.yw;
		texCoordProj = dPos;		
		
		float3 reflect = reflectionMap.Sample(samLinear, texCoordProj.xy/texCoordProj.w).rgb;
		
		float fresnel = fresnelTerm(normal, eyeVecNorm);
		
		float3 depthN = depth * fadeSpeed;
		float3 waterCol = saturate(length(sunColor) / sunScale);
		refraction = lerp(lerp(refraction, depthColour * waterCol, saturate(depthN / visibility)),
						  bigDepthColour * waterCol, saturate(depth2 / extinction));

		float foam = 0.0f;		

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001f * wind + sin(timer * 0.001 + position.x) * 0.005;
		float2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002f * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		if(depth2 < foamExistence.x)
			foam = (foamMap.Sample(samLinear, texCoord) + foamMap.Sample(samLinear, texCoord2)) * 0.5f;
		else if(depth2 < foamExistence.y)
		{
			foam = lerp((foamMap.Sample(samLinear, texCoord) + foamMap.Sample(samLinear, texCoord2)) * 0.5f, 0.0f,
						 (depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x));
			
		}
		
		if(maxAmplitude - foamExistence.z > 0.0001f)
		{
			foam += (foamMap.Sample(samLinear, texCoord) + foamMap.Sample(samLinear, texCoord2)) * 0.5f * 
				saturate((level - (waterLevel + foamExistence.z)) / (maxAmplitude - foamExistence.z));
		}


		half3 specular = 0.0f;

		half3 mirrorEye = (2.0f * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		half dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
		specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 512.0f)) * (shininess * 1.8f + 0.2f))* sunColor;
		specular += specular * 25 * saturate(shininess - 0.05f) * sunColor;		

		color = lerp(refraction, reflect, fresnel);
		color = saturate(color + max(specular, foam * sunColor));
		
		color = lerp(refraction, color, saturate(depth * shoreHardness));
	}
	if(position.y > level)
		color = color2;

	return float4(color, 1.0f);
}


technique11 Water
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, QuadVS()));
		SetGeometryShader(CompileShader(gs_4_0, QuadGS()));
        SetPixelShader(CompileShader(ps_5_0, waterPS()));
        
		SetRasterizerState(rsCullNone);
        SetDepthStencilState(DisableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
