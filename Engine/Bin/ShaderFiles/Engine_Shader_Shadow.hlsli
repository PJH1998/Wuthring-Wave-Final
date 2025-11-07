#include "Engine_Shader_Function.hlsli"
#include "Engine_Shader_Defines.hlsli"

#define MAX_SECTOR 64

cbuffer CSMDatas : register(b1)
{
    float4 g_vClipDistances;
    float g_fLastDistance;
    float3 padding;
};

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

float4 g_fShadowBais = 0.0001f;//float4(0.0001f, 0.02f, 0.03f, 0.05f);
float4 g_fMinShadowBias = 0.0001f;

float g_fShadowMapBais = 0.01f;

float g_DebugSlopeScale = 2.f;

float g_iCascadeSizeX = 4096;
float g_iCascadeSizeY = 2304;

int iNumSectorX;
int iNumSectorToLayer;
float2 vSectorWorldSize;
float2 vMin;
float2 vShadowMapSize;
 
float4x4 g_SectorViewMatrix[MAX_SECTOR];
float4x4 g_SectorProjMatrix[MAX_SECTOR];
float4 g_vSectorUV[16];

float ShadowPCF(float3 UVDepth, int iIndex, int iNumWeight, Texture2DArray<float> ShadowMap, float2 vTexelSize, float2 vMinUV, float2 vMaxUV)
{
    float fShadow = 0.f;
    
    int iRadius = iNumWeight * 2 + 1;
    
    [unroll]
    for (int x = -iNumWeight; x <= iNumWeight; ++x)
    {
        [unroll]
        for (int y = -iNumWeight; y <= iNumWeight; ++y)
        {
            float2 vOffset = float2(x, y) * vTexelSize;
            
            float2 vUV = UVDepth.xy + vOffset;
            
            vUV.x = clamp(vUV.x, vMinUV.x, vMaxUV.x);
            vUV.y = clamp(vUV.y, vMinUV.y, vMaxUV.y);
            
            fShadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(vUV, iIndex), UVDepth.z);
        }
    }
    
    fShadow /= pow(iRadius, 2);
    
    return fShadow;
}

float ShadowPCSS(float3 UVDepth, int iIndex, int iNumWeight, Texture2DArray<float> ShadowMap, float2 vTexelSize, float2 vMinUV, float2 vMaxUV)
{
    float fShadow = 1.f;
    
    
    
    
    return fShadow;
}

float RPB_Gradiant(float fViewDepth)
{
    float DepthDDX = ddx(fViewDepth * 0.0001f);
    float DepthDDY = ddy(fViewDepth * 0.0001f);
    
    float GradiantX = abs(DepthDDX);
    float GradiantY = abs(DepthDDY);
    
    float Gradiant = length(float2(GradiantX, GradiantY));
   
    return Gradiant;
}

int2 Find_Sector(float4 vWorldPos)
{
    float2 vSectorPos = vWorldPos.xz - vMin;
    
    int2 vSector = (int2) floor(vSectorPos / vSectorWorldSize);
    
    int iIndex = vSector.x + (vSector.y * iNumSectorX);
    
    int iLayer = floor(iIndex / iNumSectorToLayer);
    
    int2 vIndex = int2(iIndex, iLayer);
    
    return vIndex;
}

float Compute_Cascade(float fViewZ, float NdotL, float4 vWorldPos, Texture2DArray<float> Cascade)
{
    int iCascadeIndex = 0;
    
    float fFinalShadow = 1.f;
    
    for (int i = 0; i < 4; i++)
    {
        if (fViewZ > g_vClipDistances[i])
            iCascadeIndex = i;
    }
    
    if (fViewZ >= g_fLastDistance)
        return fFinalShadow;

    float Gradiant = RPB_Gradiant(fViewZ);
    //float fSlopeFactor = (1.f - fDot); // Row
    float fSlopeFactor = sqrt(1.f - pow(NdotL, 2)); // High

    float BlendFactor = 0.f;
    
    float fShadowBlend = 0.f;
    
    float2 vTexelSize = float2((1.f / g_iCascadeSizeX), (1.f / g_iCascadeSizeY));
    
    // Blend Cascade
    if (iCascadeIndex < 3)          // Max Cascade Check
    {
        int iBlendCascadeIndex = iCascadeIndex + 1;
    
        float CurrentNear = g_vClipDistances[iCascadeIndex];
        float CurrentFar = g_vClipDistances[iBlendCascadeIndex];
        
        float BlendRegion = (CurrentFar - CurrentNear) * 0.15f; // Cascade Blend Distance ( Begin ratio 0.85)
        
        BlendFactor = saturate((fViewZ - (CurrentFar - BlendRegion)) / BlendRegion);
        
        vector vShadowBlendPos;
        matrix matShadowBlendLightVP;
        
        matShadowBlendLightVP = mul(g_ShadowViewMatrix[iBlendCascadeIndex], g_ShadowProjMatrix[iBlendCascadeIndex]);
        vShadowBlendPos = mul(vWorldPos, matShadowBlendLightVP);
        
    //    if (IsInNDC(vShadowBlendPos))
        {
            float2 vBlendTexcoord = Compute_Texcoord(vShadowBlendPos.xy);
        
            float fBlendBias = max(g_fShadowBais[iBlendCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
            fBlendBias = max(fBlendBias, g_fMinShadowBias[iBlendCascadeIndex]);
            float fBlendDepth = vShadowBlendPos.z - fBlendBias;

            fShadowBlend = ShadowPCF(float3(vBlendTexcoord, fBlendDepth), iBlendCascadeIndex, 1, Cascade, vTexelSize, float2(0.f, 0.f), float2(1.f, 1.f)); // 2 == Kernel size
        }
    }
    
    // Current Cascade
    
    vector vShadowPos;
    matrix matShadowLightVP;
    float fBias = 0.f;
    
    matShadowLightVP = mul(g_ShadowViewMatrix[iCascadeIndex], g_ShadowProjMatrix[iCascadeIndex]);
    vShadowPos = mul(vWorldPos, matShadowLightVP);
    
    //if (IsInNDC(vShadowPos))
    {
        float2 vTexcoord = Compute_Texcoord(vShadowPos.xy);
    
        fBias = max(g_fShadowBais[iCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
        fBias = max(fBias, g_fMinShadowBias[iCascadeIndex]);
    
        float fDepth = vShadowPos.z - fBias;
    
        float fShadow = ShadowPCF(float3(vTexcoord, fDepth), iCascadeIndex, 1, Cascade, vTexelSize, float2(0.f, 0.f), float2(1.f, 1.f));
        
        fFinalShadow = lerp(fShadow, fShadowBlend, BlendFactor);
    
        fFinalShadow = saturate(fFinalShadow + 0.3f);
    }
    
    return fFinalShadow;
}

float Compute_ShadowMap(float fViewZ, float NdotL, float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    float fShadow = 1.f;
    
    int2 vSector = Find_Sector(vWorldPos);
    
    int iIndex = vSector.x;
    
    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    vProjPos.xyz /= vProjPos.w;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    
    int iUVIndex = iIndex % iNumSectorToLayer;
    
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
    float Gradiant = RPB_Gradiant(fViewZ);
    //float fSlopeFactor = (1.f - fDot); // Row
    float fSlopeFactor = sqrt(1.f - pow(NdotL, 2)); // High

    float BiasFactor = 0.f;
    
    float fBias = max(g_fShadowMapBais, g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
    float fDepth = vProjPos.z - fBias;
    
    fShadow = ShadowPCF(float3(vTexcoord, fDepth), vSector.y, 2, ShadowMapTexture, vTexelSize, vStartTex, vEndTex);
    
    return fShadow;
}