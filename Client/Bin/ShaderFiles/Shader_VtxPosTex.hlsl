#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture;

float g_rotationSpeed = 2.0f; // 전체 회전 속도 (양수: 시계 방향 효과*)
float g_swirlStrength = 3.0f; // 휘어짐 강도 (클수록 많이 꼬임)
float g_suctionSpeed = 0.5f; // 빨려 들어가는 속도 (텍스처 줌인 효과)
float g_TotalTime;
float4 vColor = float4(1.f, 1.f, 1.f, 1.f);
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

struct VS_OUT_POTAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vTempTexcoord : TEXCOORD1;
};

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

    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MOUSE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (0.5f >= Out.vColor.a)
        discard;
    
    if(0.f == Out.vColor.r)
        discard;
    
    return Out;
}

struct PS_IN_POTAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vTempTexcoord : TEXCOORD1;
};
struct PS_OUT_POTAL
{
    float4 vColor : SV_TARGET0;
    float4 vEmissive: SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    
};
PS_OUT_POTAL PS_POTAL2(PS_IN In)
{
    PS_OUT_POTAL Out = (PS_OUT_POTAL) 0;
    float2 CenterUV = In.vTexcoord - float2(0.5f, 0.5f);
    

    float Theta = (g_TotalTime * 1.f);
    float C = cos(Theta);
    float S = sin(Theta);
    float2 NewTexcoord;
    NewTexcoord.x = CenterUV.x * C - CenterUV.y * S;
    NewTexcoord.y = CenterUV.x * S + CenterUV.y * C;
    NewTexcoord += float2(0.5f, 0.5f);
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, NewTexcoord);
    vector vMask = g_MaskTexture.Sample(DefaultSampler, NewTexcoord);
    
        Out.vDistortion = float4(vMask.rgb, 0.6f);
    float Alpha = g_TotalTime / 5.f;
    if (Alpha>=1.f)
        Alpha = 1.f;
    Out.vColor = vDiffuse;
    Out.vColor = float4(0.5f, 0.5f, 0.f, Alpha);
    Out.vColor *= vMask;
    Out.vEmissive = Out.vColor;

        float Dist = distance(In.vTexcoord, float2(0.5f, 0.5f));
    if (Dist >= 0.5f)
        discard;
    
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

    pass Mouse // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MOUSE();
    }

    pass Potal // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POTAL2();
    }
}