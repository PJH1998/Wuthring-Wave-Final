#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
class CBehavior_Tree;
NS_END

NS_BEGIN(Client)

class CHavocWarrior final : public CActor
{
public:
	typedef struct tagHavocWarriorDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
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

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CAnimMachine* m_pAnimMachineCom = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom = { nullptr };

	vector<_float3>			m_PatrolPoints;

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

	_int					m_iHP{};
	_bool					m_isAnimationFinished{};
	_bool					m_isBlocked{};

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(HAVOCWARRIOR_DESC* pDesc);
	void						Ready_PartObjects(HAVOCWARRIOR_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);

	void						OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
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
	static		CHavocWarrior* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END