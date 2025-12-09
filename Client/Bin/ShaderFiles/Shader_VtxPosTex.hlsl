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
PS_OUT PS_POTAL2(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 CenterUV = In.vTexcoord - float2(0.5f, 0.5f);
    

    float Theta = -(g_TotalTime * 1.f);
    float C = cos(Theta);
    float S = sin(Theta);
    float2 NewTexcoord;
    NewTexcoord.x = CenterUV.x * C - CenterUV.y * S;
    NewTexcoord.y = CenterUV.x * S + CenterUV.y * C;
    NewTexcoord += float2(0.5f, 0.5f);
    
    
    //vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, NewTexcoord);
    vector vMask = g_MaskTexture.Sample(DefaultSampler, NewTexcoord);

    //if (vMask.a == 0.f)
    //    discard;
    //Out.vColor = vDiffuse;
    Out.vColor = float4(0.5f, 0.5f, 0.f, 1.f);
    //Out.vColor*= vMask;
    Out.vColor *= (1 - vMask.r);
    
    float Dist = distance(In.vTexcoord, float2(0.5f, 0.5f));
    
    float EdgeSoftness = 0.02f;
    float CircleAlpha = 1.f - smoothstep(0.45f, 0.49f, Dist);
    
    Out.vColor.a *= CircleAlpha;
    if(Out.vColor.a <=0.01f)
        discard;
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

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POTAL2();
    }
}