SamplerComparisonState samPCF
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = Less_Equal;
};


////--------------------------------------------------------------------------------------
//// Percentage closer filtering
////--------------------------------------------------------------------------------------
//float ShadowPCF(Texture2D texShadow,float2 tc,
//                       float2 dx,
//                       float2 dy,
//                       float Distance)
//{
//    // Preshader
//	float2 size;
//	texShadow.GetDimesnsions(size.x, size.y);
//    float2 texSize = 1 / size;
//    
//    float2 kernel = (dx, dy)*texSize;
//
//	float2 coordsUL = tc + kernel*-0.5;
//	float result = 0;
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
