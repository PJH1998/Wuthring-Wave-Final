#include "Engine_Shader_Defines.hlsli"
#include "Engine_Shader_Shadow.hlsli"

Texture2DArray<float4> g_LUT_Texture : register(t1);

const int  g_iLutIndex = 0;
float g_fLutLerpIntensity = 0.f;

float g_fLightFar;

Texture2D g_Texture;

//OBJECTS
Texture2D g_DiffuseTexture; // Color
Texture2D g_NormalTexture;  // Normal
Texture2D g_DepthTexture;   // (Depth.x = Proj.z / Proj.w) , (Depth.y = Proj.w)
Texture2D g_PBRTexture;     // (PBR.x = Metallic), (PBR.y = Roughness ), (PBR.z = IsDynamic) 

//BackBuffer
Texture2D g_BackBufferTexture;

//COMBINED
Texture2D g_LightAccTexture;
Texture2D g_SsaoTexture;        

//Emissive
Texture2D g_EmissiveTexture;

//Distoriton
Texture2D g_DistortionTexture;

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
Texture3D g_VoulmetricTexture;
float2 g_vFogRange;

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

//Light
vector  g_vLightDirection = 0.f;
vector  g_vLightDiffuse = 1.f;
vector  g_vLightAmbient = 1.f;
vector  g_vMtrlAmbient = 0.4f;
vector  g_vLightPosition;
float   g_fLightRange; 
vector  g_vLightSpecular = 1.f;
vector  g_vMtrlSpecular = 1.f;

int g_DebugCSMIndex;

//SHADOWMAP
Texture2DArray<float> g_ShadowMap;
bool g_HasShadowMap;

//CASCADE
Texture2DArray<float> g_Cascade : register(t2);

float4 g_vShadowLightDirection;

float4 g_vRimColor = float4(0.7f, 0.4f, 0.f, 1.f);
float4 g_fRimIntensity = 0.8f;

//SFX
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

    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vLightResult = g_LightAccTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if(vLightResult.a == 0.f)
        discard;
    
    Out.vColor = vLightResult;
    
    if (any(vPBRDesc.z))
        return Out;
        
    float fSSao = g_SsaoTexture.Sample(DefaultSampler, In.vTexcoord).r;
    Out.vColor *= fSSao;
        
///////// Shadow Begin /////////
    
    Out.vColor.a = 1.f;
///////// Shadow End /////////

    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vLightAcc : SV_TARGET0;
};

PS_OUT_LIGHT PS_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
        
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    
    //vector vWorldPos = Compute_WorldPos(In.vTexcoord, g_DepthTexture);
    //
    //vector vLook = normalize(g_vCamPosition - vWorldPos);
    vector vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    vector vLook = normalize(vViewPos * -1.f);
    
    float3 vLightDir = g_vLightDirection.xyz * -1.f;
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float NdotL = dot(normalize(vLightDir), vNormal.xyz);
    float fRimPower = Compute_RimPower(vNormal, vLook, NdotL);
    
    if (vPBRDesc.z)
    {
        float fToonShade = smoothstep(-0.3f, -0.1f, NdotL);
        
        float3 vPBR = Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, vPBRDesc.x, vPBRDesc.y);
        Out.vLightAcc.xyz = g_vLightDiffuse.xyz * ((vPBR * fToonShade) + fRimPower);
    }
    else
    {
        float3 vPBR = Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, g_fGlobalMetallic, g_fGlobalRoughness);
        
        vector vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
        float fViewZ = vViewPos.z;

        vector vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
        float fShadowMap = 1.f;
    
        vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    
        float fNdotL = saturate(dot(vNormal, g_vShadowLightDirection * -1.f));
    
        if (g_HasShadowMap)
        {
            fShadowMap = Compute_ShadowMap(fViewZ, fNdotL, vWorldPos, g_ShadowMap);
        }
   
        float fShadow = Compute_Cascade(fViewZ, fNdotL, vWorldPos, g_Cascade);
    
        float fFinalShadow = min(fShadowMap, fShadow);
    
//        fFinalShadow = lerp(0.7f, 1.f, fFinalShadow);
        
        Out.vLightAcc.xyz = g_vLightDiffuse.xyz * (vPBR * fFinalShadow);
    }
    
    float4 vAmbientColor = lerp(vDiffuse, g_vLightDiffuse, g_vLightAmbient);
   
    Out.vLightAcc.xyz += (vAmbientColor * g_vMtrlAmbient).xyz;
    
    Out.vLightAcc.a = 1.f;
    
    return Out;
}


PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
        
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    
    vector vWorldPos = Compute_WorldPos(In.vTexcoord, g_DepthTexture);
    
    vector vLook = normalize(g_vCamPosition - vWorldPos);
    
    float3 vLightDir = g_vLightPosition.xyz - vWorldPos.xyz;
    float fDistance = length(vLightDir);
    
    float fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float NdotL = dot(normalize(vLightDir), vNormal.xyz);
    float fRimPower = Compute_RimPower(vNormal, vLook, NdotL);
    float fToonShade = smoothstep(-0.3f, -0.1f, NdotL);
 
    if (vPBRDesc.z)
    {
        float3 vPBR = Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, normalize(vLightDir), vDiffuse.xyz, vPBRDesc.x, vPBRDesc.y);
        Out.vLightAcc.xyz = g_vLightDiffuse.xyz * (vPBR * fToonShade + fRimPower);
        Out.vLightAcc.xyz *= fAtt;
    }
    else
    {
        float3 vPBR = Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, g_fGlobalMetallic, g_fGlobalRoughness);
        Out.vLightAcc.xyz = g_vLightDiffuse.xyz * (vPBR + fRimPower);
        Out.vLightAcc.xyz *= fAtt;
    }

    float4 vAmbientColor = lerp(vDiffuse, g_vLightDiffuse, g_vLightAmbient);
   
    Out.vLightAcc.xyz += (vAmbientColor * g_vLightAmbient).xyz * fAtt;
    
    Out.vLightAcc.a = 1.f;
    
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
    
    float fViewZ = vViewPos.z == 0.f ? g_vFogRange.y : clamp(vViewPos.z, 0.1f, g_vFogRange.y);;
    
    vector vOriginColor = g_LutResultTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (fViewZ < g_vFogRange.x)
    {
        Out.vColor = vOriginColor;
        return Out;
    }
        
    float fZ = log(fViewZ / g_vFogRange.x) / log(g_vFogRange.y / g_vFogRange.x);
    
    float3 vUV = float3(In.vTexcoord, fZ);
    
    float4 VF = g_VoulmetricTexture.Sample(DefaultSampler, vUV);
    
    float3 vFogColor = saturate(VF.xyz);
    float fAlpha = saturate(1.f - VF.a);
    
  //  fAlpha = lerp(0.8f, 0.f, saturate(VF.a));
    
    Out.vColor.xyz = lerp(vOriginColor.xyz, vFogColor, fAlpha);
    Out.vColor.a = 1.f;
    
    //float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    //float2 vTexScale = float2(1.f / g_fWidth, 1.f / g_fHeight);
    
    //float2 vTexcoord = fmod(vWorldPos.xy, float2(g_fWidth, g_fHeight)) * vTexScale;
    
    //float2 vNoseTexcoord = float2(vTexcoord.x + (g_fFogTime * vTexScale.x), vTexcoord.y); //vTexcoord + (g_fFogTime * vTexScale);
    
    //float fNoise = g_FogNoiseTexture.Sample(DefaultSampler, vNoseTexcoord).r;
    
    //float fFogDepthWeight = clamp((smoothstep(g_vFogDepthDistance.x, g_vFogDepthDistance.y, fViewDepth)), 0.f, 1.f);
    
    //vector vOriginColor = g_LutResultTexture.Sample(DefaultSampler, In.vTexcoord);
    //vOriginColor.xyz *= (1.f - min(fFogDepthWeight, 0.8f));
    
    //float fFogWeight = fFogDepthWeight;// * lerp(0.2f, 1.f, fFogHeightWeight);
    //fFogWeight *= fNoise;
    
    //Out.vColor = lerp(vOriginColor, g_vFogColor, fFogWeight);
    
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
    
    if(AO >= 0.8f)
        AO = 1.f;
    
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
    
    Out.vColor.z = vViewPos.z;          // Depth ���
    
    float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    float4x4 PrevVP = mul(g_PrevCamViewMatrix, g_PrevCamProjMatrix);
    
    float4 vPrevProjPos = mul(vWorldPos, PrevVP);
    vPrevProjPos /= vPrevProjPos.w;
    
    float2 vPrevTexcoord = Compute_Texcoord(vPrevProjPos.xy);
    
    float2 vCurTexcoord = float2(In.vTexcoord.x * g_fWidth, In.vTexcoord.y * g_fHeight);    // �ȼ� �Ÿ��� ����
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
   
    fShadow = g_Cascade.SampleCmpLevelZero(ShadowSampler, float3(In.vTexcoord, g_DebugCSMIndex), 1.f);
    
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

PS_OUT_BACKBUFFER PS_MAIN_DEBUG_SHADOW_MAP(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float fShadow = 0.f;
   
    fShadow = g_ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(In.vTexcoord, g_DebugCSMIndex), 1.f);
    
    
    //fShadow = g_ShadowMap.Sample(DefaultSampler, float3(In.vTexcoord, 0.f));
    
    float4 vColor = 1.f;
    
    vColor.xyz = fShadow;
    vColor.a = 1.f;
    
    if (In.vTexcoord.x == 0.5f || In.vTexcoord.y == 0.5f)
        vColor = 0.f;
   
    //if (fShadow != 1.f)
    //{
    //    vColor = float4(fShadow, 0.f, 0.f, 1.f);
    //}
   
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
    
    pass SHAODW_MAP // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG_SHADOW_MAP();
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
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

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