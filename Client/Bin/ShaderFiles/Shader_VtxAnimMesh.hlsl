#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float g_fLightFar;

texture2D g_DiffuseTexture;
texture2D g_SecondDiffuseTexture;
texture2D g_NormalTexture;
texture2D g_MaskTexture[4] : register(t8);

float g_fDissolveRate = 0.f;
float g_fFlowRate = 0.f;

matrix g_BoneMatrices[512];

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
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
    
    matrix matBone, matBW, matWV, matWVP;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matBone = 
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    float4 vTangent = mul(float4(In.vTangent, 0.f), matBone);
    float4 vBinormal = mul(float4(In.vBinormal, 0.f), matBone);
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTangent = normalize(mul(vTangent, g_WorldMatrix));
    Out.vBinormal = normalize(mul(vBinormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(vPosition, matWVP);

    return Out;
}

struct VS_OUT_SHADOW
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matrix matBone =
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    Out.vPosition = mul(vPosition, matWVP);
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

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_NORMALTEX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector NormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    //float3 vNormal = NormalDesc.xyz * 2.f - 1.f;
    float3 vNormal = NormalDesc.xyz;
    
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    Out.vNormal = vector(mul(vNormal, WorldMatrix) * 0.5f + 0.5f, 0.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_ZANNI_HAIR(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector vMask0 = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (vMask0.a == 0.f)
        discard;
    
    // Dissolve
    if(vMask0.g > 0.f)
    {
        vector vMask1 = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
        float fDissolve = g_fDissolveRate * vMask0.g * 0.25f;
        if (vMask1.r > 1.f - fDissolve)
            discard;
    }

    float2 vFlowUV = In.vTexcoord;
    float2 vDiffuseUV = In.vTexcoord;
    // Flow
    if(vMask0.r > 0.f)
    {
        vFlowUV.y -= g_fFlowRate;
        vector vMask2 = g_MaskTexture[2].Sample(DefaultSampler, vFlowUV);
        vDiffuseUV.y -= vMask2.r;
    }
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, vDiffuseUV);
    
    float3 vEmissiveColor = float3(0.1f, 0.1f, 0.1f);
    // Emissive
    Out.vDiffuse.rgb += (vEmissiveColor * (1.f - vMask0.b)) * 0.5f;
    
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    float fBright = g_fLuminence[0] * Out.vDiffuse.r + g_fLuminence[1] * Out.vDiffuse.g + g_fLuminence[2] * Out.vDiffuse.b;
    if (fBright > g_fEmissiveThreshold)
        Out.vEmissive = Out.vDiffuse * 0.4f;
    
    return Out;
}

PS_OUT PS_ZANNI_HAIRFX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.a = 0.1f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    vector vMask0 = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (vMask0.a > 0.1f)
        Out.vDistortion = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord) * 0.005f;
    else
        Out.vDistortion = 0.f;
    
    return Out;
}

PS_OUT PS_LUPA_TAIL(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector vMask0 = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (vMask0.a == 0.f)
        discard;
    
    // Dissolve
    if (vMask0.g > 0.f)
    {
        vector vMask1 = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
        float fDissolve = g_fDissolveRate * vMask0.g * 0.25f;
        if (vMask1.r > 1.f - fDissolve)
            discard;
    }

    float2 vFlowUV = In.vTexcoord;
    float2 vDiffuseUV = In.vTexcoord;
    // Flow
    if (vMask0.r > 0.f)
    {
        vFlowUV.y -= g_fFlowRate;
        vector vMask2 = g_MaskTexture[2].Sample(DefaultSampler, vFlowUV);
        vDiffuseUV.y -= vMask2.r;
    }
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, vDiffuseUV);
    
    float3 vEmissiveColor = float3(0.1f, 0.1f, 0.1f);
    // Emissive
    Out.vDiffuse.rgb += (vEmissiveColor * (vMask0.b)) * 0.5f;
    
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    float fBright = g_fLuminence[0] * Out.vDiffuse.r + g_fLuminence[1] * Out.vDiffuse.g + g_fLuminence[2] * Out.vDiffuse.b;
    if (fBright > g_fEmissiveThreshold)
        Out.vEmissive = Out.vDiffuse * pow(1.2f, 1.2f);
    
    return Out;
}

PS_OUT PS_LUPA_FLAG(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector vMask0 = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse0 = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vDiffuse0.r < 0.1f)
        discard;
    
    vector vDiffuse1 = g_SecondDiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse0 * vMask0.a + vDiffuse1 * (1.f - vMask0.a);
    
    Out.vDistortion = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord) * 0.005f;

    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    return Out;
}


struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

struct PS_OUT_LIGHTDEPTH
{
    float4 vLightDepth : SV_TARGET0;
};

PS_OUT_LIGHTDEPTH PS_SHADOW(PS_IN_SHADOW In)
{
    PS_OUT_LIGHTDEPTH Out = (PS_OUT_LIGHTDEPTH) 0;
    
    //Out.vLightDepth = float4(In.vProjPos.w / g_fLightFar, 0.f, 0.f, 0.f);
    Out.vLightDepth = float4(In.vProjPos.z, 0.f, 0.f, 0.f);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultNormal // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass NormalTexture // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NORMALTEX();
    }

    pass Shadow // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHADOW();
    }

    pass Zanni_HAIR // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ZANNI_HAIR();
    }

    pass Zanni_HAIR_FX // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ZANNI_HAIRFX();
    }

    pass Lupa_Tail// 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUPA_TAIL();
    }

    pass Lupa_Flag // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUPA_FLAG();
    }
}