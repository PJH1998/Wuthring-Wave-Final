#pragma once
#include "Engine_Define.h"
#include "Client_Enum.h"

namespace Client
{
	// ============================================================
	// [1, 2] Circle Cooldown
	// ============================================================

	typedef union tagUIData_CooldownCircle
	{
		struct
		{
			_float fCDRate;				// _11
			_float fColorMul1;			// _12
			_float fColorMul2;			// _13
			_float bUseCustomColor;		// _14

			_float4 vColor;				// _21 ~ _24

			_float fStartRatio;			// _31
			_float bUseNoise;			// _32
			_float fElapsedTime;		// _33
			_float fUVScrollSpeed;		// _34

			_float4 vMaskColor;
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_COOLDOWN_CIRCLE;


	// ============================================================
	// [3] Rect Cooldown
	// ============================================================

	typedef union tagUIData_CooldownRect
	{
		struct
		{
			_float fCDRate;			// _11
			_float fColorMul1;		// _12
			_float fColorMul2;		// _13
			_float padding0;		// _14

			// unused
			_float4 padding1;		// _21 ~ _24
			_float4 padding2;		// _31 ~ _34
			_float4 padding3;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_COOLDOWN_RECT;


	// ============================================================
	// [4] Player HP
	// ============================================================

	typedef union tagUIData_PlayerHP
	{
		struct
		{
			_float4 vColorGrad1;	// _11 ~ _14
			_float4 vColorGrad2;	// _21 ~ _24

			_float fHPRate;			// _31
			_float bUseNoise;		// _32
			_float fElapsedTime;	// _33
			_float fUVScrollSpeed;	// _34

			_float4 vMaskColor;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_PLAYER_HP;


	// ============================================================
	// [5] Player Energy
	// ============================================================

	typedef union tagUIData_PlayerTransmit
	{
		struct
		{
			_float4 vColorGrad1;	// _11 ~ _14
			_float4 vColorGrad2;	// _21 ~ _24

			_float bVisible;		// _31
			_float fHeight;			// _32

			// unused
			_float padding0;		// _33
			_float padding1;		// _34

			_float4 padding2;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_PLAYER_TRANSMIT;


	// ============================================================
	// [6] Simple Mask
	// ============================================================

	typedef union tagUIData_SimpleMask
	{
		struct
		{
			_float4 vColor;			// _11 ~ _14

			_float bActive;			// _21
			
			// unused
			_float padding0;		// _22
			_float padding1;		// _23
			_float padding2;		// _24

			_float4 padding3;		// _31 ~ _34
			_float4 padding4;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_SIMPLEMASK;


	// ============================================================
	// [7] Active Feedback
	// ============================================================

	typedef union tagUIData_ActiveFeedback
	{
		struct
		{
			_float2 vDestScaleX;	// _11 ~ _12
			_float fStartAlpha;		// _13
			_float fTimeRatio;		// _14

			_float4 vColor;			// _21 ~ _24

			// unused
			_float4 padding0;		// _31 ~ _34
			_float4 padding1;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_ACTIVEFEEDBACK;


	// ============================================================
	// [8] Enemy HP
	// ============================================================

	typedef union tagUIData_EnemyHP
	{
		struct
		{
			_float4 vColorGrad1;	// _11 ~ _14
			_float4 vColorGrad2;	// _21 ~ _24

			_float fAlpha;			// _31
			_float fYScale;			// _32
			_float fElapsedTime;	// _33
			_float fCoordSpeed;		// _34

			_float fOriginCoordX;	// _41
			_float fOriginCoordY;	// _42

			// unused
			_float padding0;		// _43
			_float padding1;		// _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_ENEMY_HP;


	// ============================================================
	// [9] Overflow Palette
	// ============================================================

	typedef union tagUIData_OverflowPalette
	{
		struct
		{
			_float4 vColorCurr;		// _11 ~ _14
			_float4 vColorDest;		// _21 ~ _24

			_float2 vChangePos;		// _31 ~ _32
			_float bChanging;		// _33
			_float fRadius;			// _34

			_float2 vExtraImgSize;	// _41 ~ _42

			// unused
			_float padding0;		// _43
			_float padding1;		// _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_OVFL_PALETTE;


	// ============================================================
	// [10] Simple Colorize
	// ============================================================

	typedef union tagUIData_SimpleColorize
	{
		struct
		{
			_float4 vColor;			// _11 ~ _14

			// unused
			_float4 padding0;		// _21 ~ _24
			_float4 padding1;		// _31 ~ _34
			_float4 padding2;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_SIMPLE_COLORIZE;


	// ============================================================
	// [11] Wave Circle
	// ============================================================

	typedef union tagUIData_WaveCircle
	{
		struct
		{
			_float4 vColor;			// _11 ~ _14

			_float bDistort;		// _21
			_float fElapsedTime;	// _22
			_float fDistortStrength;// _23
			_float fRotateSpeed;	// _24

			_float bDisableNormalize; // _31
			_float fAlphaMul;		// _32

			// unused
			_float padding0;		// _33
			_float padding1;		// _34

			_float4 padding2;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_WAVECIRCLE;


	// ============================================================
	// [TEXT] TEXT Instance
	// ============================================================

	typedef union tagUIData_Text
	{
		struct
		{
			_float fAlpha;			// _11

			// unused
			_float padding0;		// _12
			_float padding1;		// _13
			_float padding2;		// _14
			_float4 padding3;		// _21 ~ _24
			_float4 padding4;		// _31 ~ _34
			_float4 padding5;		// _41 ~ _44
		};

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		_float4x4 raw;

	} UIDATA_TEXT;


	// ============================================================
	// Master Extra Data Union
	// ============================================================

	typedef union tagUIExtraData
	{
		_float4x4 raw;

		struct
		{
			_float4 Extra0;
			_float4 Extra1;
			_float4 Extra2;
			_float4 Extra3;
		};

		UIDATA_COOLDOWN_CIRCLE		CooldownCircle;
		UIDATA_COOLDOWN_RECT		CooldownRect;
		UIDATA_PLAYER_HP			PlayerHP;
		UIDATA_PLAYER_TRANSMIT		PlayerTransmit;
		UIDATA_SIMPLEMASK			SimpleMask;
		UIDATA_ACTIVEFEEDBACK		ActiveFeedback;
		UIDATA_ENEMY_HP				EnemyHP;
		UIDATA_OVFL_PALETTE			OverflowPalette;
		UIDATA_SIMPLE_COLORIZE		SimpleColorize;
		UIDATA_WAVECIRCLE			WaveCircle;
		UIDATA_TEXT					Text;

	} UI_EXTRA_DATA;


	// ============================================================
	// Safety Check
	// ============================================================

	static_assert(sizeof(UI_EXTRA_DATA) == 64, "UI_EXTRA_DATA must be 64 bytes.");
}