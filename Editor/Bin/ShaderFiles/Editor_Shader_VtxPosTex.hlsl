// ==============================
// * Sampler, State
// ==============================
sampler DefaultSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;
};

RasterizerState RS_Default
{
    FillMode = solid;
    CullMode = back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_None
{
    FillMode = solid;
    CullMode = none;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = LESS_EQUAL;
};

BlendState BS_Default
{
    BlendEnable[0] = false;
};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = Add;
};


// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix      g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D   g_Texture;
float       g_AlphaStrength;


// Gradient Variables
float2      g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f };     // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool        g_InverseScreenDiscard = false;                                 // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float4      g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                   // (좌, 우, 상, 하) (left, right, top, bottom)
float2      g_ScreenSize = { 1920.f, 1080.f };


// Cutout Variables
float       g_CutoutAlphaDiscard = 0.3f;


// Nine-Sector Variables
float2      g_ImageSize = { 0.f, 0.f };
float4      g_SectorBorder = { 0.f, 0.f };                                  // based on local texcoord.     for 9sector
float       g_UIScale = 1.f;                                                // UI Scaler






//float2      g_ScreenLT = { 200.f, 200.f }, g_ScreenRB = { 1720.f, 880.f };     // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
//bool        g_InverseScreenDiscard = false;                                 // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
//float4      g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                   // (좌, 우, 상, 하) (left, right, top, bottom)



// ==============================
// * Function 
// ==============================

bool Check_isInSpace(float2 originPos, float2 vScreenLT, float2 vScreenRB, bool isInverse = false)
{
    // all float2 is based on Screen Space.
    // (LT : 0, 0 / RB : ScreenWidthX, ScreenWidthY)
    
    bool isInSpace = (
        originPos.x >= vScreenLT.x &&
        originPos.x <= vScreenRB.x &&
        originPos.y >= vScreenLT.y &&
        originPos.y <= vScreenRB.y
    );
    
    if (isInverse)
        isInSpace = !isInSpace;
    
    return isInSpace;
}

float Check_SpaceRatio(float2 originPos, float2 startPos, float2 endPos)
{
    // returns Ratio from startPos to endPos. (StartPos : 0, EndPos : 1)
    // Only works based on screen space.
    
    // (LT : 0, 0 / RB : ScreenWidthX, ScreenWidthY)
    
    float result = 0.f;
    
    if (    ((startPos.x < originPos.x  && originPos.x < endPos.x   ) &&
            (startPos.y < originPos.y   && originPos.y < endPos.y   ))  )
    {
        float AtoB = length(originPos - startPos);
        float BtoC = length(endPos - originPos);
        
        result = AtoB / (AtoB + BtoC);
    }
    else
    {
        if (originPos.x < startPos.x)
            return 0.f;
        else if (originPos.x > endPos.x)
            return 1.f;
        else if (originPos.y < startPos.y)
            return 0.f;
        else if (originPos.y > endPos.y)
            return 1.f;
    }
    
    return result;
}

float Check_SpaceRatioP(float originPoint, float startPoint, float endPoint)
{
    return saturate((originPoint - startPoint) / (endPoint - startPoint));
}

float2 Calc_NineSectorUV(float2 originPos, float2 modSize, float2 border, float2 imageSize) // 1. texcoord 상 좌표?
{
    /*
    
    [ originPos ]   : 현재 주시중인 texcoord의, 크기가 반영된 텍스쳐 위의 좌표값
    [ modSize ]     : 크기가 반영된 텍스쳐의 크기
    [ border ]      : 섹터가 구분될 기준 폭 width
    [ imageSize ]   : 크기가 반영되지 않은 원본 텍스쳐의 크기
    
    1. 좌측과 상단의 경우. 즉 바로 border 보다 적게 이동한 위치의 경우.
    단순히 전체 크기 대비 해당 위치의 비교값을 주면 됨.
    
    2. 우측과 하단의 경우. 즉 끝에서부터 border 만큼 뺀 것 사이에 있는 경우.
    비율화하기 전에는 끝으로부터의 거리가 modSize에 상관없이 항상 일정함을 이용한다.
    원본 텍스쳐 크기 대비 오른쪽만큼의 거리 비율을 그대로 이식?
    
    modSize - border*2 만큼의 크기만 변한다고 생각
    
    (modSIze - border * 2) - (ImageSize - border * 2)  ..만이 0 대비 항시 변하는 크기.
    
    
    
    3. 그 외. 즉 중앙점에 있는 경우
    1, 2 에서 구한 최대 / 최소값을 기준으로 사이의 값을 비율화.
    
    
    
    
    */
    
    

    
    float2 resultUV;
    
    // x축 계산
    
    if      (originPos.x < border.x)                    // 왼쪽.
        resultUV.x = originPos.x / modSize.x;
    else if ((modSize.x - border.x) > originPos.x)      // 오른쪽. 
        resultUV.x = 1.0f - (modSize.x - originPos.x) / imageSize.x;
    else
    {
        
    }
    
    
    if      (originPos.y < border.y)                    // 위쪽
        resultUV.y = originPos.y / modSize.y;
    else if ((modSize.y - border.y) > originPos.y)      // 아래쪽.
        resultUV.y = 1.0f - (modSize.y - originPos.y) / imageSize.y;
    else
    {
        
    }
    
    
}




// ==============================
// * Vertex Shader
// ==============================
struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    /* 정점의 로컬위치 * 월드 * 뷰 * 투영 */ 
        
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}



// ==============================
// * Pixel Shader
// ==============================
struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
    
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MAIN_BLEND(PS_IN In) //?
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float2 vTexcoord;
    
    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
    //vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    Out.vColor.a = Out.vColor.a; // * saturate(vDepthDesc.y - In.vProjPos.w);
    
    return Out;
}

PS_OUT PS_CUTOUT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
        
    if (Out.vColor.a <= g_CutoutAlphaDiscard)
        discard;
    
    return Out;
}


PS_OUT PS_ALPHAENABLED_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    // 범위 내에 없으면 discard
    if (!Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB, g_InverseScreenDiscard))
        discard;
        
    return Out;
}

PS_OUT PS_GRADIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    // gradient 목적지 좌표 구함.
    float2 FixedScreenLT = { g_ScreenLT.x - g_BlendToOuterWidth.x, g_ScreenLT.y - g_BlendToOuterWidth.z };
    float2 FixedScreenRB = { g_ScreenRB.x + g_BlendToOuterWidth.y, g_ScreenRB.y + g_BlendToOuterWidth.w };
    
    // discard 클리핑용. 그려질 부분을 모두 감싸는 사각형 좌표 구함. 반전의 경우엔 모두 감싸지는 사각형.
    if (!g_InverseScreenDiscard)
    {
        float2 ClipScreenLT = float2(min(FixedScreenLT.x, g_ScreenLT.x), min(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(max(FixedScreenRB.x, g_ScreenRB.x), max(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    else
    {
        float2 ClipScreenLT = float2(max(FixedScreenLT.x, g_ScreenLT.x), max(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(min(FixedScreenRB.x, g_ScreenRB.x), min(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    
    
    float alphaRatioX = (g_InverseScreenDiscard)? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard)? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
        if      (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
        {
            if (g_ScreenLT.x >= In.vPosition.x) // 왼쪽 밖에 있음
                alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenLT.x, FixedScreenLT.x);
            else if (g_ScreenRB.x <= In.vPosition.x) // 오른쪽 밖에 있음
                alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenRB.x, FixedScreenRB.x);
        }
        else if (g_BlendToOuterWidth.x < 0 || g_BlendToOuterWidth.y < 0) // Inner Gradient
        {
            if (g_ScreenLT.x <= In.vPosition.x && In.vPosition.x <= (g_ScreenLT.x + g_ScreenRB.x) / 2.f) // 왼쪽 안에 있음
                alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenLT.x, g_ScreenLT.x);
            else if ((g_ScreenLT.x + g_ScreenRB.x) / 2.f <= In.vPosition.x && In.vPosition.x <= g_ScreenRB.x)
                alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenRB.x, g_ScreenRB.x);
        }
    
        if      (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
        {
            if (In.vPosition.y <= g_ScreenLT.y)        // 위쪽 밖
                alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenLT.y, FixedScreenLT.y);
            else if (In.vPosition.y >= g_ScreenRB.y)   // 아래쪽 밖
                alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenRB.y, FixedScreenRB.y);
        }
        else if (g_BlendToOuterWidth.z < 0 || g_BlendToOuterWidth.w < 0) // Inner Gradient
        {
            if (In.vPosition.y >= g_ScreenLT.y && In.vPosition.y <= (g_ScreenLT.y + g_ScreenRB.y) / 2.f)
                alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenLT.y, g_ScreenLT.y);
            else if (In.vPosition.y >= (g_ScreenLT.y + g_ScreenRB.y) / 2.f && In.vPosition.y <= g_ScreenRB.y)
                alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenRB.y, g_ScreenRB.y);
        }
    //}

    
    
    
    float alphaRatio        = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio   = (g_InverseScreenDiscard)? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}

PS_OUT PS_NINESECTOR_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    if (g_ImageSize.x == 0 || g_ImageSize.y == 0)
        abort();

    
    float2 vSize = { 
        length(g_WorldMatrix[0].xyz),
        length(g_WorldMatrix[1].xyz),
    };
        
    // 1. ui 크기 대비 border의 비율을 계산
    // 2. 비율에 맞게 texcoord 조절
    // 3. 조절 완료한 좌표 texcoord 로 삽입하여 return
    // 왼쪽 위가 0,0 오른쪽 아래가 1,1임에 주의.

    float2 resultUV;
    
    // 비율
    float borderRatioX = saturate(g_SectorBorder.x / g_ImageSize.x);
    float borderRatioY = saturate(g_SectorBorder.y / g_ImageSize.y);
    
    
    
    
    
    
    
    // 화면 내 로컬 좌표 (0 ~ g_UISize)
    float3 vPosition = g_WorldMatrix[3].xyz;
    
        localPos = In.vTexcoord * g_UISize;

    //// 9-slice 계산된 UV
    //float2 uv = Calc9SliceUV(localPos, g_UISize, g_Border, g_TexSize);
    
    Out.vColor = g_Texture.Sample(DefaultSampler, uv);
    return Out;
}



// ==============================
// * Technique (Pass)
// ==============================
technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass CutOutPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CUTOUT_UI();
    }

    pass AlphaPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ALPHAENABLED_UI();
    }

    pass AlphaGradientPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRADIENT_UI();
    }

    pass NineSectorPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NINESECTOR_UI();
    }
}
