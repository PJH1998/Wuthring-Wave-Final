#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 1

#define BLUR_RADIUS 6

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

float g_fGaussianWeights[13] =
{
    0.020597f, 0.037981f, 0.062950f, 0.093995f, 0.127324f, 0.153170f, 0.163967f, 0.153170f, 0.127324f, 0.093995f, 0.062950f, 0.037981f, 0.020597f
};

cbuffer BLUR_DATA : register(b0)
{
    float fWidth;
    float fHeight;
    float Paddingblur;
}

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];
groupshared float4 vSharedColorY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) BLUR_RADIUS;
    
    vSharedColorX[GTID.y][GTID.x + BLUR_RADIUS] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x - BLUR_RADIUS, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= fWidth)
            RightID.x = fWidth - 1.f;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorX[GTID.y][GTID.x + THREAD_X + BLUR_RADIUS] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float4 vColorX = 0.f;
    
    for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; ++i)
    {
        int iIndexX = GTID.x + BLUR_RADIUS + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        
        vColorX += vSampleColor * g_fGaussianWeights[i + BLUR_RADIUS];
    }
    
    vSharedColorY[GTID.y + BLUR_RADIUS][GTID.x] = vColorX;
    
    if (GTID.y < BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - BLUR_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= fHeight)
            RightID.y = fHeight - 1.f;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorY[GTID.y + THREAD_Y + BLUR_RADIUS][GTID.x] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vColorY = 0.f;
    
    for (int j = -BLUR_RADIUS; j <= BLUR_RADIUS; ++j)
    {
        int iIndexY = GTID.y + BLUR_RADIUS + j;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
        
        vColorY += vSampleColor * g_fGaussianWeights[j + BLUR_RADIUS];
    }
    
    OutputTexture[DTID.xy] = vColorY;
}

//[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
//void SSAO_BLUR(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
//{
//    float fBlurRadius = (float) SSAO_BLUR_RADIUS;

//    GroupMemoryBarrierWithGroupSync();
    
//    float4 vColorX = 0.f;
    
//    float4 vOriginColorX = vSharedColorX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
//    float fOriginDepthX = vSharedDepthX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
//    float4 vOriginNormalX = vSharedNormalX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    
//    for (int i = -SSAO_BLUR_RADIUS; i <= SSAO_BLUR_RADIUS; ++i)
//    {
//        int iIndexX = GTID.x + SSAO_BLUR_RADIUS + i;
        
//        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
//        float fSampleDepth = vSharedDepthX[GTID.y][iIndexX];
//        float4 vSampleNormal = vSharedNormalX[GTID.y][iIndexX];
        
//        vColorX += Compute_SSAO_Blur(vOriginColorX, fOriginDepthX, vOriginNormalX, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance);
//    }
    
//    vColorX /= (fBlurRadius * 2.f + 1.f);
    
//    vSharedColorY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = vColorX;
//    vSharedDepthY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = fOriginDepthX;
//    vSharedNormalY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = vOriginNormalX;
    
//    if (GTID.y < SSAO_BLUR_RADIUS)
//    {
//        int3 LeftID = int3(DTID.x, DTID.y - SSAO_BLUR_RADIUS, 0);
//        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
//        if (LeftID.y < 0)
//            LeftID.y = 0;
        
//        if (RightID.y >= fHeight_Blur)
//            RightID.y = fHeight_Blur -1.f;
        
        
//        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
//        vSharedDepthY[GTID.y][GTID.x] = g_DepthTexture.Load(LeftID).y;
//        vSharedNormalY[GTID.y][GTID.x] = Compute_Normal_DTID(g_NormalTexture, LeftID);
        
//        vSharedColorY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = InputTexture.Load(RightID);
//        vSharedDepthY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = g_DepthTexture.Load(RightID).y;
//        vSharedNormalY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = Compute_Normal_DTID(g_NormalTexture, RightID);
//    }
    
//    GroupMemoryBarrierWithGroupSync();
    
//    float4 vOriginColorY = vSharedColorY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
//    float fOriginDepthY = vSharedDepthY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
//    float4 vOriginNormalY = vSharedNormalY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
    
//    float4 vColorY = 0.f;
    
//    for (int j = -SSAO_BLUR_RADIUS; j <= SSAO_BLUR_RADIUS; ++j)
//    {
//        int iIndexY = GTID.y + SSAO_BLUR_RADIUS + j;
        
//        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
//        float fSampleDepth = vSharedDepthY[iIndexY][GTID.x];
//        float4 vSampleNormal = vSharedNormalY[iIndexY][GTID.x];
        
//        vColorY += Compute_SSAO_Blur(vOriginColorY, fOriginDepthY, vOriginNormalY, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance);
//    }
    
//    vColorY /= (fBlurRadius * 2.f + 1.f);
    
//    OutputTexture[DTID.xy] = vColorY;
//}