#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_CamViewMatrix, g_CamProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
float g_fLightFar;
vector g_vCamPosition;
float g_fFar;

Texture2D g_Texture;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_DepthTexture;

Texture2D g_ShadeTexture;
Texture2D g_SpecularTexture;
Texture2D g_LightDepthTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DistortionTexture;

Texture2D g_BackBufferTexture;

Texture2D g_SsaoTexture;

Texture2D g_BlurBeginTexture;
Texture2D g_BlurTexture;
Texture2D g_BlurEndTexture;

Texture2D g_RampTexture;
Texture2D g_NoiseTexture;

vector g_vSampleVector[32];
float g_fSSAO_Radius = 10.f;
float g_fDepthSigma = 0.01f;
float g_fMinDepthDistance = 10.f;
float g_fMinNormalWeight = 0.1f;


Texture2DArray<float> g_ShadowMap : register(t0);
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

float g_fWidth = 1920.f;
float g_fHeight= 1080.f;

int g_DebugCSMIndex;

float4 g_fShadowBais = float4(0.01f, 0.02f, 0.03f, 0.05f);
float4 g_fMinShadowBias = 0.f;
float g_DebugSlopeScale = 2.f;

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

PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
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
        
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos = vWorldPos * vDepthDesc.y;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    
    float fViewZ = vWorldPos.z;
    
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSSao = g_SsaoTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = vDiffuse * vShade * lerp(vSSao, 1.f, vShade);
    
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
    
        float2 vBlendTexcood;
        vBlendTexcood.x = vShadowBlendPos.x * 0.5f + 0.5f;
        vBlendTexcood.y = vShadowBlendPos.y * -0.5f + 0.5f;
    
        float fBlendBias = max(g_fShadowBais[iBlendCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);
    
        fBlendBias = max(fBlendBias, g_fMinShadowBias[iBlendCascadeIndex]);
        float fBlendDepth = vShadowBlendPos.z - fBlendBias;

        fShadowBlend = SampleShadowPCF(g_ShadowMap, ShadowSampler, float3(vBlendTexcood, fBlendDepth), iBlendCascadeIndex, 1);      // 2 == Kernel size
    }
    
    // Current Cascade
    {
        vector vShadowPos;
        matrix matShadowLightVP;

        matShadowLightVP = mul(g_ShadowViewMatrix[iCascadeIndex], g_ShadowProjMatrix[iCascadeIndex]);
        vShadowPos = mul(vWorldPos, matShadowLightVP);
    
        float2 vTexcood;
        vTexcood.x = vShadowPos.x * 0.5f + 0.5f;
        vTexcood.y = vShadowPos.y * -0.5f + 0.5f;
    
        float fBias = 0.f;
    
        fBias = max(g_fShadowBais[iCascadeIndex], g_DebugSlopeScale * fSlopeFactor * Gradiant);

        fBias = max(fBias, g_fMinShadowBias[iCascadeIndex]);
    
        float fDepth = vShadowPos.z - fBias;
    
        float fShadow = SampleShadowPCF(g_ShadowMap, ShadowSampler, float3(vTexcood, fDepth), iCascadeIndex, 1);

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
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));

    float fY = saturate(dot(normalize(g_vLightDirection.xyz * -1.f), vNormal.xyz));

    fY = max(0.2f, fY);
    
    float fShade = g_RampTexture.Sample(PointSampler, float2(0.5f, fY)).r;

    Out.vShade.xyz = fShade;
    Out.vShade.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;


    return Out;
}

PS_OUT_BACKBUFFER PS_GAUSSIAN_BLUR_X(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vColor = 0.f;
    
    float fTexel = 1.f / g_fWidth;
    
    [unroll]
    for (int i = -6; i < 7; ++i)
    {
        float2 vTexcoord = float2(In.vTexcoord.x + i * fTexel, In.vTexcoord.y);
        vColor += g_BlurBeginTexture.Sample(ClampSampler, vTexcoord) * g_fWeights[i + 6];
    }
    
    Out.vColor = vColor;

    return Out;
}

PS_OUT_BACKBUFFER PS_GAUSSIAN_BLUR_Y(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float2 vTexcoord = 0.f;
    vector vColor = 0.f;
    
    float fTexel = 1.f / g_fHeight;
    
    [unroll]
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x;
        vTexcoord.y = In.vTexcoord.y + fTexel;
        
        vColor += g_BlurTexture.Sample(ClampSampler, vTexcoord) * g_fWeights[i + 6];
    }

    Out.vColor = vColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_SSAO_BLUR_X(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vOriginColor = g_BlurBeginTexture.Sample(PointSampler, In.vTexcoord);
    float fOriginDepth = g_DepthTexture.Sample(PointSampler, In.vTexcoord).y;
    vector vOriginNormal = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    vOriginNormal = normalize(float4(vOriginNormal.xyz * 2.f + 1.f, 0.f));
    
    if (fOriginDepth == 0.f)
    {
        Out.vColor = vOriginColor;
        return Out;
    }
    
    vector vColor = 0.f;
    float fCount = 0.f;
    
    float fTexelSize = 1.f / 1920.f;
    [unroll]
    for (int x = -2; x <= 2; ++x)
    {
        float2 vTexcoord = float2(In.vTexcoord.x + (x * fTexelSize), In.vTexcoord.y);
        vector vSampleColor = g_BlurBeginTexture.Sample(ClampSampler, vTexcoord);
        float fSampleDepth = g_DepthTexture.Sample(ClampSampler, vTexcoord).y;
        if (fSampleDepth == 0.f || vSampleColor.r == 1.f)
        {
            continue;
        }
        
        float fDepthDist = abs(fOriginDepth - fSampleDepth);
        
        vector vSampleNormal = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
        vSampleNormal = normalize(float4(vSampleNormal.xyz * 2.f + 1.f, 0.f));
    
        float fNormalWeight = saturate(dot(vOriginNormal, vSampleNormal));
        
        float fDistWeight = g_fSSAOWeights[x+6];
        
        if (fDepthDist <= g_fMinDepthDistance && vSampleColor.r != 1.f)
        {
            vector vFinalColor = vSampleColor * (1.f + (1.f - fNormalWeight));// * (1.f - fDistWeight);
            vColor += vFinalColor;
            fCount += 1.f;
        }
        //if(fDepthDist <= 10.f && vSampleColor.r != 1.f)
        //{
        //    vColor += vSampleColor;
        //    fCount += 1.f;
        //}
    }
    
    if(fCount > 0.f)
    {  
        Out.vColor = float4((vColor.xyz / fCount), 1.f);
    }
    else
        Out.vColor = vOriginColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_SSAO_BLUR_Y(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vOriginColor = g_BlurTexture.Sample(PointSampler, In.vTexcoord);
    float fOriginDepth = g_DepthTexture.Sample(PointSampler, In.vTexcoord).y;
    vector vOriginNormal = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    vOriginNormal = normalize(float4(vOriginNormal.xyz * 2.f + 1.f, 0.f));
    
    if (fOriginDepth == 0.f)
    {
        Out.vColor = vOriginColor;
        return Out;
    }
    
    vector vColor = 0.f;
    float fCount = 0.f;
    
    float fTexelSize = 1.f / 1080.f;
    [unroll]
    for (int y = -2; y <= 2; ++y)
    {
        float2 vTexcoord = float2(In.vTexcoord.x, In.vTexcoord.y + (y * fTexelSize));
        vector vSampleColor = g_BlurTexture.Sample(ClampSampler, vTexcoord);
        float fSampleDepth = g_DepthTexture.Sample(ClampSampler, vTexcoord).y;
        if (fSampleDepth == 0.f || vSampleColor.r == 1.f)
        {
            vColor += vOriginColor;
            fCount += 1.f;
            continue;
        }
         
        float fDepthDist = abs(fOriginDepth - fSampleDepth);
        
        vector vSampleNormal = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
        vSampleNormal = normalize(float4(vSampleNormal.xyz * 2.f + 1.f, 0.f));
    
        float fNormalWeight = saturate(dot(vOriginNormal, vSampleNormal));
        
        float fDistWeight = g_fSSAOWeights[y + 6];
        
        if (fDepthDist <= g_fMinDepthDistance && vSampleColor.r != 1.f)
        {
            vector vFinalColor = vSampleColor * (1.f + (1.f - fNormalWeight));// * (1.f - fDistWeight);
            vColor += vFinalColor;
            fCount += 1.f;
        }
    }
    
    if (fCount > 0.f)
    {
        Out.vColor = float4((vColor.xyz / fCount), 1.f);
    }
    else
        Out.vColor = vOriginColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_BLUR_RETURN(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    Out.vColor = g_BlurEndTexture.Sample(DefaultSampler, In.vTexcoord);

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
    
    vector vFinalColor = g_BlurEndTexture.Sample(ClampSampler, vTexcoord);
    
    Out.vColor = vFinalColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_SSAO(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector DepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = DepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= DepthDesc.y;

    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    
    vector vViewPos = vWorldPos;
    
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    vNormal = mul(vNormal, g_CamViewMatrix);
    
    vector vNoiseNormal = g_NoiseTexture.Sample(PointSampler, In.vTexcoord * 8.f);
    vNoiseNormal = normalize(vector(vNoiseNormal.xyz * 2.f - 1.f, 0.f));
    
    float Occlusion = 0.f;
       
    [unroll]
    for (int i = 0; i < g_iSampleSize; ++i)
    {
        Occlusion += 1.f - SSAO_Factor(g_DepthTexture, DefaultSampler, g_vSampleVector[i], vNoiseNormal, vNormal, vViewPos, g_CamProjMatrix, g_fSSAO_Radius);
    }
    
    Occlusion = 1.f - (Occlusion / g_iSampleSize);
    
    Out.vColor.xyz = Occlusion;
    Out.vColor.w = 1.f;
    
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
    
    vector vFinalColr = lerp(vOriginColor, vLUT_Color, g_fLutLerpIntensity);
    
    Out.vColor = float4(vFinalColr.rgb, 1.f);

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
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
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
    pass G_Blur_X // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GAUSSIAN_BLUR_X();
    }
    pass G_Blur_Y // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GAUSSIAN_BLUR_Y();
    }
    pass SSAO_Blur_X // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO_BLUR_X();
    }
    pass SSAO_Blur_Y // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO_BLUR_Y();
    }
    
    pass BLUR_Retuurn // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR_RETURN();
    }
    
    pass Distortion // 10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISTORTION();
    }
    
    pass LUT // 11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUT();
    }
    
    pass SSAO   // 12
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO();
    }
}