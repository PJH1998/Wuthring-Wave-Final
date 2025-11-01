// Font.fx

Texture2D g_FontAtlas : register(t0);
SamplerState FontSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

// 화면 크기, 글자 색상 전달받는 상수버퍼
cbuffer FontCB : register(b0)
{
    float2 g_ScreenSize;
    float2 padding;
};
cbuffer FontColorCB : register(b1)
{
    float4 g_FontColor; // RGB + A
};

// 글리프 버텍스 구조
struct VS_IN
{
    float2 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD;
};
struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD;
};

// 정점 셰이더: 화면 좌표를 NDC(-1~1)로 변환
VS_OUT VS_Font(VS_IN In)
{
    VS_OUT Out;
    float2 ndc = (In.vPosition / g_ScreenSize) * float2(2, -2) + float2(-1, 1);
    Out.vPosition = float4(ndc, 0, 1);
    Out.vTexcoord = In.vTexcoord;
    return Out;
}

// 픽셀 셰이더: 폰트 아틀라스 R채널 = Alpha로 사용
float4 PS_Font(VS_OUT In) : SV_Target
{
    float alpha = g_FontAtlas.Sample(FontSampler, In.vTexcoord).r;
    return float4(g_FontColor.rgb, alpha * g_FontColor.a);
}

// 여기서 상태 정의
BlendState BS_FontAlpha
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
};

RasterizerState RS_CullOff
{
    FillMode = Solid;
    CullMode = None;
};

DepthStencilState DSS_Off
{
    DepthEnable = FALSE;
};

//하나의 Pass에 모든 상태 + 셰이더 묶음
technique11 FontTech
{
    pass P0
    {
        SetRasterizerState(RS_CullOff);
        SetBlendState(BS_FontAlpha, float4(0, 0, 0, 0), 0xFFFFFFFF);
        SetDepthStencilState(DSS_Off, 0);

        VertexShader = compile vs_5_0 VS_Font();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Font();
    }
}
