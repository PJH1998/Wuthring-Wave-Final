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

class CMonsterTest final : public CActor
{
public:
	typedef struct tagMonsterTestDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		const _char* pAnimationTag;
		_float		fHP;
		_float fAttackDmg;
		_float fMaxStamina;
		_float3 vDetectRange;
	}MONSTERTEST_DESC;

private:
	enum ATK_SOCKET { WEAPON_L, WEAPON_R, WHIP_L, WHIP_R, END };
	enum ATK_PATTERN { ATTACK1, ATTACK2, ATTACK3, ATTACK4, ATTACK5, ATTACK6, ATTACK7, ATTACK9, ATTACK10, ATTACK11, ATK_END };
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

	CAttackVolume*			m_pAtkVolumes[ATK_SOCKET::END] = {nullptr,};
	CAttackVolume*			m_pParryVolume = {nullptr,};

#pragma region STATE_VARIABLE
	_uint					m_iState{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[ATK_PATTERN::ATK_END]{};
	_float					m_fAttackAcc[ATK_PATTERN::ATK_END]{};
	_float					m_fDistance{};
	_float					m_fDodgeCoolTime{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};

	_bool					m_isAnimationFinished{};
	_bool					m_isBlocked{};
	_bool					m_isParalysis{};
	_bool					m_isKnockDownTrig{};
	_float					m_fParalysisAcc{};
	_bool					m_beHit{};
	_bool					m_isAggro{};
	_bool					m_isDist_Interp_Enable{};
#pragma endregion
	
#pragma region STATUS
	_float					m_fHP{};
	_float					m_fAttackDmg{};
	_float					m_fStamina{};
	_float					m_fMaxStamina{};
#pragma endregion

#pragma region PHYSICS
	_bool					m_isTurnLerp{};
#pragma endregion

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(MONSTERTEST_DESC* pDesc);
	void						Ready_PartObjects(MONSTERTEST_DESC* pDesc);

	void						Calculate_PosAndDir();
	void						Reset_Condition(_float fTimeDelta);
	void						After_Condition(_float fTimeDelta);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	void						TurnFix();
	void						TurnLerp(_bool isActive);
	void						DistanceInterpolate(_bool isActive);

#pragma region STATE_FUNC
	_bool						isAnimationRunning() { return !m_isAnimationFinished; }
	_bool						isKnockDown();
	_bool						isAttackEnable();
	_bool						DodgeCooldown();
	_bool						Attack(_uint iIndex, _float fInterval);
	void						Attack_Arrange();
	_bool						Back();
	_bool						Front();
	_bool						Left();
	_bool						Right();
#pragma endregion

public:
	static		CMonsterTest*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END