#pragma once
#include "Camera.h"

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Editor)

class CSpringCamera_Edit final : public CCamera
{
private:
	explicit CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera_Edit(const CSpringCamera_Edit& Prototype);
	virtual ~CSpringCamera_Edit() = default;

public:
	void		SetUp_Target(CTransform* pTargetTransform);

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

private:
	CTransform*				m_pTargetTransform = { nullptr };

public:
	static		CSpringCamera_Edit*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*				Clone(void* pArg) override;
	virtual		void							Free() override;
};

NS_END