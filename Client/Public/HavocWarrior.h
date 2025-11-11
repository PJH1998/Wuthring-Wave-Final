#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
class CBehavior_Tree;
NS_END

NS_BEGIN(Client)
class CAttackVolume;

class CHavocWarrior final : public CActor
{
public:
	typedef struct tagHavocWarriorDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
		_float fHp;
		_float fAttackDmg;
		_float fImpluseRate;
	}HAVOCWARRIOR_DESC;

private:
	explicit CHavocWarrior(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CHavocWarrior(const CHavocWarrior& Prototype);
	virtual ~CHavocWarrior() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool isActive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override;

private:
	CAnimMachine* m_pAnimMachineCom = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom = { nullptr };
	CAttackVolume* m_pAtkVolume = { nullptr };

	queue<_float3>			m_PatrolPoints;

#pragma region STATE_VARIABLE
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
	_float2					m_vDistanceRange{};
	_float					m_fIdleDuration{};
	_float					m_fIdleAcc{};
	_float					m_fStrikeAcc{};
	_bool					m_AirTrig{};
	_float					m_fAirAcc{};
	_bool					m_beHit{};
	_bool					m_isPushed{};
	_bool					m_isAnimationFinished{};
#pragma endregion

#pragma region STATUS
	_float					m_fHP{};
	_float					m_fAttackDmg{};
	_float					m_fImpluseRate{};
#pragma endregion

#pragma region PHYSICS
	_float3					m_vBeHit_Normal{};
	_float					m_fTimeDelta{};
	_bool					m_isTurnLerp{};
	CALLBACK_CLIENT			m_tCallDesc{};
#pragma endregion
private:
	HRESULT						Bind_Resources();
	void						Ready_Component(HAVOCWARRIOR_DESC* pDesc);
	void						Ready_PartObjects(HAVOCWARRIOR_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);
	void						After_Condition(_float fTimeDelta);
	void						Calculate_PosAndDir();
	void						TurnFix();
	void						TurnLerp(_bool isActive);
#pragma region COLLISION_EVENT
	void						OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						Patrol();
#pragma endregion

#pragma region BEHAVIOR_TREE_CONDITION
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
#pragma endregion

public:
	static		CHavocWarrior* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END