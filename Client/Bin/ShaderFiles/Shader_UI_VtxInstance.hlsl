// UI용
#include "Engine_Shader_State.hlsli"

#define PI          3.14159265359f
#define _BOOL(x)    ((x) != 0.0f)
// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_Texture;
float g_AlphaStrength;

Texture2D g_TextureFX;
float g_FXStrength;

// Gradient Variables
float2 g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f };  // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool g_InverseScreenDiscard = false;                                // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float4 g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                // (좌, 우, 상, 하) (left, right, top, bottom)
float2 g_ScreenSize = { 1920.f, 1080.f };


// Cutout Variables
float g_CutoutAlphaDiscard = 0.3f;


// Nine-Sector Variables
float2 g_ImageSize = { 0.f, 0.f };
float2 g_SectorBorder = { 0.f, 0.f }; // based on local texcoord.     for 9sector
float g_UIScale = 1.f; // UI Scaler




// Variant UI Variables
#define UIFLAG_ERROR                0           // 플래그를 주지 않았을 때의 초기값
#define UIFLAG_COOLDOWN_CIRCLE      1           // 반시계방향으로 나타나는 쿨타임 구현용
#define UIFLAG_COOLDOWN_RECT        2           // 단순 사각형에서 내려오는 쿨타임 구현용
#define UIFLAG_PLAYER_HP            3           // 플레이어 HP용
#define UIFLAG_PLAYER_TRANSMIT      4  
#define UIFLAG_SIMPLEMASK           5
#define UIFLAG_ACTIVEFEEDBACK       6
#define UIFLAG_ENEMY_HP             7

#define UIFLAG_OVFL_PALETTE         8

#define UIFLAG_END                  9

uint g_iVariantFlag = UIFLAG_ERROR;

//#define SCROLL_HP


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
    
    if (((startPos.x < originPos.x && originPos.x < endPos.x) &&
            (startPos.y < originPos.y && originPos.y < endPos.y)))
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
    // 전체 대비 왼쪽/위 로부터 얼마나 오른쪽/아래에 있는지의 비율
    
    float2 resultUV;
    
    // x축 계산
    
    if (originPos.x < border.x)                    // 왼쪽.
        resultUV.x = originPos.x / imageSize.x;
    else if ((modSize.x - border.x) < originPos.x)      // 오른쪽. 
        resultUV.x = (originPos.x - (modSize.x - imageSize.x)) / imageSize.x;
    else
    {
        // [ originPos.x - border.x ] ~ [ originPos.x ] 의 사잇값인 originPosX_OnMod 를
        // [ border.x ] ~ [ modSize.x - border.x ] 사이로 비율을 맞춰야 함 
        
        float originPosX_OnMod = originPos.x - border.x;

        float startX = border.x;
        float endX = modSize.x - border.x;
        
        float resultRatio = originPosX_OnMod / (endX - startX);
        
        resultUV.x = (border.x + (resultRatio * (imageSize.x - border.x * 2))) / imageSize.x;
    }
    
    
    if (originPos.y < border.y)                    // 위쪽
        resultUV.y = originPos.y / imageSize.y;
    else if ((modSize.y - border.y) < originPos.y)      // 아래쪽.
        resultUV.y = (originPos.y - (modSize.y - imageSize.y)) / imageSize.y;
    else
    {
        float originPosX_OnMod = originPos.y - border.y;

        float startY = border.y;
        float endY = modSize.y - border.y;
        
        float resultRatio = originPosX_OnMod / (endY - startY);
        
        resultUV.y = (border.y + (resultRatio * (imageSize.y - border.y * 2))) / imageSize.y;
    }
    
    return resultUV;
}




// ==============================
// * Vertex Shader
// ==============================
struct VS_IN_INSTANCE
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    
    float4 vSInstRight : TEXCOORD1;
    float4 vSInstUp : TEXCOORD2;
    float4 vSInstLook : TEXCOORD3;
    float4 vSInstTrans : TEXCOORD4;
    
    float2 vSInstCoordX : TEXCOORD5;
    float2 vSInstCoordY : TEXCOORD6;
    float2 vClipTexcoordX : TEXCOORD7;
    float2 vClipTexcoordY : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    
    float2 vSInstCoordX : TEXCOORD3;
    float2 vSInstCoordY : TEXCOORD4;
    float2 vClipTexcoordX : TEXCOORD5;
    float2 vClipTexcoordY : TEXCOORD6;
    
    float2 vSInstPos : TEXCOORD7;
    float2 vSInstSca : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
};


VS_OUT VS_INSTANCE(VS_IN_INSTANCE In)
{
    VS_OUT Out = (VS_OUT) 0;
    // [ 인스턴싱용 ] 각 인스턴스별 Vertex 의 Out 정의
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    
    float4x4 matAdditionalTransform = float4x4(
        In.vSInstRight,
        In.vSInstUp,
        In.vSInstLook,
        In.vSInstTrans
    );
    
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), matAdditionalTransform);
    vWorldPos = mul(vWorldPos, matWVP);
    
    Out.vPosition = vWorldPos;
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    Out.vSInstPos = In.vSInstTrans.xy;
    Out.vSInstSca = float2(length(In.vSInstRight.xyz), length(In.vSInstUp.xyz));
    // 이후 픽셀에서 사용
    
    // Pixel에서 사용 위해 바로 Output
    Out.vSInstCoordX = In.vSInstCoordX;
    Out.vSInstCoordY = In.vSInstCoordY;
    Out.vClipTexcoordX = In.vClipTexcoordX;
    Out.vClipTexcoordY = In.vClipTexcoordY;
    Out.mExtra0 = In.mExtra0;
    Out.mExtra1 = In.mExtra1;
    Out.mExtra2 = In.mExtra2;
    Out.mExtra3 = In.mExtra3;
    
    return Out;
}


VS_OUT VS_INSTANCE_VARIANT(VS_IN_INSTANCE In)
{
    VS_OUT Out = (VS_OUT) 0;
    // [ 인스턴싱용 ] 각 인스턴스별 Vertex 의 Out 정의
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    
    float4x4 matAdditionalTransform = float4x4(
        In.vSInstRight,
        In.vSInstUp,
        In.vSInstLook,
        In.vSInstTrans
    );
    float2 vFinalTexcoord = In.vTexcoord;
    float4 vFinalExtra0 = In.mExtra0;
    float4 vFinalExtra1 = In.mExtra1;
    float4 vFinalExtra2 = In.mExtra2;
    float4 vFinalExtra3 = In.mExtra3;
    
    
    // Variant : 계산 전에 계산용 행렬에 값 반영하여 원하는 transform 을 적용
    switch (g_iVariantFlag)
    {
        case UIFLAG_PLAYER_TRANSMIT:
        {
            // ==============================
            // * [4] PlayerEnergy
            // ==============================
            
            // fHeight 만큼 scale 늘리고 fHeight / 2 만큼 y 올려서 보정?
            float fHeight = -In.mExtra2.y;
            
            matAdditionalTransform[1].xyz *= fHeight;                   // Y축
            matAdditionalTransform[3].y += (fHeight - 1) * 0.5f;        // 이따만큼 올림
            
        } break;
        case UIFLAG_ACTIVEFEEDBACK:
        {
            // ==============================
            // * [6] Active Feedback (button touch feedback)
            // ==============================
            float2 vDestScale = In.mExtra0.xy;      // 목표 배율 (최초 1배)
            float fStartAlpha = In.mExtra0.z;
            float fTimeRatio = In.mExtra0.w;
            
            float fDeltaX = fTimeRatio;         // 보간 방법 바꾸고싶다면 이 fDeltaX를 수정하는 식으로?
            
            float2 vCurScale; // 현재 스케일이 0이 아님에 주의. 인스턴스별 크기가 이미 적용된 값이 들어옴.
            vCurScale.x = length(matAdditionalTransform[0].xyz);
            vCurScale.y = length(matAdditionalTransform[1].xyz);
            
            float2 vTargetScale = lerp(vCurScale, vCurScale * vDestScale, fDeltaX);

            matAdditionalTransform[0].xyz *= (vTargetScale.x / vCurScale.x);
            matAdditionalTransform[1].xyz *= (vTargetScale.y / vCurScale.y);

        } break;
        case UIFLAG_ENEMY_HP:
        {
            // ==============================
            // * [7] Dynamic Enemy HP
            // ==============================
            float fYScale = In.mExtra2.y;
            float fElapsedTime = In.mExtra2.z;
            float fCoordSpeed = In.mExtra2.w;
            
            float2 vCurScale; // 현재 스케일이 0이 아님에 주의. 인스턴스별 크기가 이미 적용된 값이 들어옴.
            vCurScale.y = length(matAdditionalTransform[1].xyz);
            matAdditionalTransform[1].xyz *= fYScale;
            
            // texcoord pushing
            #ifdef SCROLL_HP
            vFinalTexcoord.x -= fElapsedTime * fCoordSpeed * 1.f;
            #endif
            vFinalExtra3.xy = In.vTexcoord;
            
        } break;
    }
    
    
    
    
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), matAdditionalTransform);
    vWorldPos = mul(vWorldPos, matWVP);
    
    
    
    
    
    
    
    
    
    Out.vPosition = vWorldPos;
    Out.vTexcoord = vFinalTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    Out.vSInstPos = In.vSInstTrans.xy;
    Out.vSInstSca = float2(length(In.vSInstRight.xyz), length(In.vSInstUp.xyz));
    // 이후 픽셀에서 사용
    
    // Pixel에서 사용 위해 바로 Output
    Out.vSInstCoordX = In.vSInstCoordX;
    Out.vSInstCoordY = In.vSInstCoordY;
    Out.vClipTexcoordX = In.vClipTexcoordX;
    Out.vClipTexcoordY = In.vClipTexcoordY;
    Out.mExtra0 = vFinalExtra0;
    Out.mExtra1 = vFinalExtra1;
    Out.mExtra2 = vFinalExtra2;
    Out.mExtra3 = vFinalExtra3;
    
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
    
    float2 vSInstCoordX : TEXCOORD3; // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vSInstCoordY : TEXCOORD4; // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vClipTexcoordX : TEXCOORD5; // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    float2 vClipTexcoordY : TEXCOORD6; // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    
    float2 vSInstPos : TEXCOORD7;
    float2 vSInstSca : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

// 여러 적들의 HP바를 한번에 그리는 등에 사용하기 위해, 인스턴스마다 제각각,
// 본인 좌표 및 크기를 기준으로 클리핑을 적용한다.
//bool Calc_InstClip(float2 CoordPos, float2 InstPos, float2 InstScale, float2 ClipX /* 0~1 */, float2 ClipY /* 0~1 */) // true 일 시 Clip (discard)
//{
//}

/*

float2 vSInstCoordX     : TEXCOORD3;
float2 vSInstCoordY     : TEXCOORD4;
float2 vClipTexcoordX   : TEXCOORD5;
float2 vClipTexcoordY   : TEXCOORD6;

이 4가지 데이터를 이용하여 실제 적용되도록 만들기 필요.

*/


PS_OUT PS_MAIN(PS_IN In)
{
    // Apply InstCoord for atlas / sprite style
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x), // 이걸로 In.vSInstCoord 범위에 따라.. 이용?
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    
    // Apply ClipTexcoord for clipped ui. like as HP Bar
    // Calc Clip Space
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x), // 예시로 텍스쳐를 0.2 ~ 0.8 범위만 쓰는데, 클립 범위는 0.5 ~ 1.0 이라면
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y)); // 0.2 ~ 0.8 범위 내에서의 0.5 및 1.0을 클립 범위로 삼음. ( result : 0.5 ~ 0.8 )
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    // discard
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    
        
    // In.vSInstCoordX.x 와 In.vSInstCoordX.y 사이의 값을 0~1로 생각하여
    // In.vClipTexcoordX.x, y 가 그 기준으로 밖에 있다면 discard.
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    
    return Out;
}

PS_OUT PS_CUTOUT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
        
    if (Out.vColor.a <= g_CutoutAlphaDiscard)
        discard;
    
    return Out;
}


PS_OUT PS_ALPHAENABLED_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    // 범위 내에 없으면 discard
    if (!Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB, g_InverseScreenDiscard))
        discard;
        
    return Out;
}

PS_OUT PS_GRADIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;

    
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
    
    
    float alphaRatioX = (g_InverseScreenDiscard) ? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard) ? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
    if (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
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
    
    if (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
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
    
   
    float alphaRatio = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio = (g_InverseScreenDiscard) ? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
     
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}

PS_OUT PS_NINESECTOR_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;

    
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
    
    
    float alphaRatioX = (g_InverseScreenDiscard) ? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard) ? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
    if (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
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
    
    if (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
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

    
   
    float alphaRatio = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio = (g_InverseScreenDiscard) ? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
    
    
    
    float2 vSize =
    {
        length(g_WorldMatrix[0].xyz) * g_UIScale,
        length(g_WorldMatrix[1].xyz) * g_UIScale,
    };
    float2 border = g_SectorBorder * g_UIScale;
    float2 localPos = In.vTexcoord * vSize;


    // 9-slice 계산된 UV
    float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize);
    
    
    
    
    float2 finalUV;
    finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
    finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
    
    Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}

PS_OUT PS_VARIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX= float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    
    
    #ifdef SCROLL_HP
    switch (g_iVariantFlag)
    {
        case UIFLAG_ENEMY_HP: // 7
        {
             
            
        } break;
        default :
         {
    #endif
            if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
                fixedUV.y < clipY.x || fixedUV.y > clipY.y)
                discard;
            
            
    #ifdef SCROLL_HP       
        } break;
    }
    #endif
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    

    
    
    switch (g_iVariantFlag)
    {
        case UIFLAG_COOLDOWN_CIRCLE : // 1
        {
            // ==============================
            // * [1] Circle Cooldown
            // ==============================
            // * matrix info [size : 2] (skillbtn_e, skillbtn_r)
            // [CDRATE] [COLORMUL_1] [COLORMUL_2] [IS_USECUSTOMCOLOR]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // [STARTRATIO(DEG)] -
            // ==============================
            float fCooldown = In.mExtra0.x; // 0 ~ 1.
            float fColorMul1 = In.mExtra0.y;
            float fColorMul2 = In.mExtra0.z;
            bool isUseCustomColor = _BOOL(In.mExtra0.w);
            float4 vCustomColor = In.mExtra1.rgba;
            float fStartRatio = In.mExtra2.x;       // 각도(degree) 및 시계방향 기준. 0 기준 12시부터 시작.
            
            // g_fLeftCDRate 가 1 일때는 밝은 색으로
            // g_fLeftCDRate 가 0 일때는 경계가 반시계방향으로 돌며 점차 원래대로의 색으로 바뀌도록
            
            float2 localUV;
            localUV.x = saturate((fixedUV.x - In.vSInstCoordX.x) / (In.vSInstCoordX.y - In.vSInstCoordX.x));
            localUV.y = saturate((fixedUV.y - In.vSInstCoordY.x) / (In.vSInstCoordY.y - In.vSInstCoordY.x));
            
            float2 center = float2(0.5f, 0.5f);
            float2 dir = normalize(localUV - center);   // 중앙에서 목표 UV좌표로의 방향.
            float angle = atan2(dir.y, dir.x);          // +x(3시) 방향 = 0, 반시계방향이 + 기준의 라디안 상대각도를 구함
            angle += ((PI / 2.f) * (1 - fStartRatio / 90.f)); // +90도를 줘서, 기존 3시 방향이었던 각도 기준을 12시로 전환
            if (angle < 0) angle += 2.f * PI;             // 정규화 ([-180 ~ 0], [0 ~ 180] to [180 ~ 360], [0 ~ 180])
    
            float fCooldownAngle = 2.f * PI * fCooldown;  // 진행각도. cooldown 이 0~1 이므로 0도~360도로 치환됨.
            
            
            Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
            
            if (angle <= fCooldownAngle)
            {
                // 이미 지난 부분은 원래의 색으로
                Out.vColor.rgba *= fColorMul1;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
                return Out;
            }
            else
            {
                // 지나지 않은 부분은 좀 더 하얀 색으로
                if (fCooldown != 0.f)
                    Out.vColor.rgba *= fColorMul2;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
                return Out;
            }
            
        } break;
        
        case UIFLAG_COOLDOWN_RECT   : // 2
        {
            // ==============================
            // * [2] Rect Cooldown (for PartyFrame)
            // ==============================
            // * matrix info [size : 3] (frame_rover, frame_augusta, frame_galbrena)
            // [CDRATE] [COLORMUL_1] [COLORMUL_2] -
            // ==============================
            float fCooldown = In.mExtra0.x; // 0 ~ 1.
            float fColorMul1 = In.mExtra0.y;
            float fColorMul2 = In.mExtra0.z;
            
            
            // g_fLeftCDRate 가 1 일때는 어두운 색으로
            // g_fLeftCDRate 가 0 일때는 경계가 아래로 내려가며 밝아지도록
            if (fixedUV.y < fCooldown)
            // 밝게 표시될 부분
                return Out;
            else
            // 어둡게 표시될 부분
                Out.vColor *= fColorMul2;
            
            Out.vColor.a *= (1 - g_AlphaStrength);
            return Out;
        }
        case UIFLAG_PLAYER_HP:           // 3
        {
            // ==============================
            // * [3] PlayerHP
            // ==============================
            // * matrix info [size : 2] (hp_background, hp_normal)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [HPRATE] -
            // ==============================
            vector vColor1 = In.mExtra0.xyzw;
            vector vColor2 = In.mExtra1.xyzw;
            float fHPRatio = saturate(In.mExtra2.x);
            
            // 9sector.. 
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize); // calced
            float2 finalUV;
            finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
            finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
            Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
            // =====
            
            float2 HPclipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, 0.f),
                                    lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, fHPRatio));
            
            if (fixedUV.x < HPclipX.x || fixedUV.x > HPclipX.y)
                discard;
            
            //Out.vColor.rgb = vColor.rgb;
            Out.vColor.rgb = lerp(vColor1, vColor2, fixedUV.x).rgb;
            Out.vColor.a = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength);
            
            return Out;
        } break;
        case UIFLAG_PLAYER_TRANSMIT:      // 4
        {
            // ==============================
            // * [4] PlayerEnergy
            // ==============================
            // * matrix info [size : 41 * 2] (energy * 41, background * 41)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [VISIBLE] [HEIGHT]] -
            // ==============================
            vector vColor1 = In.mExtra0.rgba;
            vector vColor2 = In.mExtra1.rgba;
            bool isVisible = _BOOL(In.mExtra2.x);
            float fHeight = In.mExtra2.y;
            // border는 다 같은 이미지 여러 개 쓸 테니 여기 말고 전역으로 받는게 좋을 듯
            
            // 픽셀 자체의 크기는 픽셀 셰이더에서 제어해야 할 듯
            // 여기서는 height 값에 맞춰 9섹터만 지원하는 식으로
            
            if (!isVisible)
                discard;
            
            // 9sector.. 123
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize); // calced
            float2 finalUV;
            finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
            finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
            Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
            // =====
            
            
            //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
            Out.vColor.rgb  = Out.vColor.rgb * lerp(vColor1, vColor2, fixedUV.y).rgb; // 색상 추가
            Out.vColor.a    = saturate(Out.vColor.a * 1.5f);
            Out.vColor.a    = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength);
            
            return Out;
        } break;
        case UIFLAG_SIMPLEMASK :        // 5. T_MaskCircle.png
        {
            // ==============================
            // * [5] SimpleMask (for SkillIcon BG)
            // ==============================
            // * matrix info [size : ~5]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // [IS_ACTIVE]                                  // 필요 시 조건 추가
            // ==============================
            
            // rgb 의 평균값만큼 색을 준다.
            // rgb 의 평균값이 255에 가까우면 alpha가 1에 가까워진다.
            float4 vColor = In.mExtra0.rgba;
            bool isActive = _BOOL(In.mExtra1.x);
            
            if (!isActive)
                discard;
            
            float fAverageColor = (Out.vColor.x + Out.vColor.y + Out.vColor.z) / 3.f;
            float4 vAppliedColor = vColor * fAverageColor;
            vAppliedColor.a = fAverageColor * vColor.a * Out.vColor.a;
            
            Out.vColor = vAppliedColor;
            return Out;
        } break;
        case UIFLAG_ACTIVEFEEDBACK :    // 6
        {
            // ==============================
            // * [6] Active Feedback (button touch feedback)
            // ==============================
            // * matrix info [size : ~5? controls on hud]
            // [DESTSCALE.x] [DESTSCALE.y] [STARTALPHA] [TIMERATIO]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // ==============================
            float2 vDestScale = In.mExtra0.xy;
            float fStartAlpha = In.mExtra0.z;
            float fTimeRatio = In.mExtra0.w;
            float4 vColor = In.mExtra1.rgba;
            
            float fDeltaX = fTimeRatio;         // 보간 방법 바꾸고싶다면 이 fDeltaX를 수정하는 식으로?
            
            float fAverageColor = (Out.vColor.x + Out.vColor.y + Out.vColor.z) / 3.f;
            float4 vAppliedColor = vColor * fAverageColor;
            vAppliedColor.a = fAverageColor * vColor.a * Out.vColor.a;
            
            Out.vColor.a = vAppliedColor.a * lerp(1.f - fStartAlpha, 0.f, fDeltaX);
            
            return Out;
        } break;
        case UIFLAG_ENEMY_HP :          // 7
        {
            // ==============================
            // * [7] Dynamic Enemy HP
            // ==============================
            // * matrix info [size : dynamic] (per mobs)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [ALPHA] [YSCALE(VS)] [ELAPSEDTIME(VS)] [COORDSPEED(VS)]
            // [ORIGINCOORD(fromVS).xy]
            // ==============================
            vector vColor1 = In.mExtra0.xyzw;
            vector vColor2 = In.mExtra1.xyzw;
            float fAlpha = saturate(In.mExtra2.x);
            float2 vOriginCoord = In.mExtra3.xy;
            
            // 9sector.. 
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            
            // =====
            
            // 원본 coord (vOriginCoord) 기준으로 discard. (밀린 coord 가 아닌 원본 coord가 필요해서 따로 정의)
            float2 fixedUV  = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, vOriginCoord.x),
                                    lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, vOriginCoord.y));
            float2 clipX    = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                                    lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
            float2 clipY    = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                                    lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
            
            float2 fixedUV_Flip     = float2(1.0f - fixedUV.x, fixedUV.y);
            
            if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
                fixedUV.y < clipY.x || fixedUV.y > clipY.y)
                Out.vColor.a = 0.f;         // 추가적으로 그릴 수 있으니 discard 가 아닌, 알파로
            
            // =====

            float fEdgeAlphaWidth = 0.2f;
            
            
            float4 OriginColor = g_Texture.Sample(DefaultSampler, fixedUV);
            float4 FlippedColor = g_Texture.Sample(DefaultSampler, fixedUV_Flip);
            
            
            
            
            //Out.vColor.rgb = vColor.rgb;
            Out.vColor.rgb = lerp(vColor1, vColor2, fixedUV.x).rgb;
            Out.vColor.a = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength) * fAlpha;
            
            //#ifdef SCROLL_HP
            float fEdgeAlpha = saturate(min(vOriginCoord.x /fEdgeAlphaWidth, (1.0f - vOriginCoord.x) / fEdgeAlphaWidth));
            Out.vColor.a *= fEdgeAlpha;
            //#endif
            
            //Out.vColor.rgba = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            return Out;
        } break;
        case UIFLAG_OVFL_PALETTE :      // 8
        {
            #define CHANGE_BYCIRCLE
            
            
            // ==============================
            // * [8] Overflow Palette (UI MiniGame Gimmick)
            // ==============================
            // * matrix info [size : 80] (10 * 8. per blocks)
            // [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
            // [COLORDEST.x] [COLORDEST.y] [COLORDEST.z] [COLORDEST.w]
            // [CHGFRMPOS.x] [CHGFRMPOS.y] [IS_CHANGING] [CHNG_RADIUS] 
            //// [FXIMGSIZE.x] [FXIMGSIZE.y]
            // 텍스쳐를 하나 더 받아와서
            // 현재 winsize 및 inst transform (pos, sca) 기준으로 uv를 적절히 슬라이싱하여 적용하고
            // 색상을 흑백화 및 컬러링해서 out. 하면 될 것 같기도? 아닌가
            // ==============================
            
            float4  vCurrColor          = In.mExtra0.xyzw;
            float4  vDestColor          = In.mExtra1.xyzw;
            float2  vChangeStartPos     = In.mExtra2.xy;        // 퍼지기가 시작될 지점
            bool    IsChanging          = _BOOL(In.mExtra2.z);
            float   fChangedRadius      = In.mExtra2.w;
            //float2  vFXImageSize        = In.mExtra3.xy;
            
            // 마스크 이미지 알파 적용
            float4 vMaskColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            Out.vColor.a = vMaskColor.r;
            
            if (IsChanging) // 변화중
            {
                // 변화중에는 색상 두 개를 사용함.
                // vChangeStartPos 로부터 texcoord 의 스크린 변환 좌표까지의 길이가
                // fChangedRadius 보다 짧은 경우에 vDescColor 적용, 아니면 vCurrColor 적용시키면 될 것으로 보임,
                
                // 전부 같게 변화하는 현상 발생. 각 인스턴스 별 좌표를 기준으로 다시 바로잡을 필요가 있음. 근데 그럼 더 쉽지 않나?
                //float2 vScreenCoord = float2(In.vTexcoord.x * g_ScreenSize.x, -In.vTexcoord.y * g_ScreenSize.y);
                

                // 위에 이거는 현재 인스턴스의 한 점 자체만을 기준삼는 중.
                // 여기에 인스턴스 별 스케일과 In.vPosition 을 적절히 곱하여 인스턴스 별 현재 픽셀 좌표를 알 수 있을 것 같고
                // 이를 통해 물결처럼 퍼져나가는 효과를 기대할 수 있을 듯
                // 사용 가능한 Input?
                // float2 vSInstPos
                // float2 vSInstSca
                
                
                
                // 스크린좌표로 변환하고, 현재 포커싱중인 점이 이를 지나면 색이 바뀌게끔..
                
                #ifdef CHANGE_BYCIRCLE
                float2 vScreenX = { In.vSInstPos.x - In.vSInstSca.x / 2.f, In.vSInstPos.x + In.vSInstSca.x / 2.f };
                float2 vScreenY = { In.vSInstPos.y - In.vSInstSca.y / 2.f, In.vSInstPos.y + In.vSInstSca.y / 2.f };
                
                In.vTexcoord;   // 이게 인스턴스 기준 현재 포커싱중인 좌표
                float2 vFixedScreenPos = { 
                    lerp(vScreenX.x, vScreenX.y, In.vTexcoord.x),
                    lerp(vScreenY.x, vScreenY.y, 1.f - In.vTexcoord.y)
                };
                #endif
                
                
                #ifndef CHANGE_BYCIRCLE
                float2 vSingleInstancePos = In.vSInstPos; ;           // 지금 이거 단순 인스턴스의 한 점을 기준삼는거라, 점 닿자마자 확 바뀌는 듯
                #endif
                
                #ifdef CHANGE_BYCIRCLE
                float2 vSingleInstancePos = vFixedScreenPos; //In.vSInstPos;           // 지금 이거 단순 인스턴스의 한 점을 기준삼는거라, 점 닿자마자 확 바뀌는 듯
                #endif
                
                float fLengthFromStartPos = length(vSingleInstancePos - vChangeStartPos);
                
                float4 vTargetColor;
                
                if (fLengthFromStartPos <= fChangedRadius)      // 가깝다! -> 변해야 됨
                    vTargetColor = vDestColor;
                else                                            // 멀다! -> 아직 변하면 안됨
                    vTargetColor = vCurrColor;
                    
                Out.vColor.rgb = vTargetColor.rgb;
                Out.vColor.a = vTargetColor.a * Out.vColor.a;
            }
            else            // 평시
            {
                Out.vColor.rgb = vCurrColor.rgb;
                Out.vColor.a = vCurrColor.a * Out.vColor.a;
            }   
            
            return Out;
        } break;
        default:
        {
            Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
            return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
        }
    }
    
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
        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass CutOutPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CUTOUT_UI();
    }

    pass AlphaPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ALPHAENABLED_UI();
    }

    pass AlphaGradientPass // AlphaGradient
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRADIENT_UI();
    }

    pass NineSectorPass // AlphaGradient + Nine-Sector
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NINESECTOR_UI();
    }

    pass VariantUIPass  // AlphaPass + a. for cooldown, etc. not designed for animation.
    {                   // 쿨타임 등의 용도로 사용할 특수한 경우용 짬통 pass.. flag로 내부에서 사용할 것 나눔      
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE_VARIANT();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VARIENT_UI();
    }

}
