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
    if(vViewPos.z == 0.f)
    {
        OutputTexture[DTID.xy] = float4(1.f, 1.f, 1.f, 1.f);
        return;
    }   
        
    float fViewPosZ = vViewPos.z;
        
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, vTexcoord);

    vNormal = vector(normalize(vNormal.xyz), 0.f);
    vNormal = mul(vNormal, CamViewMatrix);
    
    float2 vNoiseScale = float2(fWidth / 512.f,  fHeight / 512.f);
    
    float2 vNoiseTexcoord = vTexcoord * vNoiseScale * 4.f;
    float4 vNoiseNormal = g_NoiseTexture.SampleLevel(NoiseSampler, vNoiseTexcoord, 0);
    float2 vNoiseXY = vNoiseNormal.xy * 2.f - 1.f;
    vNoiseNormal = float4(normalize(float3(vNoiseXY, vNoiseNormal.z)), 0.f);
    vNoiseNormal = mul(vNoiseNormal, CamViewMatrix);
    
    float TotalOcclusion = 0.f;

    [unroll]
    for (int i = 0; i < iSampleSize; ++i)
    {
        float Occlusion = 0.f;
    
        float3 vTangent = normalize(vNoiseNormal.xyz - (vNormal.xyz * dot(vNoiseNormal, vNormal)));
        float3 vBinormal = normalize(cross(vTangent, vNormal.xyz));
  
        float3x3 TBN = float3x3(vTangent, vBinormal, vNormal.xyz);
        
        vector vSampeDir = vector(mul(vSampleVector[i].xyz, TBN), 0.f);
        
        vector vSamplePos = vViewPos + (vSampeDir * fSSAO_Radius);
        vSamplePos.w = 1.f;
        float fRandomZ = vSamplePos.z;
        
        vector vProjPos = mul(vSamplePos, CamProjMatrix);
    
        float2 vProjPosXY = vProjPos.xy / vProjPos.w;
        
        float2 vSampleTexcoord = Compute_Texcoord_Proj(vProjPosXY);
            
        float SampleDepth = g_DepthTexture.SampleLevel(PointClampSampler, vSampleTexcoord, 0).y;
        
        if (SampleDepth == 0.f || SampleDepth >= fRandomZ)// 안그려져있거나, 뷰 위치보다 뒤에 있다면
        {
            Occlusion = 1.f;
        }
        else
        {
            float Distance = SampleDepth - fViewPosZ;
            
            if(Distance >= 10.f)
            {
                Occlusion = 1.f;
            }
            else
            {
                Occlusion = smoothstep(fSSAO_MaxDistance, 0.f, Distance);
            
                vector vSampleViewPos = Compute_ViewPosTexcoord(vSampleTexcoord, g_DepthTexture, DefaultSampler, ProjMatrixInv);
            
                float fNormalWeight = saturate(dot(vNormal.xyz, normalize(vSampleViewPos.xyz - vViewPos.xyz)));         // 현재 노말과 Sample 위치까지의 방향 벡터
                
                Occlusion *= (1.f - fNormalWeight);                 // 수직에 가까울수록 ( 깊이 차이가 클수록 더 어둡게, 작으면 거의 그대로 )
            }
        }
        
        TotalOcclusion += Occlusion;

    }
    
    TotalOcclusion = (TotalOcclusion / (float) iSampleSize);
        
    OutputTexture[DTID.xy] = float4(TotalOcclusion, TotalOcclusion, TotalOcclusion, 1.f);
}