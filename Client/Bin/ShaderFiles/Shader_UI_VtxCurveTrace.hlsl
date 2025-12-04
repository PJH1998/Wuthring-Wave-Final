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

float g_AlphaScale = 1.f;

// ==============================
// * Vertex Shader
// ==============================

struct VS_IN
{
    float3 vPosition        : POSITION;
    float fCurveWidth       : TEXCOORD0;
    float fCurveProgress    : TEXCOORD1; // 0~1
    //float3 vPadding         : TEXCOORD2;
};

struct VS_OUT
{
    float4 vPosition        : SV_POSITION;
    float fCurveWidth       : TEXCOORD0;
    float fCurveProgress    : TEXCOORD1;
};

VS_OUT VS_MAIN_OLD(VS_IN In)
{
    VS_OUT Out;

    float4 vWorld       = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);
    float4 vView        = mul(vWorld, g_ViewMatrix);                     
    Out.vPosition       = mul(vView, g_ProjMatrix);                      

    Out.fCurveWidth     = In.fCurveWidth;
    Out.fCurveProgress  = In.fCurveProgress;

    return Out;
}

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    // [디버깅] 행렬 곱셈 모두 주석 처리
    // float4 vWorld = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);
    // float4 vView = mul(vWorld, g_ViewMatrix);
    // Out.vPosition = mul(vView, g_ProjMatrix);

    // [강제 출력] 입력받은 로컬 좌표를 그대로 화면 좌표로 사용
    // 화면 중앙에 작게 나오도록 10으로 나눔 (Input이 -5 ~ 5 이므로 -0.5 ~ 0.5가 됨)
    Out.vPosition = float4(In.vPosition.x * 0.1f, In.vPosition.y * 0.1f, 0.0f, 1.0f);

    Out.fCurveWidth = In.fCurveWidth;
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
    float fCurveProgress    : TEXCOORD1;
};

struct PS_OUT
{
    float4 vColor;
};

PS_OUT PS_MAIN(PS_IN In) : SV_Target
{
    PS_OUT Out;
       
    // 진행도에 따른 색 그라디언트
    float4 gradColor = lerp(g_HeadColor, g_TailColor, In.fCurveProgress);

    float4 col = g_BaseColor * gradColor;

    // 좌우 가장자리 알파 페이드 (중앙 밝게, 끝은 어둡게)
    float alphaEdge = 1.0 - abs(In.fCurveWidth - 0.5) * 2.0; // u=0.5 -> 1, u=0/1 -> 0
    alphaEdge = saturate(alphaEdge);

    col.a *= alphaEdge * g_AlphaScale;
    Out.vColor = col;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}