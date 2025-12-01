#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
class CBehavior_Tree;
class CComputeShader;
NS_END

NS_BEGIN(Client)
class CGameSystem;
class CAttackVolume;

class CLeviatan final : public CActor
{
public:
	typedef struct tagLeviatanDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPosition;
		_float3 vInitRotate;
		const _char* pAnimationTag;
		_float		fHP;
		_float fAttackDmg;
		_float fMaxStamina;
		_float3 vDetectRange;
	}LEVIATAN_DESC;
private:
	enum ATK_SOCKET { FOOT_L, FOOT_R, WEAPON_GL, ATKEND };
	//					신권,	   이권,	인권	
	enum WEAPON { MAIN, DIVINITY, DISCORD, VIRTUE, YUNO, END };
	//enum ATK_PATTERN_P1 {ATTACK3, ATTACK5, ATTACK12, ATTACK13, BURST, ATTACK18, ATK1_END };
	enum ATK_PATTERN { ATTACK3, ATTACK5, ATTACK12, ATTACK13, BURST, ATTACK18, ATTACK1, ATTACK20, ATTACK22, ATK_END };
	enum LEVIATAN_SHADER { BANG, HAIR, FACE, UP, DOWN, CLOTH, ALPHA, FX };
	enum PHASE { ONE, TWO, P_END };
private:
	explicit CLeviatan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLeviatan(const CLeviatan& Prototype);
	virtual ~CLeviatan() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

	//virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	void			OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool Isactive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override;

private:
	CAnimMachine* m_pAnimMachineCom[PHASE::P_END] = { nullptr };
	CBehavior_Tree* m_pBehaviorTreeCom[PHASE::P_END] = {nullptr,};
	CGameSystem* m_pGameSystem = { nullptr };
	CComputeShader* m_pFacialComputeShaderCom = { nullptr };

	//const _float4x4* m_pToeMatrix = { nullptr };

	CAttackVolume* m_pAtkVolumes[ATK_SOCKET::ATKEND] = { nullptr, };
	CAttackVolume* m_pParryVolume = { nullptr, };
	vector<_uint>			m_ShaderIndices;

#pragma region STATE_VARIABLE
	_uint					m_iState{};
	_bool					m_isDetecting{};
	_bool					m_isTrigger{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fAttackCoolTime[PHASE::P_END][ATK_PATTERN::ATK_END]{};
	_float					m_fAttackAcc[PHASE::P_END][ATK_PATTERN::ATK_END]{};
	_float					m_fDistance{};
	_float					m_fDistanceNonY{};
	_float					m_fDodgeCoolTime{};
	_float					m_fRightDot{};
	_float					m_fFrontDot{};

	_bool					m_isAnimationFinished{};
	_bool					m_isDeadTrigger{};
	_bool					m_isBlocked{};
	_bool					m_isKnockDownTrig{};
	_float					m_fParalysisAcc{};
	_bool					m_beHit{};
	_bool					m_isAggro{};
	_bool					m_isDist_Interp_Enable{};
	_bool					m_isRender{};
	_uint					m_iPhase{};
#pragma endregion

#pragma region STATUS
	_float					m_fHP{};
	_float					m_fAttackDmg{};
	_float					m_fStamina{};
	_float					m_fMaxStamina{};
	_float					m_fHitStopRatio{};
	_bool					m_fHitAcc{};
#pragma endregion

#pragma region PHYSICS
	_bool					m_isTurnLerp{};
	_float3					m_vBeHit_Normal{};
#pragma endregion

	//그로기 상태인지 bool값, 그로기 최대시간, 현재시간 비율
#pragma region UI_BIND
	_bool					m_isParalysis{};
	_float					m_fParalysisRatio{}; //0.f ~ 1.f
#pragma endregion

private:
	HRESULT						Bind_Resources();
	void						Ready_Component(LEVIATAN_DESC* pDesc);
	void						Ready_PartObjects(LEVIATAN_DESC* pDesc);
	void						Ready_Volumes(LEVIATAN_DESC* pDesc);

	void						Calculate_PosAndDir();
	void						Reset_Condition(_float fTimeDelta);
	void						After_Condition(_float fTimeDelta);
	void						OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void						OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer);
	void						ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	void						TurnFix();
	void						TurnLerp(_bool isActive);
	void						DistanceInterpolate(_bool isActive);
	void						Reset_NotifyInteraction();

#pragma region STATE_FUNC
	_bool						isAnimationRunning() { return !m_isAnimationFinished; }
	_bool						isKnockDown();
	_bool						isAttackEnable();
	_bool						DodgeCooldown();
	_bool						Attack(_uint iIndex, _float fInterval);
	_bool						Attack_Arrange();
	_bool						CheckHit();
	_bool						Back();
	_bool						Front();
	_bool						Left();
	_bool						Right();
#pragma endregion

public:
	static		CLeviatan* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
