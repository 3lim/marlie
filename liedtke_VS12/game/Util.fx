SamplerState samPoint {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = Wrap;
};

float2 GetFPBias()
{
	return float2(0.5, 0);
}

float linstep(float min, float max, float x)  
{  
  return clamp((x - min) / (max - min), 0, 1);  
}  

float ReduceLightBleeding(float p, float Amount)  
{  
  // Remove the [0, Amount] tail and linearly rescale (Amount, 1].  
   return linstep(Amount, 1, p);  
}  

float ChebyshevUpperBound(float2 Moments,float mean, float minVariance)
{
	float p = (mean <= Moments.x);

	float variance = Moments.y- (Moments.x*Moments.x);
	variance = max(variance, minVariance); //MinVariance
	//upperBound
	float uB = mean - Moments.x;
	float p_max = variance/(variance + uB*uB);
	//return max(p, p_max);
	//reduce LightBleeding
	return max(p, p_max);
}

float2 ComputeMoments(float Depth)
{
	float2 Moments;
	Moments.x = Depth;

	float dx = ddx(Depth);// Compute partial derivatives of depth.  
	float dy = ddy(Depth);// Compute partial derivatives of depth.  
	Moments.y = Depth*Depth/*+0.25*(dx*dx+dy*dy)*/;
	return Moments;
}

float4 sampleSATSum(Texture2D satTex, float4 coords, int2 coordOffset)
{
	//sample corners
	float4 ld = satTex.SampleLevel(samPoint, coords.xy, 0, coordOffset);
	float4 lu = satTex.SampleLevel(samPoint, coords.xw, 0, coordOffset);
	float4 rd = satTex.SampleLevel(samPoint, coords.zy, 0, coordOffset);
	float4 ru = satTex.SampleLevel(samPoint, coords.zw, 0, coordOffset);

	return (ru-rd-lu+ld);
}

float4 sampleSAT(Texture2D satTex, float4 coords, float2 size)
{
	float2 deltaSize = 1.f/size;
	float4 realCoords = coords-size.xyxy;
	float2 dims = (coords.zw-coords.xy)*deltaSize;

	//sample sum and divide for average
	return sampleSATSum(satTex, realCoords, int2(0,0))/(dims.x*dims.y);
}

float4 samSAT(Texture2D satTex, float2 coords, float2 kernelSize)
{
	float4 result = satTex.Sample(samPoint, coords +0.5*kernelSize); //rechts unten
	result -= satTex.Sample(samPoint, coords + float2(0.5, -0.5)*kernelSize); // rechts Oben
	result -= satTex.Sample(samPoint, coords + float2(-0.5, 0.5)*kernelSize); //links unten
	result += satTex.Sample(samPoint, coords - 0.5*kernelSize); // links Oben

	//Average = (RU-RO-lU+LO)/w*h
	result /= kernelSize.x * kernelSize.y;

	return result;
	//sample corners

	//float2 dim;
	//satTex.GetDimensions(dim.x, dim.y);
	////float2 rightStep = float2(1,0);
	////float2 upStep = float2(0,1);
	//float2 rightStep = float2(1.f/(dim.x), 0);
	//float2 upStep = float2(0, 1.f/(dim.y));
	//float4 ld = satTex.Sample(samPoint, coords + rightStep*-pixelSpan + upStep*-pixelSpan); //linksunten
	//float4 lu = satTex.Sample(samPoint, coords + rightStep*-pixelSpan + upStep*pixelSpan); //linkOben
	//float4 rd = satTex.Sample(samPoint, coords + rightStep*pixelSpan + upStep*-pixelSpan); //rechtsUnten
	//float4 ru = satTex.Sample(samPoint, coords + rightStep*pixelSpan + upStep*pixelSpan); //rechtsOben

	//return (rd-ru-ld+lu)/float(pixelSpan*pixelSpan);
}
