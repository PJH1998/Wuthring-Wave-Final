#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
class CBehavior_Tree;
//class CRigidbody;
//class CCollider;
NS_END

NS_BEGIN(Client)

class CMonsterTest final : public CActor
{
public:
	typedef struct tagMonsterTestDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
	}MONSTERTEST_DESC;



private:
	explicit CMonsterTest(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMonsterTest(const CMonsterTest& Prototype);
	virtual ~CMonsterTest() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	//virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	void			OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool Isactive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override;

private:
	CAnimMachine*			m_pAnimMachineCom = {nullptr};
	CBehavior_Tree*			m_pBehaviorTreeCom = { nullptr };

	//CTransform*				m_pTargetTransformCom = { nullptr };

	_uint					m_iState{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[10]{};
	_float					m_fAttackAcc[10]{};
	_float					m_fDistance{};
	_float					m_fDodgeCoolTime{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};

	_int					m_iHP{};
	_bool					m_isAnimationFinished{};
	_bool					m_isBlocked{};
	_bool					m_isParalysis{};
	_bool					m_isKnockDownTrig{};
	_float					m_fParalysisAcc{};

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(MONSTERTEST_DESC* pDesc);
	void						Ready_PartObjects(MONSTERTEST_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	_bool						isAnimationRunning() { return !m_isAnimationFinished; }
	_bool						isKnockDown();
	_bool						isAttackEnable();
	_bool						DodgeCooldown();
	_bool						Attack(_uint iIndex, _float fInterval);
	_bool						Back();
	_bool						Front();
	_bool						Left();
	_bool						Right();

public:
	static		CMonsterTest*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END