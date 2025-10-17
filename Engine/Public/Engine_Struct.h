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
		_float3 vEye;
		_float3 vAt;
		_float fFovy;
		_float fNear;
		_float fFar;
	}SHADOW_LIGHT_DESC;

	typedef struct tagNotify
	{
		_float fTrackPosition;
		function<void()> Func;
		tagNotify(_float _fTrackPosition, function<void()> _Func)
			: fTrackPosition{ _fTrackPosition }, Func{ _Func } {};
	}NOTIFY;

	typedef struct tagKeyFrame
	{
		_float3 vScale;
		_float4 vRotation;
		_float3 vTranslation;
		_float fTrackPosition;
	}KEYFRAME;

	typedef struct tagActionFrame
	{
		_float		fDuration;
		_float4	vRotation;
		_float		fDistance;
	}ACTIONFRAME;

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


	typedef struct ParticleSRV
	{
		_float4 DefaultPos; 
		_float  fSpeed;
		_float	_pad0[3];
	}PARTICLE_SRV;

	typedef struct ParticleCB
	{
		_float3 vPivot;		
		_float  fTimeDelta;

		_uint	IsLoop;		// 0이면 false, 1이면 true
		_float	fSpreadWeight;
		_float  fDropWeight;
		_float  fRotationWeight;
	}PARTICLE_CB;


}


#endif // Engine_Struct_h__
