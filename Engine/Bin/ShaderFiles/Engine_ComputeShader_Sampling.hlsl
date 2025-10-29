#include "Engine_ComputeShader_Function.hlsli"
typedef row_major matrix matrix_rm;

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

const static float g_fLuminence[3] = { 0.2126, 0.7152, 0.0722 };

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

Texture2D<float4> BaseTexture : register(t1);

cbuffer SIZE_DATA : register(b0)
{
    float2 fOutSize;
    float2 Paddingblur;
}

[numthreads(16, 16, 1)]
void UpSample(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    
    float2 fTexcoord = float2(DTID.xy) / fOutSize;
    float2 fFrac = fmod(fTexcoord * 0.5f * fOutSize, 1.f);
    
    int2 iLowID = int2(fTexcoord * 0.5f * fOutSize);
    
    
    float4 vColor = 0.f;
       
    float4 vLT = InputTexture.Load(int3(iLowID.x, iLowID.y, 0));
    float4 vRT = InputTexture.Load(int3(iLowID.x + 1, iLowID.y, 0));
    float4 vLB = InputTexture.Load(int3(iLowID.x, iLowID.y + 1, 0));
    float4 vRB = InputTexture.Load(int3(iLowID.x + 1, iLowID.y + 1, 0));
    
    vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
    
    float4 vBaseColor = BaseTexture.Load(int3(DTID.xy, 0));
    
    //float4 vFinalColor = vBaseColor + (vColor);
    float4 vFinalColor = 1.f - exp(-(vBaseColor + vColor * 2.f));
    vFinalColor.a = vColor.a;
    
    OutputTexture[DTID.xy] = vFinalColor;
}