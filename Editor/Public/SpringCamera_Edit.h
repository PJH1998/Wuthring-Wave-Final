#pragma once
#include "Camera.h"

NS_BEGIN(Engine)
class CTransform;
class CCollider;
NS_END

NS_BEGIN(Editor)

class CSpringCamera_Edit final : public CCamera
{
private:
	explicit CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera_Edit(const CSpringCamera_Edit& Prototype);
	virtual ~CSpringCamera_Edit() = default;

public:
	void		Update_Target(const _fvector& TargetPos, _float fOffsetY) { XMStoreFloat4(&m_vTargetPosition, TargetPos); m_fOffsetY = fOffsetY; };

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

private:
	CCollider*					m_pColliderCom = { nullptr };
	_float4						m_vTargetPosition = {};
	_float							m_fOffsetY = {};				// Target Pos Y + OffsetY <= Look

	// Spring
	_bool							m_isSpring = { false };
	_float							m_fStiffness = {};		// Spring Force
	_float							m_fDamp = {};			// °¨¼è °è¼ö

private:
	//void							Spring();
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