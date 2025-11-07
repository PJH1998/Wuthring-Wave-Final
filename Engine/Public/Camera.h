#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC {
		_float4	vEye{}, vAt{};
		_float		fFovy{};
		_float		fNear{}, fFar{};
		_float		fMouseSensor{};
	}CAMERA_DESC;
protected:
	explicit CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	_float							Get_Distance() { return m_fDistance; }
	_float							Get_Near() { return m_fNear; }
	_float							Get_Far() { return m_fFar; }
	void							OnShake(const _float3& vDir);


public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void				Priority_Update(_float fTimeDelta);
	virtual		void				Update(_float fTimeDelta);
	virtual		void				Late_Update(_float fTimeDelta);
	virtual		void				Render();

	void							Update_Matrix();

protected:
	_float							m_fFovy = {};
	_float							m_fAspect = {};
	_float							m_fNear{}, m_fFar{};
	_float							m_fMouseSensor{};

	_float							m_fDistance = {};

	_float4x4						m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::END)] = {};

	// Shaking
	_bool							m_isShake = { false };
	_float							m_fShakeDuration = {};		// Shake 지속시간
	_float							m_fShakeTimeAcc = {};		// Shake 진행시간
	_float							m_fShakeStiffness = {};		// Shake 복원력
	_float							m_fShakeDamp = {};			// Shake 감쇠
	_float3						m_vShakeVelocity = {};		// Shake 힘
	_float3						m_vShakeOffset = {};			// Shake로 인한 카메라 이동량

protected:
	void							Lerp_Distance(_float fTimeDelta);
	void							Key_Move(_float fTimeDelta);
	void							Mouse_Move();
	void							Mouse_Move_Up();

	void							Shaking(_float fTimeDelta);

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END