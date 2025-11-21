#include "Engine_Shader_Defines.hlsli"
typedef row_major matrix matrix_rm;
row_major matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float g_fLightFar;

Texture2D g_DiffuseTexture;
Texture2D g_SecondDiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture[4] : register(t8);

row_major matrix g_ShadowViewMatrix[4];
row_major matrix g_ShadowProjMatrix[4];

float g_fOutLineRadius = 0.001f;
float4 g_vOutLineColor = float4(0.3f, 0.15f, 0.f, 1.f);

float g_fDissolveRate = 0.f;
float g_fFlowRate = 0.f;

//matrix g_BoneMatrices[512];
row_major matrix g_OffsetMatrices[512];
bool g_HasNormal = false;
uint g_iNumBones;

cbuffer GlobalConstants
{
    int g_iNumBlendWeightsToUse = 2; 
}

StructuredBuffer<matrix_rm> g_CombinedBoneMatrices;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
    float2 vTexcoord : TEXCOORD0;
    
    row_major float4x4 TransformMatrix : WORLD;
    uint iBaseIndex : INSTANCEID;
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
    
    uint ibaseIndex = In.iBaseIndex * g_iNumBones;
    matrix_rm matBone, matBW, matVP;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matBone =
    mul(g_OffsetMatrices[In.vBlendIndex.x], g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.x)]) * In.vBlendWeight.x +
    mul(g_OffsetMatrices[In.vBlendIndex.y], g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.y)]) * In.vBlendWeight.y +
    mul(g_OffsetMatrices[In.vBlendIndex.z], g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.z)]) * In.vBlendWeight.z +
    mul(g_OffsetMatrices[In.vBlendIndex.w], g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.w)]) * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    vPosition = mul(vPosition, In.TransformMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    float4 vTangent = mul(float4(In.vTangent, 0.f), matBone);
    float4 vBinormal = mul(float4(In.vBinormal, 0.f), matBone);
    
    matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matVP);
    Out.vNormal = normalize(mul(vNormal, In.TransformMatrix));
    Out.vTangent = normalize(mul(vTangent, In.TransformMatrix));
    Out.vBinormal = normalize(mul(vBinormal, In.TransformMatrix));
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

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vPBR.y = 0.2f;
    Out.vPBR.z = 1.f;
    
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
    Out.vPBR.y = 0.2f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_NPCFACE(PS_IN In)
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
    Out.vPBR.y = 0.2f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_AUGUSTA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    // 
    float4 vNormal = 0.f;
    
    if(g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        //Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        //Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    //Test
    Out.vPBR.a = 1.f;

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

//PS_OUT PS_ROVER(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//
//    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
//    
//    // 
//    float4 vNormal = 0.f;
//    
//    if (g_HasNormal)
//    {
//        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
//        vNormal = normalize(vNormalDesc * 2.f - 1.f);
//        
//        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
//            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
//            
//        float3 vTangent = In.vTangent.xyz;
//        float3 vBinormal = In.vBinormal.xyz * -1.f;
//        float3 vInNormal = In.vNormal.xyz;
//        
//        float3x3 WorldMatrix;
//        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
//        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
//        
//        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    else
//    {
//        vNormal = In.vNormal;
//        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    
//    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
//    
//    //Test
//    Out.vPBR.a = 1.f;
//
//    vNormal.xyz = vNormal * 0.5f + 0.5f;
//    
//    Out.vNormal = vNormal;
//    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
//    Out.vDepth.y = In.vProjPos.w;
//    Out.vDepth.z = 1.f;
//    
//    
//    return Out;
//}

//PS_OUT PS_GALBRENA(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//
//    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
//    
//    // 
//    float4 vNormal = 0.f;
//    
//    if (g_HasNormal)
//    {
//        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
//        vNormal = normalize(vNormalDesc * 2.f - 1.f);
//        
//        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
//            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
//            
//        float3 vTangent = In.vTangent.xyz;
//        float3 vBinormal = In.vBinormal.xyz * -1.f;
//        float3 vInNormal = In.vNormal.xyz;
//        
//        float3x3 WorldMatrix;
//        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
//        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
//        
//        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    else
//    {
//        vNormal = In.vNormal;
//        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    
//    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
//    
//    //Test
//    Out.vPBR.a = 1.f;
//
//    vNormal.xyz = vNormal * 0.5f + 0.5f;
//    
//    Out.vNormal = vNormal;
//    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
//    Out.vDepth.y = In.vProjPos.w;
//    Out.vDepth.z = 1.f;
//    
//    
//    return Out;
//}

//PS_OUT PS_LOGO_ROVER(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//
//    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
//    
//    // 
//    float4 vNormal = 0.f;
//    
//    if (g_HasNormal)
//    {
//        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
//        vNormal = normalize(vNormalDesc * 2.f - 1.f);
//        
//        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
//            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
//            
//        float3 vTangent = In.vTangent.xyz;
//        float3 vBinormal = In.vBinormal.xyz * -1.f;
//        float3 vInNormal = In.vNormal.xyz;
//        
//        float3x3 WorldMatrix;
//        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
//        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
//        
//        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    else
//    {
//        vNormal = In.vNormal;
//        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
//        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
//    }
//    
//    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
//    
//    //Test
//    Out.vPBR.a = 1.f;
//
//    vNormal.xyz = vNormal * 0.5f + 0.5f;
//    
//    Out.vNormal = vNormal;
//    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
//    Out.vDepth.y = In.vProjPos.w;
//    Out.vDepth.z = 1.f;
//    
//    
//    return Out;
//}





/*------------------------------------------------SHADOW BEGIN------------------------------------------------*/

struct VS_OUT_SHADOW
{
    float4 vPosition : POSITION;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    uint ibaseIndex = In.iBaseIndex * g_iNumBones;
    
    uint iX = max(In.vBlendIndex.x, g_iNumBlendWeightsToUse);
    uint iY = max(In.vBlendIndex.y, g_iNumBlendWeightsToUse);
    uint iZ = max(In.vBlendIndex.z, g_iNumBlendWeightsToUse);
    uint iW = max(In.vBlendIndex.w, g_iNumBlendWeightsToUse);
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matrix matBone =
   g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.x)] * g_OffsetMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.y)] * g_OffsetMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.z)] * g_OffsetMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.w)] * g_OffsetMatrices[In.vBlendIndex.w] * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    
    Out.vPosition = mul(vPosition, g_WorldMatrix);

    return Out;
}

struct GS_IN
{
    float4 vPosition : POSITION;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    uint iIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(12)]
void GS_SHADOW(triangle GS_IN In[3], inout TriangleStream<GS_OUT> Vertices)
{
    for (int Face = 0; Face < 4; Face++)
    {
        GS_OUT Out = (GS_OUT) 0;
        Out.iIndex = Face;
        
        matrix matVP;
        matVP = mul(g_ShadowViewMatrix[Face], g_ShadowProjMatrix[Face]);

        for (int i = 0; i < 3; i++)
        {
            Out.vPosition = mul(In[i].vPosition, matVP);
            Vertices.Append(Out);
        }
        Vertices.RestartStrip();
    }
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
};

void PS_SHADOW(PS_IN_SHADOW In)
{
    if (In.vPosition.z >= 1.f)
        discard;
}
/*------------------------------------------------SHADOW END------------------------------------------------*/


/*------------------------------------------------OULTINE BEGIN------------------------------------------------*/

struct VS_OUT_OUTLINE
{
    float4 vPosition : SV_POSITION;
    bool IsDraw : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_OUTLINE VS_OUTLINE(VS_IN In)
{
    VS_OUT_OUTLINE Out = (VS_OUT_OUTLINE) 0;
    uint ibaseIndex = In.iBaseIndex * g_iNumBones;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matrix matBone =
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.x)] * g_OffsetMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.y)] * g_OffsetMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.z)] * g_OffsetMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_CombinedBoneMatrices[(ibaseIndex + In.vBlendIndex.w)] * g_OffsetMatrices[In.vBlendIndex.w] * fWeightW;
    
    matrix matWV;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
   
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    
    vector vViewPos = mul(vPosition, matWV);
    
    vector vViewNormal = normalize(mul(vNormal, matWV));
   
    if(vViewNormal.z < 0.f)
    {
        vViewNormal.z *= -1.f;
    }
    
    vViewNormal = normalize(float4(vViewNormal.x, vViewNormal.y, vViewNormal.z * 0.01f, 0.f));
    
    vector vOutLinePos = vViewPos + (vViewNormal * g_fOutLineRadius);
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    Out.IsDraw = true;
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN_OUTLINE
{
    float4 vPosition : SV_POSITION;
    bool IsDraw : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT_OUTLINE
{
    float4 vColor : SV_TARGET0;
    float4 vDepth : SV_TARGET1;
    float4 vPBR : SV_TARGET2;
};

PS_OUT_OUTLINE PS_OUTLINE(PS_IN_OUTLINE In)
{
    PS_OUT_OUTLINE Out = (PS_OUT_OUTLINE) 0;

    if (In.IsDraw)
    {
        Out.vColor = g_vOutLineColor;
        Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
        Out.vDepth.y = In.vProjPos.w;
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }
    else
        discard;
        
    return Out;
}

/*------------------------------------------------OULTINE END------------------------------------------------*/

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

    pass Augusta // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_AUGUSTA();
    }

    pass Shadow // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = compile gs_5_0 GS_SHADOW();
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
 
    pass Outline //4
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OUTLINE();
    }
 
    pass NPCFace // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NPCFACE();
    }

    //pass Rover // 5
    //{
    //    SetRasterizerState(RS_Cull_None);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
    //
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_ROVER();
    //}
    //
    //pass Galbrena // 6
    //{
    //    SetRasterizerState(RS_Cull_None);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
    //
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_GALBRENA();
    //}
    //
    //pass NormalYellow // 7
    //{
    //    SetRasterizerState(RS_Cull_None);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
    //
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_NORMALTEX();
    //}
    //
    //pass LogoRover // 8
    //{
    //    SetRasterizerState(RS_Cull_None);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
    //
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_LOGO_ROVER();
    //}

  
}