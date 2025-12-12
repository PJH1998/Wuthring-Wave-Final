#include "Engine_Shader_Function.hlsli"
#include "Engine_Shader_Defines.hlsli"

#define MAX_SECTOR 64

float4 g_vClipDistances;
float g_fLastDistance;

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

float4 g_fShadowBais = 0.0001f;//float4(0.0001f, 0.02f, 0.03f, 0.05f);
float4 g_fMinShadowBias = 0.0001f;

float g_fShadowMapBais = 0.001f;

float g_DebugSlopeScale = 2.f;

float g_iCascadeSizeX = 4096;
float g_iCascadeSizeY = 2304;

int iNumSector;
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
            //fShadow = min(ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(vUV, iIndex), UVDepth.z), fShadow);

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

    //float Gradiant = RPB_Gradiant(fViewZ);
    //float fSlopeFactor = (1.f - fDot); // Row
    //float fSlopeFactor = sqrt(1.f - pow(NdotL, 2)); // High

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
        
            float fBlendBias = g_fShadowBais[iBlendCascadeIndex];//            max(g_fShadowBais[iBlendCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
            //fBlendBias = max(fBlendBias, g_fMinShadowBias[iBlendCascadeIndex]);
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
    
        fBias = g_fShadowBais[iCascadeIndex]; //max(g_fShadowBais[iCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
        //fBias = max(fBias, g_fMinShadowBias[iCascadeIndex]);
    
        float fDepth = vShadowPos.z - fBias;
    
        float fShadow = ShadowPCF(float3(vTexcoord, fDepth), iCascadeIndex, 1, Cascade, vTexelSize, float2(0.f, 0.f), float2(1.f, 1.f));
        
        fFinalShadow = lerp(fShadow, fShadowBlend, BlendFactor);
    
        fFinalShadow = saturate(fFinalShadow + 0.3f);
    }
    
    return fFinalShadow;
}

int2 Find_Sector(float4 vWorldPos)
{
    float2 vSectorPos = vWorldPos.xz - vMin;
    
    int2 vSector = (int2) floor((vSectorPos + 0.1f) / vSectorWorldSize);
    
    int iIndex = vSector.x + (vSector.y * iNumSectorX);
    
    int iLayer = floor(iIndex / iNumSectorToLayer);
    
    int2 vIndex = int2(iIndex, iLayer);
    
    return vIndex;
}

struct NeighborData
{
    int iNumNeighbor;
    int2 vSectos[4];
};

NeighborData Check_Neighbor(float2 vTexcoord, int2 vSector, float fNeighborDistance)
{
    NeighborData Neighbor = (NeighborData) 0;
    
    bool4 Dir = false; // x = LEFT, y = RIGHT, z = UP, w = BOTTOM
    
    Dir.x = (vTexcoord.x - fNeighborDistance) <= 0.f;
    Dir.y = (vTexcoord.x + fNeighborDistance) >= 1.f;
    Dir.z = (vTexcoord.y - fNeighborDistance) <= 0.f;
    Dir.w = (vTexcoord.y + fNeighborDistance) >= 1.f;
   
    int2 TempSectors[4];
    
    for (int i = 0; i < 4; ++i)
    {
        int2 vOffset = 0;
        vOffset.x = i >= 2 ? 0 : i == 1 ? 1 : 0;
        vOffset.y = i < 2 ? 0 : i == 2 ? -iNumSectorX : iNumSectorX;
        
        int iSectorOffset = vOffset.x + vOffset.y;
        int iNeighborSector = vSector.x + iSectorOffset;
        
        if (iNeighborSector >= iNumSector || iNeighborSector < 0)
            continue;
            
        int iLayer = floor(iNeighborSector / iNumSectorToLayer);
        
        if(Dir[i])
        {
            TempSectors[Neighbor.iNumNeighbor] = int2(iNeighborSector, iLayer);
            //Neighbor.vSectos[Neighbor.iNumNeighbor] = int2(iNeighborSector, iLayer);
            Neighbor.iNumNeighbor += 1;
        }
    }
    
    for (int j = 0; j < Neighbor.iNumNeighbor; ++j)
    {
        Neighbor.vSectos[j] = TempSectors[j];
    }
    
    return Neighbor;
}

float Compute_NeighborShadow(int2 vNeighborSector, float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    int iIndex = vNeighborSector.x;

    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    
    if (false == IsInNDC(vProjPos))
        return 1.f;
        
    vProjPos.xyz /= vProjPos.w;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    
    int iUVIndex = iIndex % iNumSectorToLayer;
   
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    if (any(vTexcoord < vStartTex) || any(vTexcoord > vEndTex))
        return 1.f;
    
    vTexcoord = clamp(vTexcoord, vStartTex, vEndTex);
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
    float fBias = g_fShadowMapBais; //max(g_fShadowMapBais, g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
    float fDepth = vProjPos.z - fBias;
    
    float fShadow = 0.f;
    
    fShadow = ShadowPCF(float3(vTexcoord, fDepth), vNeighborSector.y, 2, ShadowMapTexture, vTexelSize, vStartTex, vEndTex);
    
    return fShadow;
}

float Compute_ShadowMap(float fViewZ, float NdotL, float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    float fShadow = 1.f;
    
    int2 vSector = Find_Sector(vWorldPos);
    
    int iIndex = vSector.x;
    
    if (iIndex >= iNumSector || iIndex < 0)
        return fShadow;
    
    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    vProjPos.xyz /= vProjPos.w;
        
    if (vProjPos.z >= 1.f || vProjPos.z < 0.f)
        return fShadow;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    NeighborData Neighbor =  Check_Neighbor(vTexcoord, vSector, 0.005f);
    
    if (any(Neighbor.iNumNeighbor))
    {
        for (int i = 0; i < Neighbor.iNumNeighbor; ++i)
        {
            fShadow = min(Compute_NeighborShadow(Neighbor.vSectos[i], vWorldPos, ShadowMapTexture), fShadow);
        }
    }
    
    int iUVIndex = iIndex % iNumSectorToLayer;
   
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
    //float Gradiant = RPB_Gradiant(fViewZ);
    //float fSlopeFactor = sqrt(1.f - pow(NdotL, 2));
    float fBias = g_fShadowMapBais; //max(g_fShadowMapBais, fSlopeFactor * Gradiant); // 
    
    float fDepth = vProjPos.z - fBias;
   
    fShadow = min(ShadowPCF(float3(vTexcoord, fDepth), vSector.y, 2, ShadowMapTexture, vTexelSize, vStartTex, vEndTex), fShadow);
//    fShadow = min(ShadowMapTexture.SampleCmpLevelZero(ShadowSampler, float3(vTexcoord, vSector.y), fDepth), fShadow);
    return fShadow;
}