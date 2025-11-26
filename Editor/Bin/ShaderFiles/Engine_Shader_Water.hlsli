#include "Engine_Shader_Function.hlsli"
#include "Engine_Shader_Defines.hlsli"

float g_fMinStepSize;
float g_fMaxStepSize;
float g_fStartOffset;

float g_fMaxDepth;

float g_fMinTickness;
float g_fMaxTickness;

struct ENV_MAP
{
    uint iIndex;
    float fRange;
    float Padding[2];
    float4 vPosition;
};

uint g_iNumEnvMaps;
bool g_HasEnvMap;
StructuredBuffer<ENV_MAP> g_EnvMapDatas : register(t3);
TextureCube g_EnvMapTexture[8] : register(t4);

float4 Compute_Reflect(float4 vWorldPos, float4 vViewPos, float4 vViewNormal, float4 vOriginColor, Texture2D<float4> SceneTexture, Texture2D<float4> DepthTexture)
{
    float4 vColor = 0.f;

    if (vViewPos.z == 0.f || g_iStep <= 0 || g_fMaxDistance <= g_fStartOffset)
    {
        vColor = vOriginColor;
        
        return vColor;
    }
    
    float4 vLook = normalize(float4(vViewPos.xyz, 0.f));
    
    float4 vReflect = normalize(float4(reflect(vLook.xyz, vViewNormal.xyz), 0.f));
    
    float4 vReflectColor = 0.f;

    bool IsHit = false;
    
    float fOffsetSize = g_fStartOffset;
    
    [unroll]
    for (int i = 0; i < g_iStep && fOffsetSize < g_fMaxDistance; ++i)
    {
        float4 vLay = vViewPos + float4((vReflect.xyz * fOffsetSize), 0.f);
       
        float4 vProjPos = mul(vLay, g_CamProjMatrix);
        
        vProjPos /= vProjPos.w;

        if (false == IsInNDC(vProjPos))
            break;
        
        float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
        
        float fDepth = DepthTexture.Sample(DefaultSampler, vTexcoord).y;
                             
        if (fDepth <= vLay.z || fDepth == 0.f)
        {
            IsHit = true;
            vReflectColor = SceneTexture.Sample(DefaultSampler, vTexcoord);
            break;
        }
        
        float fOffsetRatio = saturate(i / g_iStep);
        
        fOffsetSize += lerp(g_fMinStepSize, g_fMaxStepSize, fOffsetRatio);
    }
    
    float fMinDistance = 10000.f;
    
    float4 vEnvColor = 0.f;
    
    uint iIndex = 0;
    uint iSampleCount = clamp(g_iNumEnvMaps, 0, 8);
    
    for (uint j = 0; j < iSampleCount; ++j)
    {
        ENV_MAP Envmap = g_EnvMapDatas[j];
    
        float fLength = length(vWorldPos - Envmap.vPosition);
    
        if (Envmap.fRange >= fLength && fMinDistance > fLength)
        {
            iIndex = j;
            
            float3 vWorldReflect = normalize(mul(vReflect, g_ViewMatrixInv).xyz);
          
            vEnvColor = g_EnvMapTexture[iIndex].Sample(DefaultSampler, vWorldReflect);
            
            fMinDistance = fLength;
        }
    }
    
    if(iSampleCount > 0)
    {
        if (IsHit)
        {
            vReflectColor = lerp(vReflectColor, vEnvColor, 0.5f);
        }
        else
        {
            vReflectColor = vEnvColor;
        }
    }

    vColor = float4(vReflectColor.xyz, 1.f);
    //float4(lerp(vOriginColor.xyz, vReflectColor.xyz, 0.5f), 1.f);
      
    return vColor;
}

float4 Compute_Refract(float4 vWolrdPos, float4 vNormal, float4 vWaterColor, Texture2D<float4> SceneTexture, float fWaterDepth)
{
    float4 vColor = 0.f;
    
    float4 vLook = normalize(vWolrdPos - g_vCamPosition);
   
    float Eta = 1.f / 1.3f;
    
    float3 vRefract = refract(vLook.xyz, vNormal.xyz, Eta);
    
    float fDepthRatio = saturate(fWaterDepth / g_fMaxDepth);
    
    float fTickness = lerp(g_fMinTickness, g_fMaxTickness, fDepthRatio);
    
    float4 vRefractWorldPos = float4(vWolrdPos.xyz + (vRefract * fTickness), 1.f);
    
    float4x4 matVP = mul(g_CamViewMatrix, g_CamProjMatrix);
    
    float4 vRefractProjPos = mul(vRefractWorldPos, matVP);
    
    float2 vProjXY = vRefractProjPos.xy / vRefractProjPos.w;
    
    float2 vUV = Compute_Texcoord(vProjXY);
    
    float4 vRefractvColor = SceneTexture.Sample(DefaultSampler, vUV);
    
    vColor = lerp(vRefractvColor, vWaterColor, fDepthRatio);
    
    return vColor;
}