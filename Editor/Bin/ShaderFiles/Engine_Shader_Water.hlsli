#include "Engine_Shader_Function.hlsli"
#include "Engine_Shader_Defines.hlsli"

float g_fMinStepSize;
float g_fMaxStepSize;
float g_fStartOffset;

float g_fMaxDepth;

float g_fMinTickness;
float g_fMaxTickness;

const static uint g_iBinaryStep = 6;
const static uint g_iStep = 24;
const static float g_fMaxDistance = 400.f;

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
    
    float2 vTexcoord = 0.f;
    
    float2 vHitRange = float2(0.f, 0.f);
    float fHitDepth = 0.f;
    
    //float Jitter = lerp(0.2f, 1.f, Hash13(vViewPos.xyz));
    
    //fOffsetSize *= Jitter;
    
    //for (int i = 0; i < g_iStep && fOffsetSize < g_fMaxDistance; ++i)
    //{   
    //    float4 vLay = vViewPos + float4((vReflect.xyz * fOffsetSize), 0.f);
       
    //    vHitRange.x = vHitRange.y;
    //    vHitRange.y = fOffsetSize;
       
    //    float4 vProjPos = mul(vLay, g_CamProjMatrix);
        
    //    vProjPos /= vProjPos.w;

    //    if (false == IsInNDC(vProjPos))
    //        break;
        
    //    vTexcoord = Compute_Texcoord(vProjPos.xy);
        
    //    float fDepth = DepthTexture.Sample(DefaultSampler, vTexcoord).y;
        
    //    if(fDepth <= 0.f)
    //        break; 
            
    //    if (fDepth <= vLay.z)//            +g_fMaxTickness)
    //    {
    //        IsHit = true;
    //        fHitDepth = fDepth;
    //        break;
    //    }
        
    //    float fOffsetRatio = saturate((float) i / (float) g_iStep);
        
    //    fOffsetSize += lerp(g_fMinStepSize, g_fMaxStepSize, fOffsetRatio);
    //}
    
    //float fDistWeight = 1.f;
    //float fStepDepth = 0.f;
    //float4 vEnvColor = 0.f;
    
    //if (IsHit && fHitDepth > 0.f)
    //{ 
    //    // binary Step
    //    for (uint i = 0; i < g_iBinaryStep; ++i)
    //    {
    //        float fBinaryOffset = (vHitRange.x + vHitRange.y) * 0.5f;
            
    //        float4 vBinaryLay = vViewPos + float4((vReflect.xyz * fBinaryOffset), 0.f);
            
    //        float4 vProjPos = mul(vBinaryLay, g_CamProjMatrix);
        
    //        vProjPos /= vProjPos.w;

    //        vTexcoord = Compute_Texcoord(vProjPos.xy);
        
    //        float fDepth = DepthTexture.Sample(DefaultSampler, vTexcoord).y;
            

    //        if (fDepth <= vBinaryLay.z)
    //            vHitRange.y = fBinaryOffset;
    //        else
    //            vHitRange.x = fBinaryOffset;
                
    //        fStepDepth = fDepth;
    //    }
        
    //    vReflectColor = SceneTexture.Sample(DefaultSampler, vTexcoord);
        
    //    fDistWeight = saturate(fStepDepth / g_fMaxDistance);
    //}
    ////else
    ////{
    ////    vReflectColor = vOriginColor;
    ////}
    ////else
    {
        float fMinDistance = 10000.f;
    
        float4 vEnvColor = 0.f;
    
        uint iIndex = 0;
        uint iSampleCount = clamp(g_iNumEnvMaps, 0, 8);
      
        bool IsInEnvMap = false;
      
        float3 vHitPlane = 0.f;
      
        ENV_MAP Envmap = (ENV_MAP) 0;
      
        for (uint j = 0; j < iSampleCount; ++j)
        {
            Envmap = g_EnvMapDatas[j];
              
            float fLength = length(vWorldPos - Envmap.vPosition);
          
            if (Envmap.fRange >= fLength && fMinDistance > fLength)
            {
                fMinDistance = fLength;
              
                float3 vWorldReflect = normalize(mul(vReflect, g_ViewMatrixInv).xyz);
              
                float3 vLocalPos = vWorldPos.xyz - Envmap.vPosition.xyz;
              
                float3 vExtents = Envmap.fRange;
              
                float3 vToPlane = ((sign(vWorldReflect) * vExtents) - vLocalPos) / vWorldReflect;
              
                float fPlaneDistance = min(vToPlane.x, min(vToPlane.y, vToPlane.z));
              
                vHitPlane = vLocalPos + (vWorldReflect * fPlaneDistance);
              
                vEnvColor = g_EnvMapTexture[j].Sample(DefaultSampler, normalize(vHitPlane));
              
                vReflectColor = vEnvColor;
            }
        }
    }
    

    //float fWeight = IsHit ? fDistWeight : 1.f;
    
    vColor = float4(vReflectColor.xyz, 1.f);

//    vColor = lerp(vReflectColor, vEnvColor, fWeight);
    
    return vColor;
}

float4 Compute_Refract(float4 vWolrdPos, float4 vNormal, float4 vWaterColor, Texture2D<float4> SceneTexture, float fWaterDepth, Texture2D<float4> DepthTexture)
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
    
    float4 vSceneWorldPos = Compute_WorldPos(vUV, DepthTexture);
    
    float3 vDir = normalize(vSceneWorldPos.xyz - vWolrdPos.xyz);
    
    float IsOver = dot(vDir, vRefract) < 0.f;
    
    if(IsOver)
    {
        return vWaterColor;
    }
    
    float4 vRefractvColor = SceneTexture.Sample(DefaultSampler, vUV);
    
    vColor = lerp(vRefractvColor, vWaterColor, fDepthRatio);
    
    return vColor;
}