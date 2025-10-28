#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define BLUR_RADIUS 6

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

cbuffer SIZE_DATA : register(b0)
{
    float2 fOutSize;
    float2 Paddingblur;
}

static float g_fGaussianWeights[13] =
{
    0.020597f, 0.037981f, 0.062950f, 0.093995f, 0.117324f, 0.153170f, 0.163967f, 0.153170f, 0.117324f, 0.093995f, 0.062950f, 0.037981f, 0.020597f
};

//static const float g_fGaussianWeights[21] =
//{
//    0.000012f, 0.000067f, 0.000314f, 0.001188f, 0.003661f, 0.009310f, 0.020597f, 0.037981f, 0.057783f, 0.073649f, 0.080657f, 
//    0.073649f, 0.057783f, 0.037981f, 0.020597f, 0.009310f, 0.003661f, 0.001188f, 0.000314f, 0.000067f, 0.000012f
//};

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_X(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) BLUR_RADIUS;
    
    vSharedColorX[GTID.y][GTID.x + BLUR_RADIUS] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x - BLUR_RADIUS, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= (int) fOutSize.x)
            RightID.x = (int) fOutSize.x - 1;
            
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
    
    OutputTexture[DTID.xy] = vColorX;
}

groupshared float4 vSharedColorY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_Y(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) BLUR_RADIUS;
    
    vSharedColorY[GTID.y + BLUR_RADIUS][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.y < BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - BLUR_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= (int) fOutSize.y)
            RightID.y = (int) fOutSize.y - 1;
            
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
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