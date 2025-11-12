#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 256
#define THREAD_Y 1
#define THREAD_Z 1

#define RADIUS_THRESHOLD 5
#define MAX_DEPTH 10

struct BoxPoint
{
    float4 vCorners[8];
    float3 vCenter;
    float fRadius;
};

StructuredBuffer<BoxPoint> g_BoxPoints : register(t0);
Texture2D<float> InputTexture : register(t1);
RWStructuredBuffer<uint> OutputTexture : register(u0);

cbuffer OCDesc : register(b0)
{
    float4x4 g_ProjMatrix;
    uint g_iNumObjects;
    uint g_iHZBMipLevel;
    float2 padding;
};

float2 WorldToScreen_Center(float3 vCenter)
{
    float2 vTexcoord = 0.f;
    
    vector vProjPos = mul(float4(vCenter, 1.f), g_ProjMatrix);
    float2 vNDC = vProjPos.xy / vProjPos.w;
    
    vTexcoord.x = vNDC.x * 0.5f + 0.5f;
    vTexcoord.y = vNDC.y * -0.5f + 0.5f;
    
    return vTexcoord;
}

float2 WorldToScreen_Corner(float4 vCorner)
{
    float2 vTexcoord = 0.f;
    
    vector vProjPos = mul(vCorner, g_ProjMatrix);
    float2 vNDC = vProjPos.xy / vProjPos.w;
    
    vTexcoord.x = vNDC.x * 0.5f + 0.5f;
    vTexcoord.y = vNDC.y * -0.5f + 0.5f;
    
    return vTexcoord;
}

bool CheckOC(BoxPoint Box)
{
    // Small -> Center + Radius
    if(Box.fRadius < RADIUS_THRESHOLD)
    {
        int iMipLevel = clamp(g_iHZBMipLevel + 3, 0, MAX_DEPTH);
        int2 vSize = max(int2(1920, 1080) >> (iMipLevel + 1), int2(1, 1));

        float2 vTexcoord = WorldToScreen_Center(Box.vCenter);
        float fBoxDepth = Box.vCenter.z;
        
        int2 px = int2(saturate(vTexcoord) * (vSize - 1));
        float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        if (0.f == fHZBDepth)
            return true;
        
        float eps = max(10.f, fBoxDepth * 0.01f);
        if (fBoxDepth < fHZBDepth + Box.fRadius + eps)
            return true;
        return false;
    }
    // Big >> Corner + Center
    else
    {
        int iMipLevel = clamp(g_iHZBMipLevel, 0, MAX_DEPTH);
        int2 vSize = max(int2(1920, 1080) >> (iMipLevel + 1), int2(1, 1));
        
        // Center
        float2 vTexcoord = WorldToScreen_Center(Box.vCenter);
        float fBoxDepth = Box.vCenter.z;
        
        int2 px = int2(saturate(vTexcoord) * (vSize - 1));
        float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        if (0.f == fHZBDepth)
            return true;
        
        float eps = max(10.f, fBoxDepth * 0.01f);
        if (fBoxDepth < fHZBDepth + Box.fRadius + eps)
            return true;

        // Corner
        for (int i = 0; i < 8; ++i)
        {
            int iOffsetX = i & 1 ? 1 : -1;
            int iOffsetY = i & 2 ? 1 : -1;
            vTexcoord = WorldToScreen_Corner(Box.vCorners[i]);
            float fCornerDepth = Box.vCorners[i].z;
        
            px = int2(saturate(vTexcoord) * (vSize - 1));
            px += int2(iOffsetX, iOffsetY);
            fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
            if (0.f == fHZBDepth)
                return true;
        
            eps = max(50.f, fCornerDepth * 0.05f);
            if (fCornerDepth < fHZBDepth + eps)
                return true;
        }
        return false;
    }
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void Occlusion_Culling(uint3 DTID : SV_DispatchThreadID)
{
    uint iIndex = DTID.x;

    if (iIndex >= g_iNumObjects)
        return;
    
   BoxPoint box = g_BoxPoints[iIndex];
   bool isVisible = CheckOC(box);

   OutputTexture[iIndex] = isVisible ? 1 : 0;
}
