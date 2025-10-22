#pragma once
#include "Camera.h"

NS_BEGIN(Engine)
class CCollider;
class CRigidbody;
NS_END

NS_BEGIN(Editor)

class CSpringCamera_Edit final : public CCamera
{
public:
	enum class CAMERA_STATE { TARGET, SPRING, LOCKON, CUTSCENE };
private:
	explicit CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera_Edit(const CSpringCamera_Edit& Prototype);
	virtual ~CSpringCamera_Edit() = default;

public:
	void							Set_Distance(_float fDistance) { m_fDistance += fDistance; }
	void							Set_FixedDistance(_float fFixedDistance) { m_fFixedDistance = fFixedDistance; }

	// SetUp Target Pos, Offset Y
	void							Update_Target(const _fvector& TargetPos, _float fOffsetY)
	{ XMStoreFloat4(&m_vTargetPosition, TargetPos); m_fOffsetY = fOffsetY; };
	// Spring (Distance Adjust) - Lerp
	// 목표 Distance, 도달 시간
	void							Use_Spring(_float fDestination, _float fDuration)
	{
		if (CAMERA_STATE::SPRING == m_eCameraState)
			return;
		m_fDestination = fDestination;
		m_fSpringDuration = fDuration;
		m_eCameraState = CAMERA_STATE::SPRING;
	}
	// Lock-On
	void							Lock_On() { m_isLockOn = !m_isLockOn; }

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

	void							OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	CAMERA_STATE			m_eCameraState = { CAMERA_STATE::TARGET };
	// Detect Collider
	CRigidbody*				m_pRigidbodyCom = { nullptr };

	_float4						m_vTargetPosition = {};		// Target Pos
	_float							m_fOffsetY = {};				// Target Pos Y + OffsetY <= Look

	// Distance Lerp
	_float							m_fFixedDistance = {};
	_float							m_fLerpSpeed = {};

	// Spring
	_float							m_fStiffness = {};		// Spring Force
	_float							m_fDestination = {};	// Spring Destination
	_float							m_fSpringDuration = {};

	// Lock-On
	_bool							m_isLockOn = { false };

private:
	void							Lerp_Distance(_float fTimeDelta);
	void							Mouse_Scroll(_float fTimeDelta);
	void							Spring(_float fTimeDelta);
	void							Compute_CamPos();
	void							Check_Ray();

private:
	void							Ready_Component();

public:
	static		CSpringCamera_Edit*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*				Clone(void* pArg) override;
	virtual		void							Free() override;
};

NS_END