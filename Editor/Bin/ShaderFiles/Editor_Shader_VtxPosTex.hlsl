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
matrix      g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D   g_Texture;
//texture2D   g_DepthTexture;
float       g_AlphaStrength;

float2      g_TexcoordLT, g_TexcoordRB;                                     // based on local texcoord.     for 9sector



float2      g_ScreenSize = { 1920.f, 1080.f };

float2      g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f };           // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool        g_InverseScreenDiscard = false;                                 // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float2      g_BlendToOuterWidth = { -200.f, 0.f }; // 이거 써야함

float       g_CutoutAlphaDiscard = 0.3f;



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
    
    
    // g_BlendToOuterWidth.xy 의 xy가 부호가 다르지 않음을 가정. 인벤토리나 esc메뉴에서 쓸 수 있을 것.
    float2 blendEndLT = { g_ScreenLT.x - g_BlendToOuterWidth.x, g_ScreenLT.y - g_BlendToOuterWidth.y };
    float2 blendEndRB = { g_ScreenLT.x + g_BlendToOuterWidth.x, g_ScreenLT.y + g_BlendToOuterWidth.y };

    float alphaRatio = 0.f;
    
    if (!Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB) &&        // 원본 박스 바깥쪽으로 알파처리
        Check_isInSpace(In.vPosition.xy, blendEndLT, blendEndRB))           // g_Screen 가 내부
    {
        // 모서리 (좌상, 좌하, 우상, 우하)
        if  (   (blendEndLT.x <= In.vPosition.x && In.vPosition.x <= g_ScreenLT.x) &&   // 좌상
                (blendEndLT.y <= In.vPosition.y && In.vPosition.y <= g_ScreenLT.y))
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, g_ScreenLT, blendEndLT);
        else if((blendEndLT.x <= In.vPosition.x && In.vPosition.x <= g_ScreenLT.x) &&   // 좌하
                (g_ScreenRB.y <= In.vPosition.y && In.vPosition.y <= blendEndRB.y))
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(g_ScreenLT.x, g_ScreenRB.y), float2(blendEndLT.x, blendEndRB.y));
        else if((g_ScreenRB.x <= In.vPosition.x && In.vPosition.x <= blendEndRB.x) &&   // 우상
                (blendEndLT.y <= In.vPosition.y && In.vPosition.y <= g_ScreenLT.y))
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(g_ScreenRB.x, g_ScreenLT.y), float2(blendEndRB.x, blendEndLT.y));
        else if((g_ScreenRB.x <= In.vPosition.x && In.vPosition.x <= blendEndRB.x) &&   // 우하
                (g_ScreenRB.y <= In.vPosition.y && In.vPosition.y <= blendEndRB.y))
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, g_ScreenRB, blendEndRB);

        
        // 상하좌우
        else if (blendEndLT.y <= In.vPosition.y && In.vPosition.y <= g_ScreenLT.y)      // 상
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(In.vPosition.x, g_ScreenLT.y), float2(In.vPosition.x, blendEndLT.y));
        else if (g_ScreenRB.y <= In.vPosition.y && In.vPosition.y <= blendEndRB.y)      // 하
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(In.vPosition.x, blendEndRB.y), float2(In.vPosition.x, g_ScreenRB.y));
        else if (blendEndLT.x <= In.vPosition.x && In.vPosition.x <= g_ScreenLT.x)      // 좌
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(g_ScreenLT.x, In.vPosition.y), float2(blendEndLT.x, In.vPosition.y));
        else if (g_ScreenRB.x <= In.vPosition.x && In.vPosition.x <= blendEndRB.x)      // 우
            alphaRatio = Check_SpaceRatio(In.vPosition.xy, float2(blendEndRB.x, In.vPosition.y), float2(g_ScreenRB.x, In.vPosition.y));

    }
    //else if (Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB) &&    // 원본 박스 안쪽으로 알파처리
    //    !Check_isInSpace(In.vPosition.xy, blendEndLT, blendEndRB))          // blendEnd 가 내부
    //    Check_SpaceRatio(In.vPosition.xy, );
    //else
    //    abort();    // 부호가 다름. 의도와 다른 사용
    
    Out.vColor.a *= (1.f - alphaRatio);
    
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
}
