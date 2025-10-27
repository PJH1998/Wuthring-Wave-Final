//// UI용
//#include "Engine_Shader_State.hlsli"
//
//// ==============================
//// * Global Variables
//// ==============================
//matrix      g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
//texture2D   g_Texture;
////texture2D   g_DepthTexture;
//float       g_AlphaStrength;
//
//float2      g_TexcoordLT, g_TexcoordRB;         // for 9sector
//float2      g_ScreenLT  , g_ScreenRB;           // for discard by pos (esc menu, inventory, etc..)
//float       g_CutoutAlphaDiscard = 0.3f;
//
//
//// ==============================
//// * Vertex Shader
//// ==============================
//struct VS_IN
//{
//    float3 vPosition : POSITION;
//    float2 vTexcoord : TEXCOORD0;
//};
//
//struct VS_OUT
//{
//    float4 vPosition : SV_POSITION;
//    float2 vTexcoord : TEXCOORD0;
//    float4 vWorldPos : TEXCOORD1;
//    float4 vProjPos : TEXCOORD2;
//    
//};
//
//VS_OUT VS_MAIN(VS_IN In)
//{
//    VS_OUT Out = (VS_OUT) 0;
//    
//    /* 정점의 로컬위치 * 월드 * 뷰 * 투영 */ 
//        
//    float4x4 matWV, matWVP;
//    
//    matWV = mul(g_WorldMatrix, g_ViewMatrix);
//    matWVP = mul(matWV, g_ProjMatrix);
//    
//    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
//    Out.vTexcoord = In.vTexcoord;
//    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
//    Out.vProjPos = Out.vPosition;
//    
//    return Out;
//}
//
//
//
//// ==============================
//// * Pixel Shader
//// ==============================
//struct PS_IN
//{
//    float4 vPosition    : SV_POSITION;
//    float2 vTexcoord    : TEXCOORD0;
//    float4 vWorldPos    : TEXCOORD1;
//    float4 vProjPos     : TEXCOORD2;
//};
//
//struct PS_OUT
//{
//    float4 vColor       : SV_TARGET0;
//    
//};
//
//PS_OUT PS_MAIN(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//    
//    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
//    
//    return Out;
//}
//
//PS_OUT PS_MAIN_BLEND(PS_IN In) //?
//{
//    PS_OUT Out = (PS_OUT) 0;
//    
//    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
//    
//    float2 vTexcoord;
//    
//    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
//    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
//    //vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
//    
//    //Out.vColor.a = Out.vColor.a * saturate(vDepthDesc.y - In.vProjPos.w);
//    
//    return Out;
//}
//
//PS_OUT PS_CUTOUT_UI(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//    
//    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
//        
//    if (Out.vColor.a <= g_CutoutAlphaDiscard)
//        discard;
//    
//    return Out;
//}
//
//
//PS_OUT PS_ALPHAENABLED_UI(PS_IN In)
//{
//    PS_OUT Out = (PS_OUT) 0;
//    
//    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
//    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
//    
//    return Out;
//}
//
//
//
//// ==============================
//// * Technique (Pass)
//// ==============================
//technique11 DefaultTechnique
//{
//    pass DefaultPass
//    {
//        SetRasterizerState(RS_Default);
//        SetDepthStencilState(DSS_Default, 0);
//        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
//        VertexShader = compile vs_5_0 VS_MAIN();
//        GeometryShader = NULL;
//        PixelShader = compile ps_5_0 PS_MAIN();
//    }
//
//    pass CutOutPass
//    {
//        SetRasterizerState(RS_Default);
//        SetDepthStencilState(DSS_Default, 0);
//        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
//        VertexShader = compile vs_5_0 VS_MAIN();
//        GeometryShader = NULL;
//        PixelShader = compile ps_5_0 PS_CUTOUT_UI();
//    }
//
//    pass AlphaPass
//    {
//        SetRasterizerState(RS_Cull_None);
//        SetDepthStencilState(DSS_Default, 0);
//        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
//
//        VertexShader = compile vs_5_0 VS_MAIN();
//        GeometryShader = NULL;
//        PixelShader = compile ps_5_0 PS_ALPHAENABLED_UI();
//    }
//}
