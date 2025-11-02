#include "Engine_Shader_Defines.hlsli"
#include "Engine_Shader_Function.hlsli"
float g_fLightFar;

Texture2D g_Texture;

//OBJECTS
Texture2D g_DiffuseTexture; // Color
Texture2D g_NormalTexture;  // Normal
Texture2D g_DepthTexture;   // (Depth.x = Proj.z / Proj.w) , (Depth.y = Proj.w)
Texture2D g_PBRTexture;     // (PBR.x = Metallic), (PBR.y = Roughness ), (PBR.z = IsDynamic) 

Texture2DArray<float> g_ShadowMap : register(t2);

//COMBINED
Texture2D g_ToonRimTexture;     // (ToonRim.x = Shade Color ), (ToonRim.y = Shade Value), (ToonRim.z = Rim Value )
Texture2D g_EmissiveTexture;
Texture2D g_DistortionTexture;
Texture2D g_SsaoTexture;        
Texture2D g_BackBufferTexture;

//BLOOM
Texture2D g_BloomTexture;


//RAMP
Texture2D g_RampTexture;            // Shade Color
Texture2D g_ColorRampTexture;       // Rim Color

//FOG
Texture2D g_LutResultTexture;
Texture2D g_FogNoiseTexture;
float2 g_vFogDepthDistance;
float2 g_vFogHeightDistance;
float4 g_vFogColor;
float g_fFogTime;

//SSAO
Texture2D g_NoiseTexture;
float4  g_vSampleVector[16];
int     g_iSampleSize;
float   g_fSSAO_Radius;
float   g_fSSAO_MaxDistance;
float   g_fSSAO_OutDistance;

//DOF
Texture2D g_DofTexture;
Texture2D g_BlurTexture;

//Motion
Texture2D g_VelocityMap;
float g_fLimitVelocity;

Texture2DArray<float4> g_LUT_Texture : register(t1);

const int  g_iLutIndex = 0;
float g_fLutLerpIntensity = 0.f;

cbuffer CSMDatas : register(b1)
{
    float4  g_vClipDistances;
    float   g_fLastDistance;
    float3  padding;
};

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

vector g_vLightDirection = 0.f;
vector g_vLightDiffuse = 1.f;
vector g_vLightAmbient = 1.f;
vector g_vMtrlAmbient = { 0.2f, 0.2f, 0.2f, 0.2f };
vector g_vLightSpecular = 1.f;
vector g_vMtrlSpecular = 1.f;


int g_DebugCSMIndex;

float4 g_fShadowBais = float4(0.01f, 0.02f, 0.03f, 0.05f);
float4 g_fMinShadowBias = 0.f;
float g_DebugSlopeScale = 2.f;

float4 g_vRimColor = float4(0.7f, 0.4f, 0.f, 1.f);
float4 g_fRimIntensity = 0.8f;

float g_fEffectIntensity;

//DEBUG
bool g_IsStylized;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_BACKBUFFER PS_MAIN_DRAW(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    // Default Combine
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
    
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    
    vector vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    float fViewZ = vViewPos.z;
    
    vector vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    //vector vRimLight = g_RimLightTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vToonRim = g_ToonRimTexture.Sample(DefaultSampler, In.vTexcoord);
    float fSSao = g_SsaoTexture.Sample(DefaultSampler, In.vTexcoord).r;
    
    vector vRimColor = vDiffuse;
    //g_ColorRampTexture.Sample(DefaultSampler, float2(0.5f, (1.f - vToonRim.z)));
    
    float fRim = clamp(vToonRim.z, 0.1f, 1.f);
    
    vector vLook = normalize(g_vCamPosition - vWorldPos);
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float3 vLightDir = g_vLightDirection.xyz * -1.f;
    
    /////////TEST
    if (vPBRDesc.z)
    {
        float3 vPBR = Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, vPBRDesc.x, vPBRDesc.y);
        Out.vColor.xyz = vPBR * (vToonRim.x) + (vRimColor.xyz * fRim);
    }
    else
    {
        float3 vPBR = Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, g_fGlobalMetallic, g_fGlobalRoughness);
        Out.vColor.xyz = vPBR * fSSao;
    }
    
    Out.vColor.xyz += vDiffuse.xyz * 0.4f; // Ambient
    
    Out.vColor.a = 1.f;
    
    if (any(vPBRDesc.z))
        return Out;
///////// Shadow Begin /////////

    int iCascadeIndex = 0;
    
    for (int i = 0; i < 4; i++)
    {
        if (fViewZ > g_vClipDistances[i])
            iCascadeIndex = i;
    }
   
    float Gradiant = RPB_Gradiant(fViewZ);
    float fDot = saturate(dot(vNormal, g_vLightDirection * -1.f));
   
    //float fSlopeFactor = (1.f - fDot); // Row
    float fSlopeFactor = sqrt(1.f - pow(fDot, 2)); // High

    float BlendFactor = 0.f;
    
    float fShadowBlend = 0.f;
    
    float2 vTexelSize = float2((1.f / g_iShadowMapSizeX), (1.f / g_iShadowMapSizeY));
    
    // Blend Cascade
    if (iCascadeIndex < 3)          // Max Cascade Check
    {
        int iBlendCascadeIndex = iCascadeIndex + 1;
    
        float CurrentNear = g_vClipDistances[iCascadeIndex];
        float CurrentFar = g_vClipDistances[iBlendCascadeIndex];
        
        float BlendRegion = (CurrentFar - CurrentNear) * 0.15f;                     // Cascade Blend Distance ( Begin ratio 0.85)
        
        BlendFactor = saturate((fViewZ - (CurrentFar - BlendRegion)) / BlendRegion);
        
        vector vShadowBlendPos;
        matrix matShadowBlendLightVP;
        
        matShadowBlendLightVP = mul(g_ShadowViewMatrix[iBlendCascadeIndex], g_ShadowProjMatrix[iBlendCascadeIndex]);
        vShadowBlendPos = mul(vWorldPos, matShadowBlendLightVP);
        
        float2 vBlendTexcood = Compute_Texcoord(vShadowBlendPos.xy);        // 직교라 w 나누기 X
 
        float fBlendBias = max(g_fShadowBais[iBlendCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
        fBlendBias = max(fBlendBias, g_fMinShadowBias[iBlendCascadeIndex]);
        float fBlendDepth = vShadowBlendPos.z - fBlendBias;

        fShadowBlend = ShadowPCF(float3(vBlendTexcood, fBlendDepth), iBlendCascadeIndex, 1, g_ShadowMap); // 2 == Kernel size
    }
    
    // Current Cascade
    {
        vector vShadowPos;
        matrix matShadowLightVP;

        matShadowLightVP = mul(g_ShadowViewMatrix[iCascadeIndex], g_ShadowProjMatrix[iCascadeIndex]);
        vShadowPos = mul(vWorldPos, matShadowLightVP);
    
        float2 vTexcood = Compute_Texcoord(vShadowPos.xy); // 직교라 w 나누기 X

        float fBias = 0.f;
        
        fBias = max(g_fShadowBais[iCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);

        fBias = max(fBias, g_fMinShadowBias[iCascadeIndex]);
    
        float fDepth = vShadowPos.z - fBias;
    
        float fShadow = ShadowPCF(float3(vTexcood, fDepth), iCascadeIndex, 1, g_ShadowMap);

        float fFinalShadow = lerp(fShadow, fShadowBlend, BlendFactor);

        fFinalShadow = saturate(fFinalShadow + 0.3f);
    
        Out.vColor.xyz *= fFinalShadow;
    }
    Out.vColor.a = 1.f;
///////// Shadow End /////////

    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vToonRim : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));

    float NdotL = dot(normalize(g_vLightDirection.xyz * -1.f), vNormal.xyz);
    
    float fToonShade = smoothstep(-0.3f, -0.1f, NdotL);

    Out.vToonRim.x = fToonShade;
  
    vector vWorldPos = Compute_WorldPos(In.vTexcoord, g_DepthTexture);
    
    vector vLook = normalize(g_vCamPosition - vWorldPos);
   
    vector vRimLight = 0.f;
    
    vRimLight = 1.f - abs(dot(vNormal, vLook));
    
    vRimLight *= smoothstep(0.2f, 1.f, NdotL);
    
    vRimLight = pow(vRimLight, 3.f);
    
    Out.vToonRim.z = vRimLight;
    
    return Out;
}


PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;


    return Out;
}

PS_OUT_BACKBUFFER PS_BLOOM(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    vector vOriginColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vColor = g_BloomTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = vOriginColor + (vColor * 0.4f);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DISTORTION(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float2 vTexcoord;
    float2 vWeight;
    vector vNormal;
    vector vNormalData;
    
    vNormalData = g_DistortionTexture.Sample(PointSampler, In.vTexcoord);
    
    vNormalData = vector((vNormalData.xy * 2.f) - 1.f, vNormalData.z, vNormalData.a);
    vWeight = (vNormalData.xy * vNormalData.z) * vNormalData.a;
    
    vWeight *= 0.12f;

    vTexcoord = In.vTexcoord + vWeight;
    
    vector vFinalColor = g_BackBufferTexture.Sample(ClampSampler, vTexcoord);
    
    Out.vColor = vFinalColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_LUT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    vector vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float2 vUV;
    
    float fSpaceSize = 1.f / g_fLUT_Size;
    float fScale = (g_fLUT_Size - 1.f) / g_fLUT_Size;
    
    float fIndex = clamp(floor(vOriginColor.b * g_fLUT_Size), 0, g_fLUT_Size - 1.f);
    float fOffsetX = fIndex * (fSpaceSize);
    
    float fScaleX = vOriginColor.r * fScale;
    
    vUV.x = fScaleX * fSpaceSize + fOffsetX;
    vUV.y = vOriginColor.g;
    
    vector vLUT_Color = g_LUT_Texture.Sample(DefaultSampler, float3(vUV, g_iLutIndex));
    
    vector vFinalColor = lerp(vOriginColor, vLUT_Color, g_fLutLerpIntensity);
    
    Out.vColor = float4(vFinalColor.rgb, 1.f);

    return Out;
}

PS_OUT_BACKBUFFER PS_FOG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    float fViewDepth = vViewPos.z;
    
    float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    float2 vTexScale = float2(1.f / g_fWidth, 1.f / g_fHeight);
    
    float2 vTexcoord = fmod(vWorldPos.xy, float2(g_fWidth, g_fHeight)) * vTexScale;
    
    float2 vNoseTexcoord = float2(vTexcoord.x + (g_fFogTime * vTexScale.x), vTexcoord.y); //vTexcoord + (g_fFogTime * vTexScale);
    
    float fNoise = g_FogNoiseTexture.Sample(DefaultSampler, vNoseTexcoord).r;
    
    float fFogDepthWeight = clamp((smoothstep(g_vFogDepthDistance.x, g_vFogDepthDistance.y, fViewDepth)), 0.f, 1.f);
    
    vector vOriginColor = g_LutResultTexture.Sample(DefaultSampler, In.vTexcoord);
    vOriginColor.xyz *= (1.f - min(fFogDepthWeight, 0.8f));
    
    float fFogWeight = fFogDepthWeight;// * lerp(0.2f, 1.f, fFogHeightWeight);
    fFogWeight *= fNoise;
    
    Out.vColor = lerp(vOriginColor, g_vFogColor, fFogWeight);
    
    return Out;
}


PS_OUT_BACKBUFFER PS_SSAO(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vViewPos = Compute_ViewPos_SSAO(In.vTexcoord, g_DepthTexture);
   
    if (vViewPos.z == 0.f || vViewPos.z >= g_fSSAO_OutDistance)
    {
        Out.vColor = 1.f;
        return Out;
    }
    
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    vNormal = mul(vNormal, g_CamViewMatrix);
    
    float2 vNoiseScale = float2(g_fWidth / 16.f, g_fHeight / 16.f);
    
    vector vNoiseNormal = Compute_Normal(g_NoiseTexture, PointSampler, In.vTexcoord * vNoiseScale);
    vNoiseNormal = mul(vNoiseNormal, g_CamViewMatrix);
    
    float Occlusion = 0.f;
    
    [unroll]
    for (int i = 0; i < g_iSampleSize; ++i)
    {
        Occlusion += SSAO_Factor(g_vSampleVector[i], vNoiseNormal, vNormal, vViewPos, g_fSSAO_Radius, g_fSSAO_MaxDistance, g_DepthTexture);
    }
    
    float AO = (Occlusion / g_iSampleSize);
    
    AO = pow(AO, 2.f);
    
    Out.vColor.xyz = AO;
    Out.vColor.w = 1.f;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DOF(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vDofColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vDofData = g_DofTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float fMask = smoothstep(0.3f, 1.f, vDofData.x);
    
    float4 vFinalColor = lerp(vOriginColor, vDofColor, fMask);
    
    Out.vColor = lerp(vOriginColor, vFinalColor, g_fEffectIntensity);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DOF_DEPTH(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float fCoc = Compute_COC(In.vTexcoord, g_DepthTexture);
    
    float4 vDepth = 0.f;
    
    vDepth.x = fCoc;
    
    vDepth.y = fCoc <= g_fFocusMinCoc ? 0.f : 1.f;
    
    vDepth.z = g_fFocusMinCoc;
    
    Out.vColor = float4(vDepth.xyz, 1.f);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vBlurColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = lerp(vOriginColor, vBlurColor, g_fEffectIntensity);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_VELOCITY_MAP(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    bool IsDyanmic = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord).z;       // Dynamic Discard;
    
    if(IsDyanmic)
        return Out;
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    Out.vColor.z = vViewPos.z;          // Depth 기록
    
    float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    float4x4 PrevVP = mul(g_PrevCamViewMatrix, g_PrevCamProjMatrix);
    
    float4 vPrevProjPos = mul(vWorldPos, PrevVP);
    vPrevProjPos /= vPrevProjPos.w;
    
    float2 vPrevTexcoord = Compute_Texcoord(vPrevProjPos.xy);
    
    float2 vCurTexcoord = float2(In.vTexcoord.x * g_fWidth, In.vTexcoord.y * g_fHeight);    // 픽셀 거리로 보정
    vPrevTexcoord = float2(vPrevTexcoord.x * g_fWidth, vPrevTexcoord.y * g_fHeight);
    
    float2 vMotionVector = vPrevTexcoord - vCurTexcoord;
    
    Out.vColor.xy = vMotionVector;
    Out.vColor.a = 1.f;
    
    return Out;
}


PS_OUT_BACKBUFFER PS_MOTION_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vBlurColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    float2 vVelocity = g_VelocityMap.Sample(DefaultSampler, In.vTexcoord).xy;
    
    bool IsBlur = length(vVelocity) > g_fLimitVelocity ? true : false;
    
    if(IsBlur)
        Out.vColor = lerp(vOriginColor, vBlurColor, g_fEffectIntensity);
    else
        Out.vColor = vOriginColor;
    
    return Out;
}


PS_OUT_BACKBUFFER PS_MAIN_DEBUG_CSM(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float fShadow = 0.f;
   
    fShadow = g_ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(In.vTexcoord, g_DebugCSMIndex), 1.f);
    
    float4 vColor = 1.f;
   
    if (fShadow != 1.f)
    {
    
    switch (g_DebugCSMIndex)
    {
        case 0:
            vColor = float4(fShadow, 0.f, 0.f, 1.f);
            break;
        case 1:
            
            vColor = float4(0.f, fShadow, 0.f, 1.f);
            break;
        case 2:
            
            vColor = float4(0.f, 0.f, fShadow, 1.f);
            break;
        case 3:
            vColor = float4(fShadow, fShadow, fShadow, 1.f);
            break;
    }
    }
   
    Out.vColor = vColor;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DebugPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DRAW();
    }
    
    pass CSM // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG_CSM();
    }
    
    pass CombinedPass // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }
    pass DirectionalPass // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_DIRECTIONAL();
    }
    pass PointPass // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_POINT();
    }
    
    pass Bloom // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLOOM();
    }
    
    pass Distortion // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISTORTION();
    }
    
    pass LUT // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUT();
    }
    
    pass Fog // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FOG();
    }
    
    pass SSAO // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO();
    }
    
    pass DOF // 10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF();
    }
    
    pass DOF_DEPTH // 11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF_DEPTH();
    }
    
    pass Blur   // 12
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR();
    }
    
    pass VelocityMap // 13
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VELOCITY_MAP();
    }
    
    pass MotionBlur // 14
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MOTION_BLUR();
    }
}