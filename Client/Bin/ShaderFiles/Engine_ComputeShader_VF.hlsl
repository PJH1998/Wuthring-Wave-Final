#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 8

#define MAX_MIPLEVEL 10
#define MIN_MIPLEVEL 3

#define EPSILON 1e-05

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
Texture2D<float4> g_OriginDepthTexture : register(t2);
RWTexture3D<float4> OutputTexture : register(u0);

SamplerState CS_DefaultSampler : register(s1);

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
};

float ComputeSliceDepth(uint iSlice, uint iSliceCount, float fFogNear, float fFogFar)
{
    return fFogNear * pow(fFogFar / fFogNear, (float) iSlice / (float(iSliceCount - 1)));
}

float ComputeDepthToProjZ(float fViewZ, float fNear, float fFar)
{
    return ((fFar * fViewZ) / (fFar - fNear)) - (fFar * fNear) / (fFar - fNear); // (f * z / f-n) - (f*n / (f - n)  = 투영 보정 (w 나누기 하기 전 )
}

float3 ComputeDirectional()
{


    return 1.f;
}

float3 ComputePoint(VF_Light Light, float4 vWorldPos)
{
    float fDist = length(Light.vPosition.xyz - vWorldPos.xyz);
    if (fDist > Light.fRange)
        return 0.f;
        
    return 1.f;
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

float ReturnMaxDepth(uint iMipLevel, uint2 DTID, float2 vScreenSize, float2 vFroxelSize)
{
    float fDepth = 0.f;
    
    uint FroxelScale = vScreenSize / vFroxelSize;
    
    uint iMipScale = 1 << iMipLevel;
    
    uint2 vIndex = (DTID.xy * FroxelScale) / iMipScale;
    
    float2 vTexcoord = (float2) vIndex + 0.5f * (iMipScale / vScreenSize);
    
    fDepth = g_MipDepthTexture.SampleLevel(CS_DefaultSampler, vTexcoord, (iMipLevel - 1));
    
    return fDepth;
}

float HenyeyGreensteinPhasefunction(float3 LightDir, float3 LightOutDir, float G)
{
    float cosTheta = dot(LightDir, LightOutDir);
    float G2 = pow(G, 2);
    float Denom = pow(1.f + G2 - 2.f * G * cosTheta, 3.f / 2.f);
    
    return (1.f / (4.f * PI)) * ((1.f - G2) / max(Denom, EPSILON));
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void VolumetricFog(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{    
    float fViewZ = ComputeSliceDepth(DTID.z, iSliceCount, fFogNear, fFogFar);
    
    float2 vUV = (float2) (DTID.xy + 0.5f) / float2(vFroxelSize.xy);
    
    float vNdcZ = ComputeDepthToProjZ(fViewZ, fCamNear, fCamFar) / fViewZ;
    
    float2 vNdcXY = float2(vUV.x * 2.f - 1.f, vUV.y * -2.f + 1.f);
   
    float4 vProjPos = float4(vNdcXY, vNdcZ, 1.f) * fViewZ;
    
    float4 vViewPos = mul(vProjPos, InvProjMatrix);
    
    float4 vWorldPos = mul(vViewPos, InvViewMatrix);
    
    
    uint iMipLevel = ComputeMipLevel(ProjMatrix, fViewZ, fScreenX, fScreenY);
    
    float fDepth = ReturnMaxDepth(iMipLevel, DTID.xy, float2(fScreenX, fScreenY), vFroxelSize.xy);
    
    float3 vOutDir = normalize(vViewPos.xyz * -1.f);
    
    float3 Lighting = 0.f;
    
    for (int i = 0; i < iLightCount; i++)
    {
        VF_Light Light = g_LightDatas[i];
        
        float3 LightDirection = 0.f;
        float fAtt = 1.f;
        
        switch (Light.iType)
        {
            case 0: // DIRECTIONAL
                LightDirection = normalize(Light.vDirection);
                break;
                
            case 1: // POINT
                LightDirection = normalize(vWorldPos.xyz - Light.vPosition.xyz);
                fAtt = saturate((Light.fRange - length(LightDirection)) / Light.fRange);
                break;
        }
        
        float PhaseFunction = HenyeyGreensteinPhasefunction(LightDirection, vOutDir, 0.5f);
        
        Lighting += (Light.vDiffuse.xyz) * fAtt * PhaseFunction;
    }
    
    float fDensity = 1.f; // 밀도
    
    OutputTexture[DTID.xyz] = float4(1.f,1.f,1.f, fDensity);
}
