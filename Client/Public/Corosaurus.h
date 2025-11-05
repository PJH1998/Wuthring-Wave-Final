#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CAnimMachine;
class CBehavior_Tree;
NS_END

NS_BEGIN(Client)
class CAttackVolume;

class CCorosaurus final : public CActor
{
public:
	typedef struct tagCorrosaurusDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
	}CORROSAURUS_DESC;

private:
	enum ATK_SOCKET { HEAD, TAIL, END };
private:
	explicit CCorosaurus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCorosaurus(const CCorosaurus& Prototype);
	virtual ~CCorosaurus() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool isActive);
	virtual void Effect_Active(const _wstring& wStrEffectTag);

private:
	CAnimMachine* m_pAnimMachineCom = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom = { nullptr };
	CAttackVolume* m_pAtkVolume[ATK_SOCKET::END] = {nullptr};

	_uint					m_iState{};
	_bool					m_isAggro{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[9]{};
	_float					m_fAttackAcc[9]{};
	_float					m_fDistance{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};
	_float2					m_vDistanceRange{};

	_float					m_fHP{};
	_bool					m_isAnimationFinished{};
	_bool					m_isBlocked{};

	CALLBACK_CLIENT			m_tCallDesc{};

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(CORROSAURUS_DESC* pDesc);
	void						Ready_PartObjects(CORROSAURUS_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);
	void						After_Condition(_float fTimeDelta);
	void						Calculate_PosAndDir();

	void						OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);

public:
	static		CCorosaurus*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
