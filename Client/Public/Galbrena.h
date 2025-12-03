#pragma once
#include "Player_Define.h"
#include "GalbrenaState_Enum.h"

NS_BEGIN(Client)
class CGalbrena final : public CCharacter
{
public:
	enum PENDING_CONDITION : _uint
	{
		HIT = 0,
		DODGE,
		PARRY,
		QTE,
		CONDITION_END
	};

	enum VOLUME
	{
		VOLUME_ARROUND = 0,
		VOLUME_ARROUND_SLASH,
		VOLUME_AIR_LOOP, // 공중에서 공격.
		VOLUME_TARGET,
		VOLUME_TARGET_BURST,
		VOLUME_KNOCKBACK,
		VOLUME_DEFAULT_E,
		VOLUME_SKILL,
		VOLUME_END
	};

	enum MESHTYPE
	{
		MESH_BANGS = 0,
		MESH_HAIR,
		MESH_FACE,
		MESH_UP,
		MESH_DOWN,
		MESH_CLOTH,
		MESH_EYE,
		MESH_EYE_OL,
		MESH_END
	};

#pragma region STATE
private:
	struct StateTransitionContext
	{
		// Ground
		EGalbrenaDashType m_eDashType = EGalbrenaDashType::END;
		EGalbrenaSpecialDashType m_eSpecialDashType = EGalbrenaSpecialDashType::END;
		EGalbrenaIdleType  m_eIdleType = EGalbrenaIdleType::END;
		EGalbrenaRunType m_eRunType = EGalbrenaRunType::END;
		EGalbrenaSprintType m_eSprintType = EGalbrenaSprintType::END;
		EGalbrenaLandType m_eLandType = EGalbrenaLandType::END;
		EGalbrenaDodgeType m_eDodgeType = EGalbrenaDodgeType::END;

		EGalbrenaAttackType m_eAttackType = EGalbrenaAttackType::END;
		EGalbrenaHeavyAttackType m_eHeavyAttackType = EGalbrenaHeavyAttackType::END;
		EGalbrenaSkillType m_eSkillType = EGalbrenaSkillType::END;
		EGalbrenaUniqueType m_eUniqueType = EGalbrenaUniqueType::END;
		EGalbrenaBurstType m_eBurstType = EGalbrenaBurstType::END;
		EGalbrenaSpecialType m_eSpecialType = EGalbrenaSpecialType::END;
		EGalbrenaQTEType m_eQTEType = EGalbrenaQTEType::END;

		// Air
		EGalbrenaJumpType m_eJumpType = EGalbrenaJumpType::END;
		EGalbrenaFallType m_eFallType = EGalbrenaFallType::END;
		EGalbrenaAirAttackType m_eAirAttackType = EGalbrenaAirAttackType::END;
		EGalbrenaAirFlyType m_eAirFlyType = EGalbrenaAirFlyType::END;

		// Climb
		EGalbrenaClimbIdleType m_eClimbIdleType = EGalbrenaClimbIdleType::END;
		EGalbrenaClimbMoveType m_eClimbMoveType = EGalbrenaClimbMoveType::END;
		EGalbrenaClimbExitType m_eClimbExitType = EGalbrenaClimbExitType::END;
		_bool m_IsClimbSecondStep = { false };

		// Hit
		EGalbrenaHitType m_eHitType = EGalbrenaHitType::END;

		// Interaction
		EGalbrenaRopeHookType m_eRopeHookType = EGalbrenaRopeHookType::END;

		// Capture
		EGalbrenaCaptureType m_eCaptureType = EGalbrenaCaptureType::END;

		// Prev Info
		_string m_strPrevInfo = {};
		void Clear()
		{
			// Land
			m_eIdleType = EGalbrenaIdleType::END;
			m_eRunType = EGalbrenaRunType::END;
			m_eSprintType = EGalbrenaSprintType::END;
			m_eDashType = EGalbrenaDashType::END;
			m_eSpecialDashType = EGalbrenaSpecialDashType::END;
			m_eLandType = EGalbrenaLandType::END;
			m_eDodgeType = EGalbrenaDodgeType::END;

			// Attack
			m_eAttackType = EGalbrenaAttackType::END;
			m_eHeavyAttackType = EGalbrenaHeavyAttackType::END;
			m_eSkillType = EGalbrenaSkillType::END;
			m_eUniqueType = EGalbrenaUniqueType::END;
			m_eBurstType = EGalbrenaBurstType::END;
			m_eSpecialType = EGalbrenaSpecialType::END;

			// Air
			m_eJumpType = EGalbrenaJumpType::END;
			m_eFallType = EGalbrenaFallType::END;
			m_eAirAttackType = EGalbrenaAirAttackType::END;
			m_eAirFlyType = EGalbrenaAirFlyType::END;

			// Climb
			m_eClimbIdleType = EGalbrenaClimbIdleType::END;
			m_eClimbMoveType = EGalbrenaClimbMoveType::END;
			m_eClimbExitType = EGalbrenaClimbExitType::END;
			m_IsClimbSecondStep = false;

			// Hit
			m_eHitType = EGalbrenaHitType::END;

			// Interaction
			m_eRopeHookType = EGalbrenaRopeHookType::END;

			// Capture
			m_eCaptureType = EGalbrenaCaptureType::END;

			m_strPrevInfo.clear(); // String 비우기.

		};
	};

	StateTransitionContext m_StateContext;


public:
	StateTransitionContext& GetStateContextForWrite()
	{
		return m_StateContext;
	};

	StateTransitionContext TakeStateContext()
	{
		StateTransitionContext tempCopy = m_StateContext;
		m_StateContext = {}; 
		return tempCopy; 
	}

#pragma endregion
public:
	enum PARTTYPE : _uint
	{
		PART_FIRSTGUN = 0,		// 기본 무기.?
		PART_SECONDGUN,		// 기본 무기.?
		PART_LION,
		PART_DARKWING,
		PART_WING,
		TYPE_END
	};

#pragma region 0. 기본 함수들.
protected:
	explicit CGalbrena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGalbrena(const CGalbrena& Prototype);
	virtual ~CGalbrena() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_OutLine() override;
	virtual void	Render_Shadow() override;


#pragma endregion


#pragma region 1. STATE
public:
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) override;
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) override;
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	virtual void Hit_Judge(void* pArg = nullptr) override;
	virtual void Grab_Judge(void* pArg = nullptr) override;
	void Sync_Position();

	virtual void Bind_QTE(_bool IsQTE) override;
	virtual void Reset_QTECamera() override;
	virtual void Bind_QTECamera() override;
#pragma region 2. NOTIFY
	public:
		virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
		virtual void Effect_Active(const _wstring& wStrEffectTag) override;
		virtual void Object_Func(const _wstring& wStrObjectTag) override;
#pragma endregion

#pragma region 3. CALL BACK
	public:
		void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
#pragma endregion

#pragma region 4. EVENT
public:
	virtual void Process_DelayedActions(_float fTimeDelta);
	virtual void Bind_ChangeEffect() override; // ChaneEffect 실행.

	virtual void Bind_DissolveTimer() override;
	virtual void Bind_DefaultShaderPath() override;
	virtual void Bind_DissolveShaderPath() override;
	virtual void Activate(_bool IsActivate) override;
#pragma endregion

#pragma endregion
private:
	class CGalbrenaShotGun* m_pGalbrenaFirstShotGun = { nullptr };
	class CGalbrenaShotGun* m_pGalbrenaSecondShotGun = { nullptr };
	class CGalbrenaDarkWing* m_pGalbrenaDarkWing = { nullptr };
	class CWing* m_pWing = { nullptr };
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END };

	// Attack Volume
	_uint m_iVolumeIdx = {};
	vector<class CAttackVolume*> m_AttackVolumes;

	_bool m_PendingConditions[CONDITION_END] = {};

	// Shader Value
	_uint m_iGalbrenaMaskIndex = {};
	_float4 m_vMaskEmssiveColor = {};


private:
	void Bind_TargetToVolumes();
	void Update_TargetDistance();
	void Update_Physics(_float fTimeDelta);
	void Update_Camera(_float fTimeDelta);
	void Render_Default(_uint iMeshIndex);
	void Render_Skin(_uint iMeshIndex);
	void Render_Back(_uint iMeshIndex);
	void Render_Eye(_uint iMeshIndex);
	_bool IsSkin(_uint iMeshIndex);
	_bool IsBack(_uint iMeshIndex);
	_bool IsEye(_uint iMeshIndex);



private:
	void Bind_Resources();
	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);
	void Ready_AttackVolumes();

public:
	static		CGalbrena* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

