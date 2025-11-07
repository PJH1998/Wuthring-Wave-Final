#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

Texture2D<float> InputTexture : register(t0);
RWTexture2D<float> OutputTexture : register(u0);

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void Occlusion_Culling(uint3 DTID : SV_DispatchThreadID)
{

}
