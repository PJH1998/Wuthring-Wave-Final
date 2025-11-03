#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_MaskTexture;
texture2D g_DiffuseTexture;

vector g_vCamPosition;

vector g_vColor;

float g_fXSize;
float g_fYSize;

float g_Sweep;      // 0 -> 1
float g_Soft;       //


struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : POSITION;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    
    return Out;
}
struct GS_IN
{
    float4 vPosition : POSITION;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4] = (GS_OUT[4])0;
    
    vector vRight, vUp, vLook;
    
    vLook = g_vCamPosition - In[0].vPosition;
    vRight = normalize(vector(cross(float3(0.f, 1.f, 0.f), vLook.xyz), 0.f)) * g_fXSize * 0.5f;
    vUp = normalize(vector(cross(vLook.xyz, vRight.xyz), 0.f)) * g_fYSize * 0.5f;
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(In[0].vPosition + vRight + vUp, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    
    Out[1].vPosition = mul(In[0].vPosition - vRight + vUp, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);

    
    Out[2].vPosition = mul(In[0].vPosition - vRight - vUp, matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);

    
    Out[3].vPosition = mul(In[0].vPosition + vRight - vUp, matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[1]);
    Vertices.Append(Out[2]);
    Vertices.RestartStrip();
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[2]);
    Vertices.Append(Out[3]);
    Vertices.RestartStrip();
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
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
    
    if (Out.vDiffuse.r < 0.2f)
        discard;

    Out.vDiffuse *= g_vColor;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    return Out;
}

PS_OUT PS_TEST(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    
    if (Out.vDiffuse.r < 0.25f)
        discard;
    
    float fCenter = In.vTexcoord - (0.5, 0.5);
    
    float fCircle = length(fCenter) / 0.5f;
    
    float fVisible = smoothstep(g_Sweep - g_Soft, g_Sweep, fCircle);
    
    Out.vDiffuse *= fVisible;
    
    if(Out.vDiffuse.a < 0.25f)
        discard;
    
    Out.vDiffuse *= g_vColor;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    return Out;
}

PS_OUT PS_TESTA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    
    if (Out.vDiffuse.r < 0.25f)
        discard;
    
    float fCenter = In.vTexcoord - (0.5, 0.5);
    
    float fCircle = length(fCenter) / 0.5f;
    
    float fVisible = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, fCircle);
    
    Out.vDiffuse *= fVisible;
    
    if (Out.vDiffuse.r < 0.2f)
        discard;
    
    Out.vDiffuse *= g_vColor;
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
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
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
 
    pass OutPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TEST();
    }

    pass InPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TESTA();
    }
    
    pass BlendOut
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TEST();
    }

    pass BlendIn
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_FXBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TESTA();
    }
}