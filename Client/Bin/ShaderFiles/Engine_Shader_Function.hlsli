#include "Engine_Shader_State.hlsli"

static float PI = 3.1415926535f;

// Emissive효과를 넣을지 판단할 때 사용하는 RGB 계수
float g_fLuminence[3] = { 0.2126, 0.7152, 0.0722 };

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_CamViewMatrix, g_CamProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
float g_fFar;
vector g_vCamPosition;

float g_fWidth = 1920.f;
float g_fHeight = 1080.f;

float g_iShadowMapSizeX = 8192;
float g_iShadowMapSizeY = 4608;

float Compute_NDF(float NdotH, float Roughness) // ThrowBridgeReitzNormalDistribution   , 미세면 표면의 거칠기 분포
{
    float RoughnessSqr = pow(Roughness, 2.f);                       
    float Distribution = NdotH * NdotH * (RoughnessSqr - 1.f) + 1.f; // 내적(노말, 반사) * 내적(노말, 반사) * ( 거칠기 - 1.f ) + 1.f 
    
    float NDF = RoughnessSqr / (PI * Distribution * Distribution); 
    
    return NDF;
}

float Compute_GSF(float NdotL, float NdotV, float Roughness) // SchlickGGXGeometricShadowingFunction    , 미세면끼리의 자기 그림자
{
    float k = Roughness / 2.f;
    
    float SmithL = (NdotL) / (NdotL * (1.f - k) + k);
    float SmithV = (NdotV) / (NdotV * (1.f - k) + k);
    
    float GS = (SmithL * SmithV);
    
    return GS;
}

float SchlickFresnel(float i)
{
    float x = clamp(1.f - i, 0.f, 1.f);
    
    return pow(x, 5.f);
}

float3 Compute_Fresnel(float3 vSpecularColor, float LdotH) // SchlickFresnelFunction    , 입사각에 따른 반사되는 비율
{
    return vSpecularColor + (float3(1.f, 1.f, 1.f) - vSpecularColor) * SchlickFresnel(LdotH);
}

float3 Compute_BRDF_PBR(float3 vNormal, float3 vViewDir, float3 vLightDir, float3 vAlbedo, float fMetallic, float fRoughness) // vViewDir = Look (WorldPos - CamPos)
{
    float3 vHalf = normalize(vViewDir + vLightDir);
    float NdotL = saturate(dot(vNormal, vLightDir));
    float NdotV = saturate(dot(vNormal, vViewDir));
    float NdotH = saturate(dot(vNormal, vHalf));
    float LdotH = saturate(dot(vLightDir, vHalf));
    
    float3 vF0 = 0.04f;
    vF0 = lerp(vF0, vAlbedo, fMetallic);
    
    float3 Fresnel = Compute_Fresnel(vF0, LdotH);
        
    float GSF = Compute_GSF(NdotL, NdotV, fRoughness);
    
    float NDF = Compute_NDF(NdotH, fRoughness);
    
    float3 Specular = (NDF * GSF * Fresnel) / max(4.f * NdotL * NdotV, 0.001f);
    
    float3 kd = (1.f - Fresnel) * (1.f - fMetallic);
    
    float3 vDiffuse = kd * vAlbedo / PI;
    
    return (vDiffuse + Specular) * NdotL;
}

float3 Compute_Stylized_PBR(float3 vNormal, float3 vViewDir, float3 vLightDir, float3 vAlbedo, float fMetallic, float fRoughness, float4 vToonRim)
{
    float3 vHalf = normalize(vViewDir + vLightDir);
    float NdotL = saturate(dot(vNormal, vLightDir));
    float NdotV = saturate(dot(vNormal, vViewDir));
    float NdotH = saturate(dot(vNormal, vHalf));
    float LdotH = saturate(dot(vLightDir, vHalf));
    
    float3 vF0 = 0.04f;
    vF0 = lerp(vF0, vAlbedo, fMetallic);
    
    float3 Fresnel = Compute_Fresnel(vF0, LdotH);
        
    float GSF = Compute_GSF(NdotL, NdotV, fRoughness);
    
    float NDF = Compute_NDF(NdotH, fRoughness);
    
    float3 Specular = (NDF * GSF * Fresnel) / max(4.f * NdotL * NdotV, 0.001f);
    
    float3 kd = (1.f - Fresnel) * (1.f - fMetallic);
    
    float3 vDiffuse = kd * vAlbedo / PI;
    
    vDiffuse *= vToonRim.x;
    
    return (vDiffuse + Specular) * NdotL;
}

float2 Compute_Texcoord(float2 vProjXY)
{
    float2 vTexcoord = 0.f;
    
    vTexcoord.x = vProjXY.x * 0.5f + 0.5f;
    vTexcoord.y = vProjXY.y * -0.5f + 0.5f;
        
    return vTexcoord;
}

float4 Compute_WorldPos(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vWorldPos = 0.f;

    
    vector vDepthDesc = DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    vWorldPos.x = vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= vDepthDesc.y;
    
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    return vWorldPos;
}

float4 Compute_ViewPos(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vViewPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos = vViewPos * vDepthDesc.y;
    vViewPos = mul(vViewPos, g_ProjMatrixInv);
    
    return vViewPos;
}

float4 Compute_Normal(Texture2D NormalTexture, sampler Sampler, float2 vTexcoord)
{
    float4 vNormal = NormalTexture.Sample(Sampler, vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    
    return vNormal;
}

float Luminame(float3 vColor)
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}

float ShadowPCF(float3 UVDepth, int iCascadeIndex, int iNumWeight, Texture2DArray<float> ShadowMap)
{
    float2 vTexelSize = float2((1.f / g_iShadowMapSizeX), (1.f / g_iShadowMapSizeY));
    float fShadow = 0.f;
    
    [unroll]
    for (int x = -iNumWeight; x <= iNumWeight; ++x)
    {
        [unroll]
        for (int y = -iNumWeight; y <= iNumWeight; ++y)
        {
            float2 vOffset = float2(x, y) * vTexelSize;
            
            fShadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(UVDepth.xy + vOffset, iCascadeIndex), UVDepth.z);
        }
    }
    
    fShadow = fShadow / pow((iNumWeight * 2 + 1), 2);
    
    return fShadow;
}

float RPB_Gradiant(float fViewDepth)
{
    float DepthDDX = ddx(fViewDepth * 0.0001f);
    float DepthDDY = ddy(fViewDepth * 0.0001f);
    
    float GradiantX = abs(DepthDDX);
    float GradiantY = abs(DepthDDY);
    
    float Gradiant = length(float2(GradiantX, GradiantY));
   
    return Gradiant;
}

bool Outline(sampler Sampler, float2 UV, float fCompareDepth, float fWeight, Texture2D DepthTexture)
{
    float2 vTexelSize = float2((1.f / g_fWidth), (1.f / g_fHeight));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = UV + vOffset;
            vector DepthDesc = DepthTexture.Sample(Sampler, UV + vOffset);
            
            if(DepthDesc.x == 1.f)
                return true;
                
            vector vWorldPos;
            
            vWorldPos.x = vTexcoord.x * 2.f - 1.f;
            vWorldPos.y = vTexcoord.y * -2.f + 1.f;
            vWorldPos.z = DepthDesc.x;
            vWorldPos.w = 1.f;
    
            vWorldPos *= DepthDesc.y;

            vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
                    
            if (abs(fCompareDepth - vWorldPos.z) >= fWeight)
                return true;
        }
    } 
        
    return false;
}


bool Outline_Normal(sampler Sampler, float2 vUV, float3 vCompareNormal, float fWeightRadians, Texture2D NormalTexture)
{
    float2 vTexelSize = float2((1.f / g_fWidth), (1.f / g_fHeight));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = vUV + vOffset;
            float3 NormalDesc = NormalTexture.Sample(Sampler, vUV + vOffset).xyz;
            float3 vNormal = normalize(vector(NormalDesc.xyz * 2.f - 1.f, 0.f));
   
            if (dot(vNormal, vCompareNormal) <= fWeightRadians)
                return true;
        }
    }
        
    return false;
}

float SSAO_Factor(vector vSampleNormal, vector vNoiseVector, vector vViewNormal, vector vViewPos, float fRadius, float fMaxDistance, Texture2D DepthTexture)
{
    float Occlusion = 0.f;
    
    float3 vTangent = normalize(vNoiseVector.xyz - (vViewNormal.xyz * dot(vNoiseVector, vViewNormal)));
    float3 vNormal = vViewNormal.xyz;
    float3 vBinormal = cross(vTangent, vNormal);
    
    float3x3 TBN = float3x3(vTangent, vBinormal, vNormal);
    
    vector vSamplePos = vViewPos + vector((mul(vSampleNormal.xyz, TBN) * fRadius), 0.f);
    
    vector vProjPos = mul(vSamplePos, g_CamProjMatrix);
    float fRandomZ = vProjPos.w;
    
    float2 vSampleUV = Compute_Texcoord((vProjPos.xy / vProjPos.w));
    
    float SampleDepth = DepthTexture.Sample(PointClampSampler, vSampleUV).y;
    
    if (SampleDepth == 0.f || SampleDepth >= fRandomZ) // 안그려져있거나, 랜덤 위치보다 뒤에 있다면
        return 1.f;
    
    float Distance = abs(SampleDepth - vViewPos.z);
    
    Occlusion = smoothstep(0.f, fMaxDistance, Distance);
    
    float fNormalWeight = saturate(dot(vViewNormal, normalize(vViewPos - vSamplePos)));
    
    Occlusion *= fNormalWeight;
    
    return Occlusion;
}

float Random(float2 St)
{
    return frac(sin(dot(St.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

float Noise(float2 St)
{
    float2 i = floor(St);
    float2 f = frac(St);
    
    float a = Random(i);
    float b = Random(i + float2(1.0, 0.0));
    float c = Random(i + float2(0.0, 1.0));
    float d = Random(i + float2(1.0, 1.0));
    
    float2 u = f * f * (3.0 - 2.0 * f);
    
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}
