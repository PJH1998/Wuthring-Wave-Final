#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CAnimMachine;
class CBehavior_Tree;
NS_END

NS_BEGIN(Client)
class CGameSystem;
class CAttackVolume;
class CCoro_Rock;

class CCorosaurus final : public CActor
{
public:
	typedef struct tagCorrosaurusDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		_float3 vInitRotate;
		_float fAxisY;
		const _char* pAnimationTag;
		_float		fHP;
		_float fAttackDmg;
		_float fMaxStamina;
		_float3 vDetectRange;
	}CORROSAURUS_DESC;

private:
	enum ATK_SOCKET { HEAD0, TAIL, END };
	enum ATK_PATTERN { ATTACK1, ATTACK2, ATTACK8, BURST, ATK_END };
	enum CORO_SHADER {TAIL2, TAIL1, LEG, HEAD, BODY};
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
	virtual		void			Render_Shadow() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

public:
	virtual void	Collider_Active(const _wstring& wStrColliderTag, _bool isActive) override;
	virtual void	Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void	Object_Func(const _wstring& wStrObjectTag) override;
	void			Sound_Active(const _wstring& wStrObjectTag);

private:
	CAnimMachine* m_pAnimMachineCom = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom = { nullptr };
	CGameSystem*		m_pGameSystem = { nullptr };
	CCoro_Rock*			m_pCoroRock = { nullptr };
	CAttackVolume* m_pAtkVolumes[ATK_SOCKET::END] = {nullptr};
	CAttackVolume* m_pParryVolume = {nullptr};
	vector<_uint>			m_ShaderIndices;
	_float4					m_vBaseColor{};

	const _float4x4*		m_pGrabSocket = { nullptr };
	const _float4x4*		m_pCameraSocket = { nullptr };
	_float4x4				m_GrabCombinedMat = {};

#pragma region CONDITION_VARIABLE
	_uint					m_iState{};
	_uint					m_iCurrentAtkIndex{};
	_bool					m_isAggro{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_bool					m_isAttack{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[ATK_PATTERN::ATK_END]{};
	_float					m_fAttackAcc[ATK_PATTERN::ATK_END]{};
	_float					m_fDistance{};
	_float					m_fDistanceNonY{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};
	_float2					m_vDistanceRange{};
	_bool					m_isAnimationFinished{};
	_bool					m_isDeadTrigger{};
#pragma endregion

	_float					m_fHP{};
	_float					m_fStamina{};
	_float					m_fMaxStamina{};
	_float					m_fAttackDmg{};
	_float					m_fHitStopRatio{};
	_bool					m_fHitAcc{};


	_float					m_fParalysisAcc{};
	_bool					m_beHit{};
	_bool					m_isBlocked{};
	_bool					m_isKnockDown{};
	_bool					m_isDist_Interp_Enable{};
	_bool					m_isTurnLerp{};
	_float3					m_vBeHit_Normal{};

//그로기 상태인지 bool값, 그로기 최대시간, 현재시간 비율
#pragma region UI_BIND
	_bool					m_isParalysis{};
	_float					m_fParalysisRatio{}; //0.f ~ 1.f
	_float3					m_vUIPosition{};
#pragma endregion

#pragma region SHADER_VALUE
	_float					m_fBehitMaxTime{};
	_float					m_fBehitAcc{};
	_float					m_fDissolveRate{};
	_bool					m_isDissolve{};
	_float4					m_vMonsterDissolveColor{};
#pragma endregion

#pragma region BEHIT_INTERACT
	_float					m_fBehitDMG{};
	TEXT_COLOR_TYPE			m_eBehitColor{};
	_wstring				m_strBehitSound{};
#pragma endregion

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(CORROSAURUS_DESC* pDesc);
	void						Ready_PartObjects(CORROSAURUS_DESC* pDesc);

	void						Reset_Condition(_float fTimeDelta);
	void						After_Condition(_float fTimeDelta);
	void						Calculate_PosAndDir();

	void						OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						test(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eLayer);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						Reset_NotifyInteraction();
#pragma region BT_CONDITIONS
	_bool						isAnimationRunning() const { return !m_isAnimationFinished; }
	_bool						isKnockDown();
	_bool						isAttackEnable();
	_bool						AttackArrange();
	_bool						Attack(_uint iIndex, _float fInterval);
	_bool						CheckHit();
	_bool						isChase();
	_bool						isPatrol();
	_bool						Back();
	_bool						Front();
	_bool						Left();
	_bool						Right();
#pragma endregion

public:
	static		CCorosaurus*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
