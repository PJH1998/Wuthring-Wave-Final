#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture;
texture2D g_MaskTexture;

float   g_Sweep;
float   g_SweepWitdh;
float   g_Soft = 0.3f;
int     g_Dir;

float   g_ColorSpeed = 0.4f;

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

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;             //지금은 그냥 그리기
    //float4 vDiffuse : SV_TARGET0;
    //float4 vNormal : SV_TARGET1;
    //float4 vDepth : SV_TARGET2;
    //float4 vEmissive : SV_TARGET3;
    //float4 vDistortion : SV_TARGET4;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
   
    if(Out.vColor.a < 0.3f)
        discard;
    
    return Out;
}

PS_OUT PS_TrailDefault(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    
    //마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
    if (vMask.a < 0.3f)
        discard;
    
    float2 ColorUV = In.vTexcoord;
    ColorUV -= g_Sweep;     //X로 긴 텍스처니까 색상 움직이듯 보여질려면 이렇게 해야하나?
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, ColorUV);
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
    float fVisible = fTailFad * fHeadFad;
   
    float alpha = fVisible * vMask.a;
    
    Out.vColor = float4(vColor.rgb * alpha, alpha);
    
    return Out;
}

PS_OUT PS_TraillTest(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    
    //마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
    if (vMask.a < 0.3f)
        discard;
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
    float fVisible = fTailFad * fHeadFad;
    
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    float fAlpha = fVisible * vMask.a;
    
    Out.vColor = float4(vColor.rgb * fAlpha, fAlpha);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TrailDefault();
    }

    pass TestPass // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillTest();
    }
}
