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
        [unroll]
        for (int y = -iNumWeight; y <= iNumWeight; ++y)
        {
            float2 vOffset = float2(x, y) * vTexelSize;
            
            fShadow += ShadowMap.SampleCmpLevelZero(Sampler, float3(UVDepth.xy + vOffset, iCascadeIndex), UVDepth.z);
        }
    }
    
    fShadow = fShadow / pow((iNumWeight * 2 + 1), 2);
    
    return fShadow;
}

float RPB_Gradiant(float fViewDepth)
{
    float DepthDDX = ddx(fViewDepth * 0.0001f);
    float DepthDDY = ddy(fViewDepth * 0.0001f);
        
//    float2 vTexelSize = float2((1.f / g_iShadowMapSizeX), (1.f / g_iShadowMapSizeY));
    
    float GradiantX = abs(DepthDDX);
    float GradiantY = abs(DepthDDY);
        
    float Gradiant = length(float2(GradiantX, GradiantY));
   
    return Gradiant;
}

bool Outline(float fWinSizeX, float fWinSizeY, Texture2D DepthTexture, sampler Sampler, float2 UV, float fCompareDepth, float fWeight, matrix ProjMatrixInv)
{
    float2 vTexelSize = float2((1.f / fWinSizeX), (1.f / fWinSizeX));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = UV + vOffset;
            vector DepthDesc = DepthTexture.Sample(Sampler, UV + vOffset);
            
            if(DepthDesc.x == 1.f)
                return true;
                
            vector vWorldPos;
            
            vWorldPos.x = vTexcoord.x * 2.f - 1.f;
            vWorldPos.y = vTexcoord.y * -2.f + 1.f;
            vWorldPos.z = DepthDesc.x;
            vWorldPos.w = 1.f;
    
            vWorldPos *= DepthDesc.y;

            vWorldPos = mul(vWorldPos, ProjMatrixInv);
                    
            if (abs(fCompareDepth - vWorldPos.z) >= fWeight)
                return true;
        }
    } 
        
    return false;
}


bool Outline_Normal(float fWinSizeX, float fWinSizeY, Texture2D NormalTexture, sampler Sampler, float2 UV, float3 vCompareNormal, float fWeightRadians)
{
    float2 vTexelSize = float2((1.f / fWinSizeX), (1.f / fWinSizeX));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = UV + vOffset;
            float3 NormalDesc = NormalTexture.Sample(Sampler, UV + vOffset).xyz;
            float3 vNormal = normalize(vector(NormalDesc.xyz * 2.f - 1.f, 0.f));
   
            if (dot(vNormal, vCompareNormal) <= fWeightRadians)
                return true;
        }
    }
        
    return false;
}

float2 Compute_UV_Offset()
{
    float2 vOffest = 0.f;

    
    return vOffest;
}