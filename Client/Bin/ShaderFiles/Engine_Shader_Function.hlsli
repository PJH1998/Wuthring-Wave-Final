// Emissive효과를 넣을지 판단할 때 사용하는 RGB 계수
float g_fLuminence[3] = { 0.2126, 0.7152, 0.0722 };

float g_iShadowMapSizeX = 8192;
float g_iShadowMapSizeY = 4608;

float Luminame(float3 vColor)
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}

float Random(float2 vRange)
{
    return frac(sin(dot(vRange.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

float SampleShadowPCF(Texture2DArray<float> ShadowMap, SamplerComparisonState Sampler, float3 UVDepth, int iCascadeIndex, int iNumWeight)
{
    float2 vTexelSize = float2((1.f / g_iShadowMapSizeX), (1.f / g_iShadowMapSizeY));
    float fShadow = 0.f;
    
    [unroll]
    for (int x = -iNumWeight; x <= iNumWeight; ++x)
    {
        for (int y = -iNumWeight; y <= iNumWeight; ++y)
        {
            float2 vOffset = float2(x, y) * vTexelSize;
            
            fShadow += ShadowMap.SampleCmpLevelZero(Sampler, float3(UVDepth.xy + vOffset, iCascadeIndex), UVDepth.z);
        }
    }
    
    fShadow = fShadow / pow((iNumWeight * 2 + 1), 2);
    
    return fShadow;
}