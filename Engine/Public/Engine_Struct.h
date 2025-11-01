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
		const _float4x4*  RootMatrix = {};
	}EFFECT_DESC;

	typedef struct ParticleSRV	
	{
		_float4 DefaultPos; 

		_float  fSpeed;
		_float	_pad0[3];

	}PARTICLE_SRV;

	typedef struct ParticleOptionCB	
	{
		_float3 vPivot;		
		_uint	IsLoop;	
		
		_uint	IsStretch;
		_uint	IsSprite;
		_float	_pad[2];
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

	// 梨꾨꼸 ?뺣낫 援ъ“泥?=> Depth2
	typedef struct tagGpuChannelInfo
	{
		_uint iStartKeyframeOffset; // Key Frame ?쒖옉 (?꾩쟻 ?몃뜳??
		_uint iNumKeyframes; // ?꾩옱 Channel?먯꽌??KeyFrame 媛쒖닔.
		_uint iBoneIndex;  // 異붽?: ??梨꾨꼸???대뼡 堉덈? 而⑦듃濡ㅽ븯?붿?
		_uint iPadding;    // 16諛붿씠???뺣젹???꾪븳 ?⑤뵫
	}GPU_CHANNELINFO;

	// 채널이 소유하는 KeyFrame(매 TrackPosition마다 뼈의 이동 정보) 구조체 => Depth3
	typedef struct tagGpuKeyFrame {
		_float4 vScale;
		_float4 vRotation;
		_float4 vTranslation;
		_float fTrackPosition;
		_float3 vPadding;  // 16바이트 정렬을 위한 패딩
	}GPU_KEYFRAME;

	// (留??꾨젅???낅뜲?댄듃)
	// Constant Buffer??珥??ш린媛 諛섎뱶??16??諛곗닔?ъ빞??
	typedef struct tagAnimationCBInfo {
		_float fTrackPosition;
		_uint  iAnimindex;
		_bool  IsRibAnimUsed = false;
		_uint  iRibbonAnimIndex;
	}ANIMATION_CBINFO;

	typedef struct tagCollisionData {
		class CCollideComponent* pComponent = { nullptr };
		void* pDesc = { nullptr };
	}COLLISION_DATA;

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
}


#endif // Engine_Struct_h__
