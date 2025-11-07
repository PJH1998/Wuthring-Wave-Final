#ifndef Engine_Struct_h__
#define Engine_Struct_h__

#include "Engine_Typedef.h"

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HWND		hWnd;
		HINSTANCE	hInst;
		WINMODE	eMode;
		_uint			iSizeX, iSizeY;
		_uint			iNumLevel;
		_uint			iNumChannel;
		_uint			iNumCollisionLayer;
	}ENGINE_DESC;

	typedef struct tagLightDesc
	{
		enum TYPE { DIRECTION, POINT, END };

		TYPE eType;
		_float4 vDiffuse;
		_float4 vAmbient;
		_float4 vSpecular;
		_float4 vDirection;
		_float4 vPosition;
		_float fRange;
	}LIGHT_DESC;

	typedef struct tagShadowLightDesc
	{
		XMFLOAT4	vDirection;
		_float		fDistance;
		_float		fFovy;
		_float		fNear;
		_float		fFar;
	}SHADOW_LIGHT_DESC;

	typedef struct tagShadowMapDesc
	{
		_uint iSectorSizeX;		// Texture Size
		_uint iSectorSizeZ;		// Texture Size

		_uint iNumSectorX;		// Sector
		_uint iNumSectorZ;		// Sector

		_float3 vCenterPos;
		_float3 vExtents;
		_float3 vLightDir;
	}SHADOW_MAP_DESC;

	typedef struct tagDecalDesc
	{
		_matrix WorldMatrix;
		_float fLifeTime;
		_float4 vColor;
	}DECAL_DESC;

	typedef struct tagNotify
	{
		_float fTrackPosition;
		function<void()> Func;
		tagNotify(_float _fTrackPosition, function<void()> _Func)
			: fTrackPosition{ _fTrackPosition }, Func{ _Func } {
		};
	}NOTIFY;

	typedef struct tagKeyFrame
	{
		_float3 vScale;
		_float4 vRotation;
		_float3 vTranslation;
		_float fTrackPosition;
	}KEYFRAME;

	typedef struct tagCameraFrame
	{
		// [Target] LookPos = TargetPos + vTranslation
		// [Scene] Position
		XMFLOAT3		vTranslation;
		XMFLOAT4		vRotation;		// Rotation
		float				fDistance = {};		// Distance
		float				fStartFrame = {};
		float				fFovy = {};
		bool				isLerp = { true };
	}CAMERA_FRAME;

	typedef struct tagMapObject
	{
		_float3 vScale;
		_float3 vRotation;
		_float4 vTranslation;
	}MAPOBJECT;

	typedef struct tagCell
	{
		_float3	vPositions[3];
		_uint		iType;
	}CELL;

	typedef struct EffectDesc
	{
		_wstring	strMyTag;
		EFFECT_TYPE eMyType = EFFECT_TYPE::END;
		_bool		IsRootOn = false;
		const _float4x4**  RootMatrix = {};
		const _float4x4** ParentMatrix = {};
		_uint	CurrentLevel;
	}EFFECT_DESC;

	typedef struct ParticleSRV	
	{
		_float4 DefaultPos; 

		_float  fSpeed;
		_float  fDelay;
		_float	_pad0[2];

	}PARTICLE_SRV;

	typedef struct ParticleOptionCB	
	{
		_float3 vPivot;		
		_uint	IsLoop;	
		
		_uint	IsStretch;
		_uint	IsSprite;
		_uint   IsDelay;
		_float	_pad;
	}PARTICLE_DefaultCB;

	typedef struct ParticleSpeedCB
	{
		_float  fTimeDelta;
		_float	fSpreadWeight;
		_float  fDropWeight;
		_float  fRotationWeight;

		_float	fGravity;
		_float	fStretchWeight;
		_float2 fStretchRange;

		_float  fSpriteWeight;
		_float  fSpriteDefault;
		_float  _pad[2];
	}PARTICLE_SPEEDCB;

	typedef struct FXMeshSRV
	{
		_float4 DefaultPos;

		_float fSpeed;						
		_float3 vColor;
	}FXMESH_SRV;

	typedef struct FXMeshCB
	{
		_float3 vPivot;
		_float fTimeDelta;

		_uint IsLoop;
		_float fSpreadWeight;
		_float fDropWeight;
		_float fRotattionWeight;

	}FXMESH_CB;

	typedef struct tagShaderMacro {
		D3D_SHADER_MACRO tagX;
		D3D_SHADER_MACRO tagY;
		D3D_SHADER_MACRO tagZ;
		D3D_SHADER_MACRO tagEnd;
	}SHADER_MACRO;

	typedef struct tagComputeShaderInfo {
		_uint iThreadGroupX;
		_uint iThreadGroupY;
		_uint iThreadGroupZ;
	}COMPUTESHADER_INFO;

	// 애니메이션 정보 구조체 => Depth1
	typedef struct AnimInfo {
		_uint  iStartChannelIndexOffset; // Channel ?쒖옉 (?꾩쟻 ?몃뜳??  
		_uint  iNumChannels; // ???대┰???ы븿??梨꾨꼸(堉???媛쒖닔
		_float fDuration;
		_uint iPadding;  // 4 諛붿씠???⑤뵫??異붽?
	}ANIMINFO;

	//
	typedef struct tagGpuChannelInfo
	{
		_uint iStartKeyframeOffset; // Key Frame 
		_uint iNumKeyframes; // 
		_uint iBoneIndex;  // 
		_uint iPadding;    // 
	}GPU_CHANNELINFO;

	// 채널이 소유하는 KeyFrame(매 TrackPosition마다 뼈의 이동 정보) 구조체 => Depth3
	typedef struct tagGpuKeyFrame {
		_float4 vScale;
		_float4 vRotation;
		_float4 vTranslation;
		_float fTrackPosition;
		_float3 vPadding;  // 16바이트 정렬을 위한 패딩
	}GPU_KEYFRAME;

	// 
	// Constant Buffer? => 16 Byte 단위를 유지해야함 => 16이 안되면 Padding 필수.
	typedef struct tagAnimationCBInfo {
		// 1. Default Animation CB info
		_float fTrackPosition; // 4 
		_uint  iAnimindex;  // 4
		_bool  IsRibAnimUsed = false; // 4 => HLSL 에서 BOOL도 4Byte 인식.
		_uint  iRibbonAnimIndex; // 4
		
		// 2. Blend Layer Control
		_bool  IsBlendEnabled;     // 4 (전체 조준 블렌드 On/Off)
		_float fBlendParamLR;       // 4 (좌/우 파라미터, -1 to 1)
		_float fBlendParamDU;       // 4 (하/상 파라미터, -1 to 1)
		_float fPadding;            // 4 (16바이트 정렬)

		// 3. RL Blend Clip
		_uint iClipIndexL;          // 4 
		_uint iClipIndexMidLR;      // 4 
		_uint iClipIndexR;          // 4 
		_uint iWeightClipLR;       // 4 

		// 4. UD Blend Clip
		_uint iClipIndexD;          // 4 
		_uint iClipIndexMidDU;      // 4 
		_uint iClipIndexU;          // 4 
		_uint iWeightClipDU;       // 4 
	}ANIMATION_CBINFO;

	typedef struct tagGpuBlendInfo {
		_bool  IsBlendEnabled = { false };     // 4  Default 값 으로 전달할지 말지 판단.
		_float fBlendParamLR;       // 4 
		_float fBlendParamDU;       // 4 
		_float fPadding;            // 4 

		// RL Blend Clip
		_string strClipxL;          // 4 
		_string strClipMidLR;      // 4 
		_string strClipxR;          // 4 
		_string strWeightClipLR;       // 4 

		// UD Blend Clip
		_string strClipxD;          // 4 
		_string strClipMidDU;      // 4 
		_string strClipxU;          // 4 
		_string strWeightClipDU;       // 4 
	}GPU_BLEND_INFO;

	typedef struct tagCollisionData {
		class CCollideComponent* pComponent = { nullptr };
		void* pDesc = { nullptr };
	}COLLISION_DATA;


	typedef struct tagSampleDesc
	{
		_float3 vPos = {};
		_float fSpawnTime = {};

	}SAMPLE_DESC;

#pragma region SEQUENCE
	// Sequence Item Frame, Tag => Sequence가 갖고 있음
	typedef struct tagSequenceItem
	{
		_float		fStartFrame = {};
		_float		fEndFrame = {};
		_wstring	strItemTag;
	}SEQUENCE_ITEM;

	// Sequence Item Data => Item Reset시 던질 Data
	typedef struct tagSequenceItemData
	{
		_float		fStartFrame = {};
		_float		fEndFrame = {};
	}SEQUENCE_ITEM_DATA;
#pragma endregion

#pragma region FONT
	typedef struct tFontGlyph
	{
		_uint   iCodepoint;         // 유니코드 코드포인트
		_short  sOffsetX;           // bearingX
		_short  sOffsetY;           // bearingY (상향 +)
		_short  sWidth;             // bitmap.width
		_short  sHeight;            // bitmap.rows
		_short  sAdvance;           // advance.x >> 6
		_float  fU0, fV0, fU1, fV1; // 아틀라스 UV
	}FTCUSTOM_FONT_GLYPH;

	typedef struct tFontInfo
	{
		FT_Face                                     pFace;                      // 폰트 객체

		_int                                        iPixelHeight;               // 설정한 픽셀 사이즈
		ID3D11ShaderResourceView* pAtlasSRV;
		ID3D11Texture2D* pAtlasTex;
		ID3D11SamplerState* pSampler;
		unordered_map<_uint, FTCUSTOM_FONT_GLYPH>   mapGlyphs;                  // 코드포인트→글리프

		//  iAtlasW / iAtlasH   : 아틀라스(폰트 텍스처)의 전체 너비·높이.
		//  iPenX / iPenY       : 현재 글리프를 채워 넣을 "펜" 위치(다음 글리프 배치 시작 좌표).
		//  iRowH               : 현재 줄(row)에서 가장 높은 글리프의 높이(줄바꿈 간격 계산용).
		_int                                        iAtlasW, iAtlasH, iPenX, iPenY, iRowH;
		_bool                                       isHasKerning;
	}FTCUSTOM_FONT;


	typedef struct tFontSingleDesc
	{
		_wstring strFontTag;
		_wstring strText;

		_float2 vScreenPos;
		_float  fScale;

		_float2 vLifeTime;
		_int	iShaderFlag;

		// for shader
		_float4 vColor;				// Font Color

		// for shader : additional info for extra pass 
		// - [Flag 1] outline
		_float4 vOutlineColor;
		_float fFontOutlineWidth;
		// - [Flag 2] grad
		_float4 vFontGradColor;		// Right Dir
		// - [Flag 3] fixed
		_bool isTargetExist = false;
		_float4 vTargetWorldPos;


		//..
	}FONT_SINGLEDESC;

#pragma endregion

	// Default 초기화 용도.
	inline const GPU_BLEND_INFO G_DefaultBlendInfo = {};
}


#endif // Engine_Struct_h__
