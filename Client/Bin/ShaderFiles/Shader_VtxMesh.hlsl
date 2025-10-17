#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D   g_DiffuseTexture[2];
texture2D   g_NormalTexture;
texture2D   g_MaskDiffuseTexture;

vector      g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector      g_vMatrlSpecular = vector(0.1f, 0.1f, 0.1f, 0.1f);

texture2D   g_MaskTexture[4] : register(t8);

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(float4(In.vPosition, 1.f), matWVP);

    return Out;
}

struct VS_OUT_SHADOW
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
//    float4 vEmissive : SV_TARGET3;
//    float4 vDistortion : SV_TARGET4;
    float4 vSpecular : SV_TARGET3;
    float4 vAmbient : SV_TARGET4;
};


PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    //vector vMaskDiffiuse = g_MaskDiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse * (1.f - vMask) + vMaskDiffiuse * vMask;
    
    //if (Out.vDiffuse.a < 0.1f)
    //    discard;
    
    //float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    //Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
    Out.vNormal = vNormalDesc * 2.f -1.f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSpecular = g_vMatrlSpecular;
    Out.vAmbient = g_vMatrlAmbient;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_ALPHA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    //Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
    
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_FOCUS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    Out.vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse *= float4(0.5f, 1.f, 0.5f, 1.f);
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    return Out;
}
struct PS_OUT_DEBUG
{
    float4 vDiffuse : SV_TARGET0;
};

PS_OUT_DEBUG PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_DEBUG Out = (PS_OUT_DEBUG) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    //vector vMaskDiffiuse = g_MaskDiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse * (1.f - vMask) + vMaskDiffiuse * vMask;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL();
    }

    pass AlphaNotDiscard // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass AlphaNotDiscardNonCull // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass SelectedObject // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_FOCUS();
    }

    pass DebugRender // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }
}