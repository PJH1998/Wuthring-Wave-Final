#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 4

Texture2D<float4> DepthTexture : register(t0);
RWTexture3D<float> OutputTexture : register(u0);

cbuffer VF_Data : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    
};

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void VolumetricFog(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    int iIndexX = DTID.x * 2;
    int iIndexY = DTID.y * 2;
    
    float fFinalDepth = 0.f;

}
