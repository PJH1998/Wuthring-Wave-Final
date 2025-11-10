#include "Engine_ComputeShader_Function.hlsli"

//#pragma pack_matrix(row_major)

#define THREAD_X 8
#define THREAD_Y 8
#define LIGHT_THREAD_Z 8
#define BEER_THREAD_Z 1

#define MAX_MIPLEVEL 10
#define MIN_MIPLEVEL 3

#define EPSILON 1e-05

#define MAX_SECTOR 64

struct VF_Light
{
    uint iType; // 0 = Directional, 1 = Point
    float fRange;
    float Padding[2];
    float4 vDiffuse;
    float4 vDirection;
    float4 vPosition;
} ;

StructuredBuffer<VF_Light> g_LightDatas : register(t0);

Texture2D<float> g_MipDepthTexture : register(t1);
Texture2DArray<float> g_ShadowMapTexture : register(t2);

Texture3D<float4> VFLightTexture : register(t3);

RWTexture3D<float4> OutputTexture : register(u0);

SamplerState DefaultSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

cbuffer VF_Data : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    float4x4 InvViewMatrix;
    float4x4 InvProjMatrix;
    float fFogNear;
    float fFogFar;
    uint iSliceCount;
    uint iLightCount;
    float3 vFroxelSize;
    float Padding;
    float fCamNear;
    float fCamFar;
    float fScreenX;
    float fScreenY;
    float fLightIntensity;
    float fDensity;
    float fPhaseFunctionG;
    float fDensityScale;
    float fFogMaxHeight; 
    float fFogMinHeight; 
    float fHegihtFallOff;
    float fDistanceFallOff; 
    float fGroundFallOff; 
    float3 Padding1;
    float3 vFogColor;
};

cbuffer ShadowMap_Data : register(b1)
{
    float4x4 g_SectorViewMatrix[MAX_SECTOR];
    float4x4 g_SectorProjMatrix[MAX_SECTOR];
    float4 g_vSectorUV[16];

    int iNumSector;
    int iNumSectorX;
    int iNumSectorToLayer;
    float Padding2;
    
    float2 vSectorWorldSize;
    float2 Padding3;
    
    float2 vMin;
    float2 Padding4;
    
    float2 vShadowMapSize;
}

float ComputeSliceDepth(uint iSlice, uint iSliceCount, float fFogNear, float fFogFar)
{
    return fFogNear * pow(fFogFar / fFogNear, (float) iSlice / (float(iSliceCount - 1)));
}

float ComputeDepthToProjZ(float fViewZ, float fNear, float fFar)
{
    return ((fFar * fViewZ) / (fFar - fNear)) - (fFar * fNear) / (fFar - fNear); // (f * z / f-n) - (f*n / (f - n)  = 투영 보정 (w 나누기 하기 전 )
}

uint ComputeMipLevel(float4x4 ProjMatrix, float fViewZ, float fScreenSizeX, float fScreenSizeY)
{
    float fFovX = (1.f / ProjMatrix._11);
    float fFovY = (1.f / ProjMatrix._22);
    
    float fFroxelWidth = 2.f * fViewZ * fFovX / fScreenSizeX;
    float fFroxelHeight = 2.f * fViewZ * fFovY / fScreenSizeY;
    
    float fPixelSize = max(fFroxelWidth, fFroxelHeight);
    
    uint iMipLevel = (uint) ceil(log2(fPixelSize * fScreenSizeY));
    
    iMipLevel = max(min(iMipLevel, MAX_MIPLEVEL), MIN_MIPLEVEL);
    
    return iMipLevel;
}

float ReturnMinDepth(uint iMipLevel, uint2 DTID, float2 vScreenSize, float2 vFroxelSize)
{
    float fDepth = 0.f;
    
    uint FroxelScale = vScreenSize / vFroxelSize;
    
    uint iMipScale = 1 << iMipLevel;
    
    uint2 vIndex = (DTID.xy * FroxelScale) / iMipScale;
    
    float2 vTexcoord = (float2) vIndex + 0.5f * (iMipScale / vScreenSize);
    
    fDepth = g_MipDepthTexture.SampleLevel(DefaultSampler, vTexcoord, (iMipLevel - 1));
    
    return fDepth;
}

float HenyeyGreensteinPhasefunction(float3 LightDir, float3 LightOutDir, float G)
{
    float cosTheta = dot(LightDir, LightOutDir);
    float G2 = pow(G, 2);
    float Denom = pow(1.f + G2 - 2.f * G * cosTheta, 3.f / 2.f);
    
    return (1.f / (4.f * PI)) * ((1.f - G2) / max(Denom, EPSILON)); /**/
}

/*------------------------------------ SHADOW_MAP ------------------------------------*/

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
        
        if (iNeighborSector >= iNumSector)
            continue;
            
        int iLayer = floor(iNeighborSector / iNumSectorToLayer);
        
        if (Dir[i])
        {
            TempSectors[Neighbor.iNumNeighbor] = int2(iNeighborSector, iLayer);
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
    vProjPos.xyz /= vProjPos.w;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    
    int iUVIndex = iIndex % iNumSectorToLayer;
   
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    vTexcoord = clamp(vTexcoord, vStartTex, vEndTex);
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
    float fBias = 0.01f;
    
    float fDepth = vProjPos.z - fBias;
    
    float fShadow = 0.f;
    
    fShadow = ShadowPCF(float3(vTexcoord, fDepth), vNeighborSector.y, 1, ShadowMapTexture, vTexelSize, vStartTex, vEndTex);
    
    return fShadow;
}

float Compute_ShadowMap(float fViewZ, float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    float fShadow = 1.f;
    
    int2 vSector = Find_Sector(vWorldPos);
    
    int iIndex = vSector.x;
    
    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    vProjPos.xyz /= vProjPos.w;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    NeighborData Neighbor = Check_Neighbor(vTexcoord, vSector, 0.05f);
    
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
    
    float fBias = 0.01f;
    
    float fDepth = vProjPos.z - fBias;
   
    fShadow = min(ShadowPCF(float3(vTexcoord, fDepth), vSector.y, 1, ShadowMapTexture, vTexelSize, vStartTex, vEndTex), fShadow);
    //ShadowMapTexture.SampleCmpLevelZero(ShadowSampler, float3(vTexcoord, vSector.y), fDepth);
    
    return fShadow;
}
/*------------------------------------ SHADOW_MAP ------------------------------------*/

[numthreads(THREAD_X, THREAD_Y, LIGHT_THREAD_Z)]
void ComputeLight(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{   
    if (any(DTID >= vFroxelSize))
        return;
        
    float fViewZ = ComputeSliceDepth(DTID.z, iSliceCount, fFogNear, fFogFar);
    //float fViewZ = ComputeSliceDepth(DTID.z, iSliceCount, fCamNear, fCamFar);
    
    float2 vUV = (float2) (DTID.xy + 0.5f) / float2(vFroxelSize.xy);
    
    float vNdcZ = ComputeDepthToProjZ(fViewZ, fCamNear, fCamFar) / fViewZ;
    
    float2 vNdcXY = float2(vUV.x * 2.f - 1.f, vUV.y * -2.f + 1.f);
   
    float4 vProjPos = float4(vNdcXY, vNdcZ, 1.f);
    vProjPos *= fViewZ;
    
    float4 vViewPos = mul(vProjPos, InvProjMatrix);
    
    float4 vWorldPos = mul(vViewPos, InvViewMatrix);
    
    float3 vLighting = 0.f;
    
    //uint iMipLevel = ComputeMipLevel(ProjMatrix, fViewZ, fScreenX, fScreenY);
    //float fMinDetph = ReturnMinDepth(iMipLevel, DTID.xy, float2(fScreenX, fScreenY), vFroxelSize.xy);
    //
    //fMinDetph = fMinDetph == 0.f ? fViewZ : fMinDetph;
    //
    //if (fMinDetph < fViewZ)
    //{
    //    OutputTexture[DTID.xyz] = float4(vLighting, fDensity);
    //    return;
    //}
    
    float fSkyWeight = saturate(exp(-fHegihtFallOff * (vWorldPos.y - fFogMaxHeight)));
    float fGroundWeight = saturate(exp(fGroundFallOff * (fFogMinHeight - vWorldPos.y)) - 1.f);
    
    float fHeightWeight = max(fSkyWeight, fGroundWeight);
    
    float fDistance = length(vViewPos.xyz);
    
    float fDistanceWeight = saturate(1.f - exp(-fDistance * fDistanceFallOff));
    
    float3 vOutDir = normalize(vViewPos.xyz * -1.f);
    
    float fVisible = max(Compute_ShadowMap(fViewZ, vWorldPos, g_ShadowMapTexture), 0.5f);
        
    for (int i = 0; i < iLightCount; ++i)
    {
        VF_Light Light = g_LightDatas[i];
        
        float3 LightDirection = 0.f;
        float fAtt = 1.f;
        
        switch (Light.iType)
        {
            case 0: // DIRECTIONAL
                LightDirection = normalize(Light.vDirection);
                fAtt = fVisible;
                break;
            case 1: // POINT
                LightDirection = normalize(vWorldPos.xyz - Light.vPosition.xyz);
                fAtt = saturate((Light.fRange - length(LightDirection)) / Light.fRange);
                break;
        }
        
        float PhaseFunction = HenyeyGreensteinPhasefunction(LightDirection, vOutDir, fPhaseFunctionG);
        
        float3 vFinalColor = lerp(vFogColor, (Light.vDiffuse.xyz), 0.4f);
        
        vLighting += vFinalColor * fAtt * PhaseFunction;
    }
   
    float fFinalDensity = fDensity * fDistanceWeight * fHeightWeight;
   
    OutputTexture[DTID.xyz] = float4(vLighting * fLightIntensity * fFinalDensity, fFinalDensity);
}

float4 ScatterStep(float3 AccumLight, float AccumTransmittance, float3 SliceLight, float SliceDensity, float Tickness)
{
    float Density = max(SliceDensity, 0.000001f);
    Density *= fDensityScale;
    
    float SliceTransmittance = exp(-Density * Tickness);
    
    float3 SliceLightIntegral = SliceLight * (1.f - SliceTransmittance) / Density;
    
    float3 ResultLight = AccumLight + (SliceLightIntegral * AccumTransmittance);
    float ResultTransmittance = AccumTransmittance * SliceTransmittance;
    
    return float4(ResultLight, ResultTransmittance);
}

[numthreads(THREAD_X, THREAD_Y, BEER_THREAD_Z)]
void VolumetricFog(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (any(DTID >= vFroxelSize))
        return;
        
    float4 Accum = float4(0.f, 0.f, 0.f, 1.f);
    uint3 vIndex = uint3(DTID.xy, 0);
    
    for (uint iSlice = 0; iSlice < vFroxelSize.z; ++iSlice)
    {
        vIndex.z = iSlice;
        
        float4 vLighting = VFLightTexture.Load(int4(vIndex, 0));
   
        uint iNextSlice = clamp(iSlice + 1, 0, vFroxelSize.z - 1);
        
        float fTickness = ComputeSliceDepth(iNextSlice, iSliceCount, fFogNear, fFogFar) - ComputeSliceDepth(iSlice, iSliceCount, fFogNear, fFogFar);
        
        Accum = ScatterStep(Accum.xyz, Accum.a, vLighting.xyz, vLighting.a, fTickness);
       
        OutputTexture[vIndex] = Accum;
    }
}