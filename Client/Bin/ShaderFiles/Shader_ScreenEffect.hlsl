#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_SceneTexture;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture;
Texture2D g_NoiseTexture;

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

struct PS_OUT_SFX
{
    float4 vColor : SV_TARGET0;
    float3 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
};

struct PS_OUT_POST_SFX
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_SFX PS_MAIN(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;

    float4 vBase = g_DiffuseTexture.Sample(DefaultSampler, float2(In.vTexcoord.x, In.vTexcoord.y));
    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vNoise = g_NoiseTexture.Sample(DefaultSampler, float2(In.vTexcoord.x * 20.f, In.vTexcoord.y));
    float4 vSecond = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    
    float4 vSFXColor = vMask;
    vSFXColor.a = max(max(vMask.r, vMask.g), vMask.b);
    
    Out.vColor = vSFXColor;
    
    //Out.vEmissive = Out.vColor;
    
    Out.vDistortion = (1.f - vBase);
    
    Out.vDistortion.a = 1.f;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}