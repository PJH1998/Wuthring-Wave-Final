#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 1

Texture2D<float4> InputTexture : register(t0);
Texture2D<float> InputMipTexture : register(t1);
RWTexture2D<float> OutputTexture : register(u0);

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void HZB(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    int iIndexX = DTID.x * 2;
    int iIndexY = DTID.y * 2;
    
    float fFinalDepth = 0.f;
    
    int2 InSize = 0;
    InputTexture.GetDimensions(InSize.x, InSize.y);
    if(0 == InSize.x)
        InputMipTexture.GetDimensions(InSize.x, InSize.y);
    
    int iSampleX0 = min(iIndexX, InSize.x - 1);
    int iSampleX1 = min(iIndexX + 1, InSize.x - 1);
    
    int iSampleY0 = min(iIndexY, InSize.y - 1);
    int iSampleY1 = min(iIndexY + 1, InSize.y - 1);
    
    if (InSize.x == 1920)
        fFinalDepth = max(
        max(
        max(InputTexture.Load(int3(iSampleX0, iSampleY0, 0)).y, 
        InputTexture.Load(int3(iSampleX1, iSampleY0, 0)).y), 
        InputTexture.Load(int3(iSampleX1, iSampleY1, 0)).y), 
        InputTexture.Load(int3(iSampleX0, iSampleY1, 0)).y);
    else
        fFinalDepth = max(
        max(
        max(InputMipTexture.Load(int3(iSampleX0, iSampleY0, 0)),
        InputMipTexture.Load(int3(iSampleX1, iSampleY0, 0))),
        InputMipTexture.Load(int3(iSampleX1, iSampleY1, 0))),
        InputMipTexture.Load(int3(iSampleX0, iSampleY1, 0)));
    
    OutputTexture[DTID.xy] = fFinalDepth;
}
