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
    float2 g_ScreenSize;        // [8]
    float2 padding;             // [8]
};
cbuffer FontColorCB : register(b1)
{
    float4 g_FontColor;         // [16] RGB + A
};



// Flag Variables..
cbuffer FontFlag : register(b2)
{
    uint g_FontFlag = 0;        // [4]
}

// Requires from Flag..
cbuffer FontOutLine : register(b3)
{
    float4 g_FontOutlineColor;  // [16] RGBA Outline Color
    float2 g_FontTexPerPixel;   // [8] 1.f / Texture Size 
    float g_FontOutlineWidth;   // [4] Outline Width Size
}
cbuffer FontGrad : register(b4)
{
    float4 g_FontGradColor;     // [16] RGBA Gradiant Color (->)
}


#define FL_NONE         0
#define FL_OUTLINE      1 << 0
#define FL_GRAD         1 << 1

#define FL_END          1 << 2


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


// ==============================
// * Vertex Shader
// ==============================
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



// ==============================
// * Pixel Shader
// ==============================
// 픽셀 셰이더: 폰트 아틀라스 {R채널 = Alpha}로 사용!!!!
float4 PS_Font(VS_OUT In) : SV_Target
{
    float alpha = g_FontAtlas.Sample(FontSampler, In.vTexcoord).r;
    
    
        
    
    if (g_FontFlag & FL_GRAD)
    {
        // Gradiant
        float4 GradRColor = g_FontGradColor;
        
        
        
   
    }
    
    
    if (g_FontFlag & FL_OUTLINE)
    {
        // Outline
        float4 OutlineColor = g_FontOutlineColor;
        
        
        float2 uv = In.vTexcoord;
        float alphaCenter = g_FontAtlas.Sample(FontSampler, uv).r;          // 현재 바라보는 픽셀 색상에서 a값 추출
        
        if (alphaCenter > 0.5f)                                             // 불투명에 가깝다 = 경계선에 있지 않다 판단.
            return float4(g_FontColor.rgb, alphaCenter * g_FontColor.a);    // 정해둔 글자 색으로 return.
        
        
        float outline = 0.0f;
        int width = (int) g_FontOutlineWidth;                               // Outline 확인 할 픽셀 범위

        for (int x = -width; x <= width; x++)                                // 현재 바라보는 픽셀 기준, 상하좌우로 범위만큼 탐색
        {
            for (int y = -width; y <= width; y++)
            {
                float2 offsetUV = uv + float2(x * g_FontTexPerPixel.x,      // 받아온 텍스쳐 크기 값을 이용, 픽셀 단위로 변경 후 적용.
                                              y * g_FontTexPerPixel.y);     //  uv는 0~1 사이이므로, 1 / 텍스쳐크기 값 단위로 계산하여
                outline += g_FontAtlas.SampleLevel(FontSampler, offsetUV, 0).r; //  픽셀 단위의 탐색이 가능.
            } // 탐색 시 확인한 알파값을 누적 저장하여, 이를 기반으로 아웃라인 여부 구분.
        }

        // 주변 어딘가에서 픽셀이 발견되면 → 그 영역부터는 아웃라인 처리
        if (outline > 0.0f)
            return g_FontOutlineColor;

        
        // 정리하면,
        // 알파값이 0.5 초과로 불투명하면 원색,
        // 반대로 0.5 이하로 투명한데, 단 0.01이라도 색이 존재는 한다면 아웃라인
        // 알파값이 0이면 투명(discard). 알파값은 r채널에 존재함에 유의
        
        // -> 알파값이 0과 그 이상이 만나는 부분은 딱 갈라져 계산현상 발생함
        
        discard;
   
    }
    
    
    return float4(g_FontColor.rgb, alpha * g_FontColor.a);
}



// ==============================
// * Technique (Pass)
// ==============================
// 하나의 Pass에 모든 상태 + 셰이더 묶음
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
