#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D   g_DiffuseTexture[2];
texture2D   g_NormalTexture[2];
texture2D   g_MaskDiffuseTexture;
texture2D   g_MetallicTexture;
vector      g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector      g_vMatrlSpecular = vector(0.4f, 0.4f, 0.4f, 0.4f);

texture2D   g_MaskTexture[4] : register(t8);

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;

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

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
};


PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);
    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    Out.vDiffuse.w = 1.f;
    
    //Out.vDiffuse = vDiffuse * (1.f - vMask.r) + vMaskDiffiuse * vMask.g;
    
    //Out.vDiffuse = vDiffuse;
   
    
    Out.vPBR.y = 0.2f;
    
    if (g_IsDynamicObject)
        Out.vPBR.z = 1.f;
    Out.vPBR.a = 1.f;
    
    if (g_HasMetallic)
    {
        vector vMetallicDesc = g_MetallicTexture.Sample(DefaultSampler, In.vTexcoord);
      //  Out.vPBR.x = 1.f - vMetallicDesc.g;
    }
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal2 = normalize(vMaskNormal * 2.f - 1.f);
            if (vMaskNormal.x > vMaskNormal.z && vMaskNormal.y > vMaskNormal.z)
                vNormal2.z = sqrt(1.f - saturate(dot(vMaskNormal.xy, vMaskNormal.xy)));

            //if (vMask.r == 0.f && vMask.g == 0.f)
            //    vNormal = vNormal1;
            //else
            //{
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
            vNormal = vNormal * vMask.g + vNormal2 * (1.f - vMask.g);
            //}
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);

            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }
        //vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        //vNormal = vNormal1;  

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal; 
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_ALPHA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    //Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
    
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 0.f;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_FOCUS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse * (1.f - vMask.r) + vMaskDiffiuse * vMask.g;
    Out.vDiffuse *= float4(0.7f, 1.f, 0.7f, 1.f);
    
    float3 vNormal;
    
    if(g_HasNormal)
    {
        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
        vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);

        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal.xyz;
        vNormal = vNormal * 0.5f + 0.5f;
        Out.vDepth.z = 1.f;
    }
    
    Out.vNormal = float4(vNormal, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

struct PS_OUT_DEBUG
{
    float4 vDiffuse : SV_TARGET0;
};

PS_OUT_DEBUG PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_DEBUG Out = (PS_OUT_DEBUG) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    
    vector vDefaultDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDefaultDiffuse * (1.f - vMask) + vMaskDiffiuse * vMask;
    return Out;
}

/*======================================================SHADOW_BEGIN======================================================*/

struct VS_OUT_SHADOW
{
    float4 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

struct GS_IN
{
    float4 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
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
        matVP = mul(g_ShadowViewMatrix[Face] , g_ShadowProjMatrix[Face]);

        for (int i = 0; i < 3; i++)
        {
            Out.vPosition = mul(In[i].vPosition, matVP);
            Out.vTexcoord = In[i].vTexcoord;
            Vertices.Append(Out);
        }
        Vertices.RestartStrip();
    }
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

void PS_SHADOW(PS_IN_SHADOW In)
{        
    if (In.vPosition.z >= 1.f)
        discard;
}

/*======================================================SHADOW_END======================================================*/


/*======================================================OUTLINE_BEGIN======================================================*/

struct VS_OUT_OUTLINE
{
    float4 vPosition : SV_POSITION;
};

VS_OUT_OUTLINE VS_OUTLINE(VS_IN In)
{
    VS_OUT_OUTLINE Out = (VS_OUT_OUTLINE) 0;
    
    matrix matVP;
    
    matrix matWV = mul(g_WorldMatrix, g_ViewMatrix);
    
    matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    vector vWorldPos = mul(float4(In.vPosition, 1.f), matWV);
    vector vNormal = normalize(mul(float4(In.vNormal, 0.f), matWV));
   
    vNormal = float4(vNormal.x, vNormal.y, (vNormal.z * 0.12f), 0.f);
   
    vector vOutLinePos = vWorldPos +(vNormal * 0.08f);
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    
    return Out;
}

struct PS_IN_OUTLINE
{
    float4 vPosition : SV_POSITION;
};

struct PS_OUT_OUTLINE
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_OUTLINE PS_OUTLINE(PS_IN_OUTLINE In)
{
    PS_OUT_OUTLINE Out = (PS_OUT_OUTLINE) 0;

    Out.vColor = float4(0.3f, 0.15f, 0.f, 1.f);
    
    return Out;
}

/*======================================================OUTLINE_END======================================================*/

struct PS_OUT_EMISSIVE
{
    float4 vDiffuse : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
};


PS_OUT_EMISSIVE PS_EMISSIVE(PS_IN In)
{
    PS_OUT_EMISSIVE Out = (PS_OUT_EMISSIVE) 0;

    float4 vColor = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = float4(vColor.xyz, 1.f);
    
    float fWeight = Luminance(vColor.xyz);
    
    if(fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz, 1.f);
        
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
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL();
    }

    pass AlphaNotDiscard // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass AlphaNotDiscardNonCull // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass SelectedObject // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_FOCUS();
    }

    pass DebugRender // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }
    pass ShadowPass     //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = compile gs_5_0 GS_SHADOW();
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
    
    pass OutlinePass    //6
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OUTLINE();
    }
    
    pass Emissive
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE();
    }
}