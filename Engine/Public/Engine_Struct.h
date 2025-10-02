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

	typedef struct tagPrefab
	{
		PREFABTYPE		eType;
		_float3			vScale;
		_float3			vAngle;
		_float3			vTranslation;
		_float				fSpawnTime;
		_char				szTag[MAX_PATH];
	}PREFAB;

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

	typedef struct tagSpawnData
	{
		_uint		iObjectID;
		_int		iNavigationIndex;
		_float3	vPosition;
		_float3	vRotation;
	}SPAWNDATA;

	typedef struct tagCell
	{
		_float3	vPositions[3];
		_uint		iType;
	}CELL;

	typedef struct tagCollision
	{
		class CCollider*		pOtherCollider;
		_bool					isPreCollide;
	}COLLISION;

	typedef struct tagAABB
	{
		_float3		vPosition;
		_float			fRadiusX, fRadiusY, fRadiusZ;
	}AABB;

	typedef struct tagVertexPosition
	{
		_float3		vPosition;

		static const _uint iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXPOS;

	typedef struct tagVertexPositionColor
	{
		_float3		vPosition;
		_float4		vColor;

		static const _uint iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOSCOL;

	typedef struct tagVertexPositionTexture
	{
		_float3		vPosition;
		_float2		vTexcoord;

		static const _uint iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOSTEX;

	typedef struct tagVertexTrail
	{
		_float3		vPosition;
		_float2		vTexcoord;
		_float2		vLifeTime;

		static const _uint iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXTRAIL;

	typedef struct tagVertexNormalTexture
	{
		_float3		vPosition;
		_float3		vNormal;
		_float2		vTexcoord;

		static const _uint iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXNORTEX;

	typedef struct tagVertexMesh
	{
		_float3		vPosition;
		_float3		vNormal;
		_float3		vTangent;
		_float3		vBinormal;
		_float2		vTexcoord;

		static const _uint iNumElements = { 5 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXMESH;

	typedef struct tagVertexAnimMesh
	{
		_float3		vPosition;
		_float3		vNormal;
		_float3		vTangent;
		_float3		vBinormal;
		_uint4			vBlendIndex;
		_float4		vBlendWeight;
		_float2		vTexcoord;

		static const _uint iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXANIMMESH;

	typedef struct tagVertexInstanceMesh
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
	}VTXINSTANCE_MESH;

	typedef struct tagVertexInstanceParticle
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
		_float2		vLifeTime;
	}VTXINSTANCE_PARTICLE;

	typedef struct tagVertexParticle
	{
		static const _uint iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXPARTICLE;

	typedef struct tagVertexPointParticle
	{
		static const _uint iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXPOINTPARTICLE;
}


#endif // Engine_Struct_h__
