#include "Engine_ComputeShader_Function.hlsli"
typedef row_major matrix matrix_rm;

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);
SamplerState DefaultSampler : register(s0);

[numthreads(16, 16, 1)]
void DownSample(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    int iIndexX = DTID.x * 2;
    int iIndexY = DTID.y * 2;
    
    float4 vColor = 0.f;
       
    vColor += InputTexture.Load(int3(iIndexX, iIndexY, 0));
    vColor += InputTexture.Load(int3(iIndexX + 1, iIndexY, 0));
    vColor += InputTexture.Load(int3(iIndexX, iIndexY + 1, 0));
    vColor += InputTexture.Load(int3(iIndexX + 1, iIndexY + 1, 0));
    
    vColor /= 4.f;
    
    OutputTexture[DTID.xy] = vColor;
}

