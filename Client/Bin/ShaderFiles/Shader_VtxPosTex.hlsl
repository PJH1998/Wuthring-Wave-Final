#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;
texture2D g_ColorTexture;
float3 g_vColor;

texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_MaskTexture;

int     g_iSkillIconIndex, g_iNumRow, g_iNumCol;
float  g_fRotationRadian;

float g_fRate;
float g_fAlphaRate = 0.f;

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

VS_OUT VS_SKILL(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);

    Out.vTexcoord.x = (1.f / (float)g_iNumCol) * ((g_iSkillIconIndex % g_iNumCol) + In.vTexcoord.x);
    Out.vTexcoord.y = (1.f / (float)g_iNumRow) * ((g_iSkillIconIndex / g_iNumCol) + In.vTexcoord.y);
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT)0;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT PS_COOLTIME(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    //Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor = float4(g_vColor, 0.f);
    
    float2 vStandard = float2(0.5f, 0.5f);
    float2 vUp = float2(0.f, -1.f);
    float2 vDistance = In.vTexcoord - vStandard;
    float2 vDir = normalize(vDistance);
    
    if (In.vTexcoord.x >= 0.5f)
    {
        if (g_fRotationRadian < 3.14f && cos(g_fRotationRadian) >= dot(vUp, vDir)) // ÄðÅ¸ÀÓ ±¸°£(¾îµÓ°Ô)
            //Out.vColor.rgb -= 0.3f;
            Out.vColor.a = 0.5f;
    }
    else
    {
        if (g_fRotationRadian < 3.14f || cos(g_fRotationRadian) <= dot(vUp, vDir))
            //Out.vColor.rgb -= 0.3f;
            Out.vColor.a = 0.5f;
    }
    
    if (length(vDistance) > 0.5f)
        discard;

    return Out;
}

PS_OUT PS_BAR(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    if(Out.vColor.a < 0.5f)
        discard;
    if(In.vTexcoord.x > g_fRate)
        discard;
    
    return Out;
}

PS_OUT PS_GAUGE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vMask = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    Out.vColor = float4(g_vColor, 1.f);
    
    float2 vStandard = float2(0.5f, 0.5f);
    float2 vUp = float2(0.f, -1.f);
    float2 vDistance = In.vTexcoord - vStandard;
    float2 vDir = normalize(vDistance);
    
    if (In.vTexcoord.x < 0.5f)
    {
        if (g_fRotationRadian > 3.14f || cos(g_fRotationRadian) <= dot(vUp, vDir))
            Out.vColor.a = 1.f;
        else
            discard;
    }
    else
    {
        if (g_fRotationRadian > 3.14f && cos(g_fRotationRadian) > dot(vUp, vDir))
            Out.vColor.a = 1.f;
        else
            discard;
    }
    
    if (vMask.a < 0.5f)
        discard;
    
    return Out;
}

PS_OUT PS_DOT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    if(Out.vColor.a < 0.1f)
        discard;
    
    if(In.vTexcoord.x < g_fRate)
    {
        float fAlpha = Out.vColor.a;
        //Out.vColor = Out.vColor + g_ColorTexture.Sample(DefaultSampler, In.vTexcoord);
        Out.vColor.rgb = g_vColor;
        Out.vColor.a += 0.4f;
    }
        
    
    return Out;
}

PS_OUT PS_ELEMENT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float2 vStandard = float2(0.5f, 0.5f);
    float2 vUp = float2(0.f, -1.f);
    float2 vDistance = In.vTexcoord - vStandard;
    float2 vDir = normalize(vDistance);
    
    if(Out.vColor.r < 0.9f)
    {
        if (In.vTexcoord.x < 0.5f)
        {
            if (g_fRotationRadian > 3.14f || cos(g_fRotationRadian) <= dot(vUp, vDir))
                Out.vColor.rgb = g_vColor;
        }
        else
        {
            if (g_fRotationRadian > 3.14f && cos(g_fRotationRadian) > dot(vUp, vDir))
                Out.vColor.rgb = g_vColor;
        }
    }

    //if (Out.vColor.a < 0.5f)
    //    discard;
    
    return Out;
}

PS_OUT PS_MISSILE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    Out.vColor.a = max(max(Out.vColor.r, Out.vColor.g), Out.vColor.b);
    
    Out.vColor.rg += 0.2f;
    Out.vColor.b += 0.5f;
    
    return Out;
}

PS_OUT PS_FADE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = float4(0.f, 0.f, 0.f, g_fAlphaRate);
    
    return Out;
}

struct PS_OUT_PORTAL
{
    float4 vColor : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
};

PS_OUT_PORTAL PS_PORTAL(PS_IN In)
{
    PS_OUT_PORTAL Out = (PS_OUT_PORTAL) 0;

    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);

    if(vMask.r < 0.1f)
        discard;
    
    Out.vDistortion = vNormal * 0.005f;
    
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

    pass SkillIconPass // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SKILL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass SkillCoolTimePass // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_COOLTIME();
    }

    pass RatePass // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BAR();
    }
    
    pass DotPass // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOT();
    }

    pass GaugePass // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GAUGE();
    }

    pass WorldUI // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BAR();
    }

    pass ElementPass // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ELEMENT();
    }

    pass MissilePass // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MISSILE();
    }

    pass PortalPass // 9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_PORTAL();
    }

    pass FadePass // 10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FADE();
    }
}