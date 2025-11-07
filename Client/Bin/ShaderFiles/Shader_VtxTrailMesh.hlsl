#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture;
texture2D g_MaskTexture;

//색상
float   g_Sweep;
float   g_SweepWitdh;
float   g_Soft = 0.3f;  //툴에서 받아올 수 있게 해주자.

//공용
int     g_Dir;          //안쓰는중
float   g_Time;
float   g_Alpha;
int   g_MaskFlag;     // 0이면 R로, 1이면 알파로

//밝기 죽이기?
float g_ColorGain;      // 밝기 스케일 0~1
float g_ColorGamma;     // 톤 커버, (1 == 그대로, >1 어두워지게)

//마스크
float2 g_MaskSweep;
float2 g_MaskOffset;
float g_MaskMix; 
float g_MaskSoft;

//임시
float g_MaskSpeed = 1.f;
float g_ColorSpeed;

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
    float4 vDiffuse : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
   
    if (Out.vDiffuse.a < 0.3f)
        discard;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    return Out;
}

PS_OUT PS_TrailDefault(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    //float2 UV = In.vTexcoord;
    
    //float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;
    
    //if( MaskR < 0.2f)
    //    discard;
    
    //float fY = 1.f - g_MaskSweep;
    //float fVisibleY;
    
    //fVisibleY = 1.f - step(fY, UV.y);
    
    //float fVisibleX;
    
    //float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
    //float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
    //fVisibleX = fTailFad * fHeadFad;
    
    //float2 ColorUV = In.vTexcoord;
    //ColorUV -= g_Sweep;
    //float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, ColorUV);
    
    //float fAlpha = fVisibleY * fVisibleX * MaskR;
    
    //if(fAlpha < 0.2f)
    //    discard;
    
    //Out.vDiffuse = float4(vColor.arb, fAlpha);
    
    //float fWeight = Luminance(Out.vDiffuse.xyz);
    
    //if (fWeight >= g_fEmissiveThreshold)
    //    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    
 
    
    float2 MaskUV = In.vTexcoord;
    
    MaskUV -= g_Sweep;
    
    float4 vMask = g_MaskTexture.Sample(ClampSampler, MaskUV);
    
    float MaskAlpha;
    
    if( g_MaskFlag == 1)
    {
        if(vMask.a < 0.3f)
            discard;
        
        MaskAlpha = vMask.a;

    }
    else
    {
        if (vMask.r < 0.35f)
            discard;
        
        MaskAlpha = vMask.r;
    }

    float2 ColorUV = In.vTexcoord;
    ColorUV -= g_Sweep; //X로 긴 텍스처니까 색상 움직이듯 보여질려면 이렇게 해야하나?
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, ColorUV);
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;
    
    
    float alpha;
    
    if(g_Dir == 1)
    {
        float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
        float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
        float fVisible = fTailFad * fHeadFad;
   
        alpha = fVisible * MaskAlpha;
    }
    else
    {
        float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.x);
    
        float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep,1 - In.vTexcoord.x);
    
        float fVisible = fTailFad * fHeadFad;
   
        alpha = fVisible * MaskAlpha;
    }
    
    if (alpha < 0.3f)
        discard;
    
    Out.vDiffuse = float4(vColor.rgb, alpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    
    return Out;
}

PS_OUT PS_TraillTest(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    float2 UV = In.vTexcoord;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;
    
    if (MaskR < 0.35f)
        discard;
    
    float fY = 1.f - g_MaskSweep;
    float fVisibleY;
    
    fVisibleY = 1.f - step(fY, UV.y);
    
    float fVisibleX;
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.x);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.x);
    
    fVisibleX = fTailFad * fHeadFad;
    
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;
    
    float fAlpha = fVisibleY * fVisibleX * MaskR;
    
    if (fAlpha < 0.2f)
        discard;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse *= g_Alpha;
    
    //float4 vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);
    
    ////마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
    //if (vMask.r < 0.35f)
    //    discard;
    
    //// 1 - x 
    //float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.x);
    
    //float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.x);
    
    //float fVisible = fTailFad * fHeadFad;
    
    //float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    //float fAlpha = fVisible * vMask.a;
    
    //Out.vDiffuse = float4(vColor.rgb * fAlpha, fAlpha);
    
    //if (Out.vDiffuse.r < 0.35f)      //테스트
    //    discard;
    
    //float fWeight = Luminance(Out.vDiffuse.xyz);
    
    //if (fWeight >= g_fEmissiveThreshold)
    //    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    return Out;
}

PS_OUT PS_Y_OUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    float2 UV = In.vTexcoord;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;
    
    if (MaskR < 0.35f)
        discard;
    
    float fY = 1.f - g_MaskSweep;
    float fVisibleY;
    
    fVisibleY = 1.f - step(fY, UV.y);
    
    float fVisibleX;
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.y);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.y);
    
    fVisibleX = fTailFad * fHeadFad;
    
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;
    
    float fAlpha = fVisibleY * fVisibleX * MaskR;
    
    if (fAlpha < 0.2f)
        discard;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse *= g_Alpha;
    
    return Out;
}

PS_OUT PS_Y_IN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    float2 UV = In.vTexcoord;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;
    
    if (MaskR < 0.35f)
        discard;
    
    float fY = 1.f - g_MaskSweep;
    float fVisibleY;
    
    fVisibleY = 1.f - step(fY, UV.y);
    
    float fVisibleX;
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.y);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.y);
    
    fVisibleX = fTailFad * fHeadFad;
    
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;
    
    float fAlpha = fVisibleY * fVisibleX * MaskR;
    
    if (fAlpha < 0.2f)
        discard;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse *= g_Alpha;
    
    return Out;
}

PS_OUT PS_TraillTestA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 UV = In.vTexcoord;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;
    
    if (MaskR < 0.35f)
        discard;
    
    float fY = 1.f - g_MaskSweep;
    float fVisibleY;
    
    fVisibleY = 1.f - step(fY, UV.y);
    
    float fVisibleX;
    
    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
    float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
    fVisibleX = fTailFad * fHeadFad;
    
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    float fAlpha = fVisibleY * fVisibleX * MaskR;
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;
    
   if (fAlpha < 0.2f)
       discard;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    //float4 vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);
    
    ////마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
    //if (vMask.r < 0.35f)
    //    discard;
    
    //// 1x 
    //float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);
    
    //float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);
    
    //float fVisible = fTailFad * fHeadFad;
    
    //float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    //float fAlpha = fVisible * vMask.a;
    
    //Out.vDiffuse = float4(vColor.rgb * fAlpha, fAlpha);
    
    //if (Out.vDiffuse.r < 0.35f)      //테스트
    //    discard;
    
    //float fWeight = Luminance(Out.vDiffuse.xyz);
    
    //if (fWeight >= g_fEmissiveThreshold)
    //    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse *= g_Alpha;
    
    return Out;
}

// ==Test==
PS_OUT PS_TraillDesh(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 UV = In.vTexcoord;
    
    UV.y -= g_MaskSpeed * g_Sweep;      //타임
    UV.y = frac(UV.y);
    
    float4 mask = g_MaskTexture.Sample(ClampSampler, UV);
    
    if(mask.r < 0.35f)
        discard;
    
    Out.vDiffuse = mask * 0.6f;
    
    if (Out.vDiffuse.r < 0.2f)      //테스트
        discard;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse.rgb *= Out.vDiffuse.a;
    
    return Out;
}


PS_OUT PS_TraillDeshB(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    
    float2 UV = In.vTexcoord;
    
    UV.y += g_MaskSpeed * g_Sweep; //타임
    UV.y = frac(UV.y);
    
    float4 mask = g_MaskTexture.Sample(ClampSampler, UV);
    
    if (mask.r < 0.35f)
        discard;
    
    Out.vDiffuse = mask * 0.6f;
    
    if (Out.vDiffuse.r < 0.2f)      //테스트
        discard;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vDiffuse.rgb *= Out.vDiffuse.a;
    
    return Out;
}
// ==Test==
technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TrailDefault();
    }

    pass TestPass // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillTest();
    }

    pass TestPassA // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillTestA();
    }

    pass PS_TraillTest //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillDesh();
    }

    pass PS_TraillTestB //4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillDeshB();
    }

    pass TrailYOut// 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Y_OUT();
    }

    pass TrailYIn // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Y_IN();
    }

    pass PS_Debug //7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

}
