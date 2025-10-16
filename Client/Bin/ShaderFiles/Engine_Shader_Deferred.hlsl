#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
matrix g_LightViewMatrix, g_LightProjMatrix;
float g_fLightFar;
vector g_vCamPosition;

Texture2D g_LUTTexture;
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

vector g_vLightDirection = 0.f;
vector g_vLightDiffuse = 1.f;
vector g_vLightAmbient = 1.f;
vector g_vMtrlAmbient = { 0.7f, 0.7f, 0.7f, 0.f };
vector g_vLightSpecular = 1.f;
vector g_vMtrlSpecular = 1.f;

float g_fWidth = 1920.f;
float g_fHeight= 1080.f;

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
    
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = vDiffuse * vShade; //+vSpecular;
    
    // Shadow Àû¿ë
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos = vWorldPos * vDepthDesc.y;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    vector vShadowPos;
    matrix matShadowLightVP;
    matShadowLightVP = mul(g_LightViewMatrix, g_LightProjMatrix);
    vShadowPos = mul(vWorldPos, matShadowLightVP);
    
    float2 vTexcood;
    //vTexcood.x = vShadowPos.x / vShadowPos.w * 0.5f + 0.5f;
    //vTexcood.y = vShadowPos.y / vShadowPos.w * -0.5f + 0.5f;
    vTexcood.x = vShadowPos.x * 0.5f + 0.5f;
    vTexcood.y = vShadowPos.y * -0.5f + 0.5f;
    
    vector vLightDepth = g_LightDepthTexture.Sample(DefaultSampler, vTexcood);
    
    float fLightDepth = vLightDepth.x;
    //float fDepth = vShadowPos.w / g_fLightFar;
    float fDepth = vShadowPos.z;
    
    float fDistance = fDepth - fLightDepth;
    if (fDistance > 0.0005f)
        Out.vColor.rgb = Out.vColor.rgb * 0.5f;
    //saturate(fDistance * 30.f);
    
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
    
    float fShade = max(dot(normalize(g_vLightDirection), -1.f * vNormal), 0.f);
    Out.vShade = g_vLightDiffuse * saturate(fShade + (g_vLightAmbient * g_vMtrlAmbient));

    vector DepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = DepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= DepthDesc.y;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    vector vReflect = reflect(normalize(g_vLightDirection), vNormal);
    vector vCamDir = vWorldPos - g_vCamPosition;
    float fSpecular = pow(max(dot(normalize(vReflect) * -1.f, normalize(vCamDir)), 0.f), 50.f);
    
    Out.vSpecular = g_vLightSpecular * g_vMtrlSpecular * fSpecular;
    
    return Out;
}

PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;


    return Out;
}

struct PS_OUT_BLUR
{
    float4 vBlur : SV_TARGET0;
};

PS_OUT_BLUR PS_BLUR_X(PS_IN In)
{
    PS_OUT_BLUR Out = (PS_OUT_BLUR) 0;

    vector vColor = 0.f;
    
    float fTexel = 1.f / g_fWidth;
    
    for (int i = -6; i < 7; ++i)
    {
        float2 vTexcoord = float2(In.vTexcoord.x + i * fTexel, In.vTexcoord.y);
        vColor += g_EmissiveTexture.Sample(ClampSampler, vTexcoord) * g_fWeights[i + 6];
    }
    
    Out.vBlur = vColor;

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

technique11 DefaultTechnique
{
    pass DebugPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }
    pass CombinedPass // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }
    pass DirectionalPass // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_DIRECTIONAL();
    }
    pass PointPass // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_POINT();
    }
    pass Blur_X // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR_X();
    }
    pass Blur_Y // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR_Y();
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
}