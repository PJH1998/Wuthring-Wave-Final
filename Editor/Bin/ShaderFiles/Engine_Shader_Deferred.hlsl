#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
float g_fLightFar;
vector g_vCamPosition;

Texture2D g_Texture;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_DepthTexture;
Texture2D g_Mat_SpecularTexture;
Texture2D g_Mat_AmbientTexture;

Texture2D g_ShadeTexture;
Texture2D g_SpecularTexture;
Texture2D g_LightDepthTexture;
Texture2D g_EmissiveTexture;
Texture2D g_BlurTexture;
Texture2D g_BackBufferTexture;
Texture2D g_DistortionTexture;
Texture2D g_BlurEndTexture;

Texture2D g_RampTexture;

Texture2DArray<float> g_ShadowMap : register(t0);
Texture2DArray<float4> g_LUT_Texture : register(t1);

const int  g_iLutIndex = 0;
float g_fLutLerpIntensity = 0.7f;

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
vector g_vMtrlAmbient = { 0.4f, 0.4f, 0.4f, 0.4f };
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
    
    Out.vColor = vDiffuse * vShade * vSpecular;
    
///////// Shadow ���� /////////

    int iCascadeIndex = 0;
    
    for (int i = 0; i < 4; i++)
    {
        if (fViewZ > g_vClipDistances[i])
            iCascadeIndex = i;
    }
   
    float Gradiant = RPB_Gradiant(fViewZ);
   
    float fDot = saturate(dot(vNormal, g_vLightDirection * -1.f));
   
    //float fSlopeFactor = (1.f - fDot); // ���� ��� �ΰ�, �ܼ� ���� ���� ���ü��� Ŀ����
    float fSlopeFactor = sqrt(1.f - pow(fDot, 2)); // ���� ���⸦ ����� ������ ����

    float BlendFactor = 0.f;
    
    float fShadowBlend = 0.f;
    
    float2 vTexelSize = float2((1.f / g_iShadowMapSizeX), (1.f / g_iShadowMapSizeY));
    
    // Blend Cascade
    if (iCascadeIndex < 3)          // Cascade ���� ������ ����
    {
        int iBlendCascadeIndex = iCascadeIndex + 1;
    
        float CurrentNear = g_vClipDistances[iCascadeIndex];
        float CurrentFar = g_vClipDistances[iBlendCascadeIndex];
        
        float BlendRegion = (CurrentFar - CurrentNear) * 0.15f;         // ����������� Blend �Ұ��� ���� ( 0.15 == 0.85 �������� )
        
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

        fShadowBlend = SampleShadowPCF(g_ShadowMap, ShadowSampler, float3(vBlendTexcood, fBlendDepth), iBlendCascadeIndex, 2);      // 2 == Kernel size
    }
    
    // ���� Cascade
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
    
        float fShadow = SampleShadowPCF(g_ShadowMap, ShadowSampler, float3(vTexcood, fDepth), iCascadeIndex, 2);

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

    fY = max(0.5f, fY);
    
    float fShade = g_RampTexture.Sample(PointSampler, float2(0.5f, fY)).r;

 //   if (fShade <= radians(30.f))
 //   {
 //       float fRatio = max(0.f, fShade) / radians(30.f);
 //       
 //       fShade = lerp(0.8f, 1.f, fRatio);
 //   }   
 //   else
 //       fShade = 1.f;

    Out.vShade.xyz = fShade;
    Out.vShade.w = 1.f;
        
    //int iShade = fShade * 4;
    
    //fShade = iShade / 4.f;
    
    
    //Out.vShade = g_vLightDiffuse * saturate(fShade + (g_vLightAmbient * g_vMtrlAmbient));
    //Out.vShade.w = 1.f;
//    Out.vShade = g_vLightDiffuse * saturate(fShade + (g_vLightAmbient * g_vMtrlAmbient));


    //vector vReflect = reflect(normalize(g_vLightDirection), vNormal);
    //float Rim = saturate(dot(vNormal, vCamDir));
    
    //if(Rim > 0.3f)
    //    Rim = 1.f;
    
    
    //Out.vSpecular = float4(pow(1.f - Rim, 5.f) * float3(1.f, 1.f, 1.f), 1.f);
    //float fSpecular = pow(max(dot(normalize(vReflect) * -1.f, normalize(vCamDir)), 0.f), 50.f);
    
//    float fShade = max(dot(normalize(vCamDir), vNormal), 0.f);
//    
//    Out.vShade = g_RampTexture.Sample(DefaultSampler, float2(0.f, fShade));
    
    //Out.vSpecular = g_vLightSpecular * g_vMtrlSpecular * (g_vLightDiffuse * fSpecular);
    
    vector DepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = DepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= DepthDesc.y;

    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    
    float fViewZ = vWorldPos.z;
    
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    vector vCamDir = g_vCamPosition - vWorldPos;
    
    bool IsOutline = false;
    
    //if (DepthDesc.z == 1.f)
    //{
    //    IsOutline = Outline_Normal(1920.f, 1080.f, g_NormalTexture, DefaultSampler, In.vTexcoord, vNormal.xyz, radians(15.f));
    //}
    
    //float fDot = dot(normalize(vCamDir), vNormal);
    
    //if(fDot <= radians(15.f))
    //{
    //    if (vWorldPos.z <= 300.f)
    //    {
    //        if (DepthDesc.z == 1.f && DepthDesc.x != 1.f)
    //            IsOutline = Outline(1920.f, 1080.f, g_DepthTexture, DefaultSampler, In.vTexcoord, fViewZ, 5.f, g_ProjMatrixInv);
    //    }
    //}
    
    Out.vSpecular = 1.f;
    
    if (IsOutline == true)
        Out.vSpecular = float4(0.35f, 0.1f, 0.f, 1.f);
    
    return Out;
}

PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;


    return Out;
}

PS_OUT_BACKBUFFER PS_BLUR_X(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vColor = 0.f;
    
    float fTexel = 1.f / g_fWidth;
    
    for (int i = -6; i < 7; ++i)
    {
        float2 vTexcoord = float2(In.vTexcoord.x + i * fTexel, In.vTexcoord.y);
        vColor += g_EmissiveTexture.Sample(ClampSampler, vTexcoord) * g_fWeights[i + 6];
    }
    
    Out.vColor = vColor;

    return Out;
}

PS_OUT_BACKBUFFER PS_BLUR_Y(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float2 vTexcoord;
    vector vColor;
    
    float fTexel = 1.f / g_fHeight;
    
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x;
        vTexcoord.y = In.vTexcoord.y + fTexel;
        
        vColor += g_BlurTexture.Sample(ClampSampler, vTexcoord) * g_fWeights[i + 6];
    }
    
    vColor.a = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vFinalColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = vFinalColor + vColor;
    
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

PS_OUT_BACKBUFFER PS_LUT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    vector vOriginColor = g_BackBufferTexture.Sample(PointSampler, In.vTexcoord);
    
    float fLUT_Index = floor(vOriginColor.b * g_fLUT_Size);
    
    float fStartU = fLUT_Index / g_fLUT_Size;
    
    float fU = vOriginColor.r / g_fLUT_Size;
    
    fU = fU + fStartU;
    float fV = vOriginColor.g;
    
    vector vLUT_Color = g_LUT_Texture.Sample(PointSampler, float3(fU, fV, g_iLutIndex)).bgra;
    
    vector vFinalColr = lerp(vOriginColor, vLUT_Color, g_fLutLerpIntensity);
    
    Out.vColor = float4(vFinalColr.rgb, 1.f);

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
    pass Blur_X // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR_X();
    }
    pass Blur_Y // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR_Y();
    }
    
    pass Distortion // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISTORTION();
    }
    
    pass LUT // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUT();
    }
   
}