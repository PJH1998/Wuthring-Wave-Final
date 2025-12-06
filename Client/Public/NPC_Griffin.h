#pragma once
#include "Actor.h"
NS_BEGIN(Client)
class CNPC_Griffin final : public CActor
{

public:
	typedef struct tagGriffinDesc :CActor::ACTOR_DESC{
		_wstring pAnimMachineTag;
		_float4x4 pTransformMatrix;
	}GRIFFIN_DESC;
private:
	explicit CNPC_Griffin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNPC_Griffin(const CNPC_Griffin& Prototype);
	virtual ~CNPC_Griffin() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;


public:
	//virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
	//virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	//virtual void Object_Func(const _wstring& wStrObjectTag) override; // 임시
	//virtual void Hit_Judge(void* pArg = nullptr) {};// 임시

private:
	CAnimMachine* m_pAnimMachine = { nullptr };

	_uint m_iAnimState = {};
	_bool m_IsRender = {};
private:
	HRESULT		Bind_Resources();
	void		Ready_Component(GRIFFIN_DESC* pDesc);
	void		Ready_InstanceCells(GRIFFIN_DESC* pDesc);

public:
	static CNPC_Griffin* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END