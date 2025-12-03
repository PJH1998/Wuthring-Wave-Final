// UI용
#include "Engine_Shader_State.hlsli"

// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
//Texture2D g_Texture;
//float g_AlphaStrength;

float4 g_BaseColor; // 기본 색
float4 g_HeadColor; // 시작 색
float4 g_TailColor; // 끝 색

// ==============================
// * Vertex Shader
// ==============================

struct VS_IN
{
    float3 vPosition        : POSITION;
    float fCurveProgress    : TEXCOORD0; // 0~1
};

struct VS_OUT
{
    float4 vPosition        : SV_POSITION;
    float fCurveProgress    : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vWorld = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    Out.vPosition = mul(vView, g_ProjMatrix);

    Out.fCurveProgress = In.fCurveProgress;

    return Out;
}

// ==============================
// * Pixel Shader
// ==============================

struct PS_IN
{
    float4 vPosition        : SV_POSITION;
    float fCurveWidth       : TEXCOORD0;
    float fCurveProgress    : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor;
};

PS_OUT PS_MAIN(PS_IN In) : SV_Target
{
    PS_OUT Out = (PS_OUT) 0;
    
    // 곡선 진행도에 따른 그라디언트
    float4 gradColor = lerp(g_HeadColor, g_TailColor, In.fCurveProgress);

    // 베이스 색 곱해주기
    float4 outColor = g_BaseColor * gradColor;

    // 노이즈, 디스토션, glow 등은 여기서?
    Out.vColor = outColor;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}