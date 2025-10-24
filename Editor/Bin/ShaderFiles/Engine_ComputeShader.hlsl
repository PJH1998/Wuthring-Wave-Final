#include "Engine_ComputeShader_Function.hlsli"
typedef row_major matrix matrix_rm;

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

Texture2D<float4> g_NormalTexture : register(t1);
Texture2D<float4> g_DepthTexture : register(t2);
Texture2D<float4> g_NoiseTexture : register(t3);

SamplerState DefaultSampler : register(s1);
SamplerState PointClampSampler : register(s2);
SamplerState NoiseSampler : register(s3);

cbuffer SSAO_DATA : register(b0)
{
    vector vSampleVector[16];
    matrix_rm CamViewMatrix;
    matrix_rm CamProjMatrix;
    matrix_rm ProjMatrixInv;
    float fWidth;
    float fHeight;
    int iSampleSize;
    float fSSAO_Radius;
    float fSSAO_MaxDistance;
    float3 Padding;
}

[numthreads(8, 8, 1)]
void SSAO(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float2 vTexcoord = Compute_Texcoord(DTID, fWidth, fHeight);
    
    float4 vViewPos = Compute_ViewPos(vTexcoord, g_DepthTexture, DTID, ProjMatrixInv);
    
    float vViewPosZ = vViewPos.z;
        
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, vTexcoord);
    vNormal = mul(vNormal, CamViewMatrix);
    
    float2 vNoiseTexcoord = frac(vTexcoord * 4.f);
    vector vNoiseNormal = g_NoiseTexture.SampleLevel(NoiseSampler, vNoiseTexcoord, 0);
    
    float TotalOcclusion = 0.f;
    
    [unroll]
    for (int i = 0; i < iSampleSize; ++i)
    {
        float Occlusion = 0.f;
    
        float3 vTangent = normalize(vNoiseNormal.xyz - (vNormal.xyz * dot(vNoiseNormal, vNormal)));
        float3 vTBNNormal = vNormal.xyz;
        float3 vBinormal = cross(vTangent, vTBNNormal);
  
        float3x3 TBN = float3x3(vTangent, vBinormal, vTBNNormal);
        
        vector vSamplePos = vViewPos + vector((mul(vSampleVector[i].xyz, TBN) * fSSAO_Radius), 0.f);
        vSamplePos.w = 1.f;
        
        vector vProjPos = mul(vSamplePos, CamProjMatrix);
        float fRandomZ = vProjPos.w;
    
        float2 vProjPosXY = vProjPos.xy / vProjPos.w;
        
        float2 vSampleTexcoord = Compute_Texcoord_Proj(vProjPosXY);
            
        float SampleDepth = g_DepthTexture.SampleLevel(PointClampSampler, vSampleTexcoord, 0).y;
            
        if (SampleDepth == 0.f  || SampleDepth >= vViewPosZ)
        {
            TotalOcclusion += 1.f;
            continue; // 안그려져있거나, 뷰 위치보다 뒤에 있다면
        }
            
        float Distance = abs(SampleDepth - vViewPosZ);
        //Distance *= 0.1f;
        
        Occlusion = smoothstep(0.f, fSSAO_MaxDistance, Distance);
        
        //float fNormalWeight = saturate(dot(vNormal, normalize(vSamplePos - fRandomZ)));
        
        //Occlusion *= (fNormalWeight);
        
        TotalOcclusion += (1.f - Occlusion);
    }
    
    TotalOcclusion = (TotalOcclusion / (float) iSampleSize);
    
    OutputTexture[DTID.xy] = float4(TotalOcclusion, TotalOcclusion, TotalOcclusion, 1.f);
}