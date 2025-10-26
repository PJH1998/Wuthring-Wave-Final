#pragma once
#include "Camera.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)

class CSpringCamera final : public CCamera
{
public:
	enum class CAMERA_STATE { TARGET, SPRING, LOCKON, CUTSCENE };
private:
	explicit CSpringCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera(const CSpringCamera& Prototype);
	virtual ~CSpringCamera() = default;

public:
	void							Set_Distance(_float fDistance) { m_fDistance += fDistance; }
	void							Set_FixedDistance(_float fFixedDistance) { m_fFixedDistance = fFixedDistance; }

	// SetUp Target Pos, Offset Y
	void							Update_Target(const _fvector& TargetPos, _float fOffsetY);

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
	void							Lock_On()
	{
		if (CAMERA_STATE::LOCKON == m_eCameraState)
		{
			m_eCameraState = CAMERA_STATE::TARGET;
			m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		}
		else if (CAMERA_STATE::TARGET == m_eCameraState)
		{
			m_eCameraState = CAMERA_STATE::LOCKON;
			m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::CAMERA));
		}
	}

public:
	_vector Get_LookVector_NoPitch();
	_vector Get_RightDirection_NoPitch();

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

	void							OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	CAMERA_STATE			m_eCameraState = { CAMERA_STATE::TARGET };
	_float4						m_vLookPosition = {};
	// Detect Collider
	CRigidbody*				m_pRigidbodyCom = { nullptr };

	_float4						m_vTargetPosition = {};		// Target Pos
	_float							m_fOffsetY = {};				// Target Pos Y + OffsetY <= Look

	// Distance Lerp
	_float							m_fFixedDistance = {};
	_float							m_fLerpSpeed = {};
	_float							m_fMinDistance = {};
	_float							m_fMaxDistance = {};

	// Spring
	_float							m_fStiffness = {};		// Spring Force
	_float							m_fDestination = {};	// Spring Destination
	_float							m_fSpringDuration = {};

	// Lock-On
	vector<CTransform*>		m_TargetTransforms;
	CTransform*				m_pTargetTransform = { nullptr };
	_float							m_fLockOnOffsetY = {};

private:
	// Default
	void							Lerp_Distance(_float fTimeDelta);
	void							Mouse_Scroll(_float fTimeDelta);

	// Spring
	void							Spring(_float fTimeDelta);

	// Target
	void							Compute_CamPos();
	void							Check_Ray();

	// LockOn
	void							Lerp_Move(_float fTimeDelta);				// Quat Lerp
	void							Sorting_Target();								// Target Transforms Sort (Distance Less)
	void							Dual_Targeting(_float fTimeDelta);			// Dual Target Compute
	void							Dynamic_Distance();

private:
	void							Ready_Component();

public:
	static		CSpringCamera*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END