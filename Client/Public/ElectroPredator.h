#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
class CBehavior_Tree;
NS_END

NS_BEGIN(Client)

class CElectroPredator final : public CActor
{
public:
	typedef struct tagHavocWarriorDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
	}ELECTROPREDATOR_DESC;

private:
	explicit CElectroPredator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CElectroPredator(const CElectroPredator& Prototype);
	virtual ~CElectroPredator() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool Isactive);
	virtual void Effect_Active(const _wstring& wStrEffectTag);

private:
	CAnimMachine* m_pAnimMachineCom = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom = { nullptr };

	queue<_float3>			m_PatrolPoints;

	_uint					m_iState{};
	_bool					m_isAggro{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[3]{};
	_float					m_fAttackAcc[3]{};
	_float					m_fDistance{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};

	_bool					m_isAnimationFinished{};
	_bool					m_isBlocked{};
	_bool					m_beHit{};
	_float					m_fIdleDuration{};
	_float					m_fIdleAcc{};

	_int					m_iHP{};
	_float					m_fAttackDmg{};

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(ELECTROPREDATOR_DESC* pDesc);
	void						Ready_PartObjects(ELECTROPREDATOR_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);
	void						Calculate_PosAndDir();

	void						OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						Patrol();
	_bool						isAnimationRunning() { return !m_isAnimationFinished; }
	_bool						isKnockDown();
	_bool						isAttackEnable();
	_bool						Attack(_uint iIndex, _float fInterval);
	_bool						isChase();
	_bool						isPatrol();
	_bool						Back();
	_bool						Front();
	_bool						Left();
	_bool						Right();

public:
	static		CElectroPredator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END