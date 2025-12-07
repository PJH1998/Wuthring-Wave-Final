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

VS_OUT_POTAL VS_POTAL(VS_IN In)
{
    VS_OUT_POTAL Out = (VS_OUT_POTAL) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    float2 vCenter = float2(0.5f, 0.5f);
    float2 TempvTexcoord = In.vTexcoord - vCenter;
    Out.vTexcoord = In.vTexcoord;
    In.vTexcoord.x = TempvTexcoord.x * cos(g_TotalTime) - TempvTexcoord.y * sin(g_TotalTime);
    In.vTexcoord.y = TempvTexcoord.x * sin(g_TotalTime) + TempvTexcoord.y * cos(g_TotalTime);
    
    Out.vTempTexcoord = In.vTexcoord + vCenter;
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


PS_OUT PS_POTAL(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 DiscardColor = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    if (DiscardColor.r > 0.7f)
        discard;
    // -----------------------------------------------------------
    // 2. 마스크 UV 변형 (회전 + 꼬임 + 빨려듦)
    // -----------------------------------------------------------
    
    // UV 중심 이동 (0.5, 0.5 -> 0, 0)
    float2 center = float2(0.5f, 0.5f);
    float2 centeredUV = In.vTexcoord - center;

    // 극좌표 변환
    float radius = length(centeredUV);
    float angle = atan2(centeredUV.y, centeredUV.x);

    // [파라미터 설정]
    float rotationSpeed = 2.0f; // 마스크 회전 속도
    float swirlStrength = 3.0f; // 휘어짐 강도
    float suctionSpeed = 0.5f; // 마스크가 안으로 빨려 들어가는 속도

    // [회전 및 꼬임 계산]
    // 중심에 가까울수록(radius가 작을수록) 더 많이 회전
    float angleOffset = (g_TotalTime * rotationSpeed) + (swirlStrength * (1.0f - radius));
    float distortedAngle = angle - angleOffset; // 시계 방향 회전

    // [빨려 들어가는 효과 계산]
    // 반지름에 시간을 더해 샘플링 위치를 바깥쪽으로 계속 이동시킴 -> 시각적으로는 안으로 빨려 듦
    float distortedRadius = radius + (g_TotalTime * suctionSpeed);

    // 극좌표 -> 직교 좌표 복원
    float2 distortedUV;
    sincos(distortedAngle, distortedUV.y, distortedUV.x);
    distortedUV *= distortedRadius; // 변형된 반지름 적용
    distortedUV += center; // 중심점 원복

    // -----------------------------------------------------------
    // 3. 마스크 텍스처 샘플링 (변형된 UV 사용)
    // -----------------------------------------------------------
    float4 maskColor = g_MaskTexture.Sample(DefaultSampler, distortedUV);
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, distortedUV);
    // -----------------------------------------------------------
    // 4. 최종 합성
    // -----------------------------------------------------------
    // 디퓨즈 색상에 왜곡된 마스크의 값을 적용합니다.
    // 마스크의 Red 채널을 알파(투명도)로 쓴다고 가정:
    // (옵션) 마스크 텍스처 자체의 색상도 곱하고 싶다면:
    Out.vColor *= maskColor;

    
    return Out;
}

struct PS_IN_POTAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vTempTexcoord : TEXCOORD1;
};
PS_OUT PS_POTAL2(PS_IN_POTAL In)
{
    PS_OUT Out = (PS_OUT) 0;
    //vector vMask = g_MaskTexture.Sample(DefaultSampler, In.vTempTexcoord);

    //if (vMask.a < 0.1f)
    //    discard;
    
    //vector vDiffuseMask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    //Out.vColor = vColor;
    
    //if (vDiffuseMask.a == 1.f)
    //    discard;
    
    //Out.vColor *= vDiffuseMask;
    
    //if (distance(In.vTempTexcoord, 0.5f) > 0.5f)
    //    discard;
    
    
    
    // -----------------------------------------------------------
    // 1. 극좌표계 변환 및 파라미터 설정 (이전과 동일)
    // -----------------------------------------------------------
    //float2 center = float2(0.5f, 0.5f);
    //float2 centeredUV = In.vTexcoord - center;
    //float radius = length(centeredUV);
    //float angle = atan2(centeredUV.y, centeredUV.x);

    //float rotationSpeed = 1.5f;
    //float swirlStrength = 4.0f;
    //float suctionSpeed = 0.8f;
    
    //// -----------------------------------------------------------
    //// 2. 동적 UV 변형 계산 (이전과 동일)
    //// -----------------------------------------------------------
    //float angleOffset = (g_TotalTime * rotationSpeed) + (swirlStrength * (1.0f - smoothstep(0.0f, 0.5f, radius)));
    //float distortedAngle = angle - angleOffset;

    //float distortedRadius = radius + (g_TotalTime * suctionSpeed);

    //float2 distortedUV;
    //sincos(distortedAngle, distortedUV.y, distortedUV.x);
    //distortedUV *= distortedRadius;
    //distortedUV += center;

    //// -----------------------------------------------------------
    //// 3. 텍스처 샘플링
    //// -----------------------------------------------------------
    //// 변형된 UV로 마스크 텍스처를 샘플링합니다.
    //float4 maskSample = g_MaskTexture.Sample(DefaultSampler, distortedUV);

    //// -----------------------------------------------------------
    //// 4. 최종 색상 및 알파 합성 (수정됨)
    //// -----------------------------------------------------------
    
    //// [기본 색상 결정 - 빛 효과 제거]
    //// 이전의 g_GlowColor * g_GlowIntensity 곱셈을 제거했습니다.
    //// 마스크 텍스처가 흑백 이미지이므로, Red 채널 값을 이용해 하얀색 패턴을 만듭니다.
    //float3 finalRGB = float3(1.0f, 1.0f, 1.0f) * maskSample.r;

    //// [깊이감 및 가장자리 처리] (이전과 동일하게 유지)
    //// 1. 중심부 깊이감 표현 (가운데로 갈수록 어두워짐)
    //float depthFade = smoothstep(0.0f, 0.3f, radius);
    //finalRGB *= depthFade;

    //// 2. 외곽 원형 자르기
    //float outerEdgeMask = 1.0f - smoothstep(0.45f, 0.5f, radius);
    
    //// [최종 출력]
    //// 알파 값은 마스크 자체의 밝기와 외곽 마스크에 따라 결정됩니다.
    //// 마스크의 검은 부분은 투명하게, 하얀 패턴 부분만 나타납니다.
    //float finalAlpha = maskSample.r * outerEdgeMask;

    //// (선택 사항) 중심부로 갈수록 투명해지게 하려면 아래 주석을 푸세요.
    //// finalAlpha *= depthFade; 

    //return float4(finalRGB, finalAlpha);
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
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POTAL();
    }

    pass Potal2 // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_POTAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POTAL2();
    }
}