// Compute the filter size in pixels
float2 GetFilterSize(float2 dx, float2 dy, float2 TexSize)
{
    return 2 * (abs(dx) + abs(dy)) * TexSize;
}
// Compute the upper left and size of the filter tile
// MinFilterWidth, MaxSizeDerivatives and TexSize given in texels
// Rest of the parameters and returns are in normalized coordinates
// NOTE: Can provide an upper bound for the size (in texels) computed via derivatives.
// This is necessary since GPU's finite differencing screws up in some cases,
// returning rediculous sizes here. For operations that loop on the filter area
// this is a big problem...
float2 GetFilterTile(float2 tc, float2 dx, float2 dy, float2 TexSize,
                     float2 MinFilterWidth, float2 MaxSizeDerivatives,
                     out float2 Size)
{
    float2 TexelSize = 1 / TexSize;

    // Compute the filter size based on derivatives
    float2 SizeDerivatives = min(GetFilterSize(dx, dy, TexSize),
                                 MaxSizeDerivatives);
    
    // Force an integer tile size (in pixels) so that bilinear weights are consistent
    Size = round(max(SizeDerivatives, MinFilterWidth)) * TexelSize;
    
    // Compute upper left corner of the tile
    return (tc - 0.5 * (Size - TexelSize));
}

SamplerComparisonState samPCF
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = Less_Equal;
};
SamplerState sam2Point
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = MIN_MAG_LINEAR_MIP_POINT;
};


//--------------------------------------------------------------------------------------
// Percentage closer filtering
//--------------------------------------------------------------------------------------
float4 TexturePCF(Texture2D texShadow,float2 tc,
                       int2 size)
{
	float2 texSize;
	texShadow.GetDimensions(texSize.x, texSize.y);
	float2 texelSize = 1.f/texSize;
	float2 blurSize = size*texelSize;
	float2 coordsUL = tc + blurSize*-0.5;
	float4 result = 0;
	for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            float2 TexCoord = coordsUL + float2(x, y) * texelSize;
            
            result += texShadow.Sample(sam2Point, TexCoord);
        }
    }
    result /= float(size.x * size.y);
    return result;
}
float4 TexturePCF(Texture2DMS<float4,4> texShadow,float2 tc,
                       int2 size)
{
	float3 texSize;
	texShadow.GetDimensions(texSize.x, texSize.y, texSize.z);
	float2 texelSize = 1.f/texSize;
	float2 blurSize = size*texelSize;
	float2 coordsUL = tc + blurSize*-0.5;
	float4 result = 0;
	for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            float2 TexCoord = coordsUL + float2(x, y) * texelSize;
            
            result += texShadow.Load(int2(tc*texSize.xy),0);
        }
    }
    result /= float(size.x * size.y);
    return result;
}
//float4 TexturePCF(Texture2D texShadow,float2 tc,
//                       float2 dx,
//                       float2 dy,
//                       float Distance)
//{
//	float2 size;
//	texShadow.GetDimensions(size.x, size.y);
//    float2 texSize = 1 / size;
//    
//    float2 kernel = (dx, dy)*texSize;
//
//	float2 coordsUL = tc + kernel*-0.5;
//	float4 result = 0;
//    int2 PixelSize = round(kernel * size);
//    for (int y = 0; y < PixelSize.y; ++y) {
//        for (int x = 0; x < PixelSize.x; ++x) {
//            float2 TexCoord = coordsUL + float2(x, y) * texSize;
//            
//            result += texShadow.SampleCmpLevelZero(samPCF, TexCoord, Distance);
//        }
//    }
//    
//    return result / float(kernel.x * kernel.y);
//}
