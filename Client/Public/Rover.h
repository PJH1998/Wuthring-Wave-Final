#pragma once
#include "Player_Define.h"
#include "RoverState_Enum.h"

NS_BEGIN(Client)
class CRover final : public CCharacter
{
#pragma region STATE ���� ���濡 ���.
private:
	struct StateTransitionContext
	{
		// Ground
		ERoverDashType m_eDashType = ERoverDashType::END;
		ERoverIdleType  m_eIdleType = ERoverIdleType::END;
		ERoverRunType m_eRunType = ERoverRunType::END;
		ERoverLandType m_eLandType = ERoverLandType::END;

		ERoverAttackType m_eAttackType = ERoverAttackType::END;
		ERoverSkillType m_eSkillType = ERoverSkillType::END;
		ERoverUniqueType m_eUniqueType = ERoverUniqueType::END;
		ERoverBurstType m_eBurstType = ERoverBurstType::END;
		ERoverSpecialType m_eSpecialType = ERoverSpecialType::END;

		// Air
		ERoverJumpType m_eJumpType = ERoverJumpType::END;
		ERoverFallType m_eFallType = ERoverFallType::END;
		ERoverAirAttackType m_eAirAttackType = ERoverAirAttackType::END;

		// Climb
		ERoverClimbIdleType m_eClimbIdleType = ERoverClimbIdleType::END;
		ERoverClimbMoveType m_eClimbMoveType = ERoverClimbMoveType::END;
		ERoverClimbExitType m_eClimbExitType = ERoverClimbExitType::END;
		_bool m_IsClimbSecondStep = { false };

		// Hit
		ERoverHitType m_eHitType = ERoverHitType::END;

		// ���ؽ�Ʈ ��� �� �ʱ�ȭ
		void Clear()
		{
			// Land
			m_eIdleType = ERoverIdleType::END;
			m_eRunType = ERoverRunType::END;
			m_eDashType = ERoverDashType::END;
			m_eLandType = ERoverLandType::END;

			// Attack
			m_eAttackType = ERoverAttackType::END;
			m_eSkillType = ERoverSkillType::END;
			m_eUniqueType = ERoverUniqueType::END;
			m_eBurstType = ERoverBurstType::END;
			m_eSpecialType = ERoverSpecialType::END;

			// Air
			m_eJumpType = ERoverJumpType::END;
			m_eFallType = ERoverFallType::END;
			m_eAirAttackType = ERoverAirAttackType::END;

			// Climb
			m_eClimbIdleType = ERoverClimbIdleType::END;
			m_eClimbMoveType = ERoverClimbMoveType::END;
			m_eClimbExitType = ERoverClimbExitType::END;
			m_IsClimbSecondStep = false;

			m_eHitType = ERoverHitType::END;
		};
	};

	StateTransitionContext m_StateContext;


public:
	// ���� State���� ȣ��
	StateTransitionContext& GetStateContextForWrite()
	{
		return m_StateContext;
	};

	// ȣ�� �޴� State
	StateTransitionContext TakeStateContext()
	{
		StateTransitionContext tempCopy = m_StateContext; // ���� ���ؽ�Ʈ�� ����
		m_StateContext = {}; // ���� ���ؽ�Ʈ�� ��� ��� �⺻�� �ʱ�ȭ)
		return tempCopy; // ���纻�� ��ȯ
	}

#pragma endregion
public:
	enum PARTTYPE : _uint
	{
		PART_WEAPON = 0,
		TYPE_END
	};

#pragma region 0. �⺻ �Լ�
protected:
	explicit CRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRover(const CRover& Prototype);
	virtual ~CRover() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;
#pragma endregion


#pragma region 1. STATE ����.
public:
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true) override;
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	virtual void Hit_Judge(void* pArg = nullptr) override;
	void Sync_Position();

#ifdef _DEBUG
public:
	virtual void PartRotation(_uint iPartType, _fvector vQuaternion);
#endif // _DEBUG

#pragma region 2. NOTIFY
	public:
		virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
		virtual void Effect_Active(const _wstring& wStrEffectTag) override;

#pragma endregion


#pragma endregion
private:
	class CRoverWeapon* m_pRoverWeapon = { nullptr };
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END }; // State���� Ȱ��ȭ?

	
#ifdef _DEBUG
	// RayCast ����
	vector<pair<_float, _float>> m_RayCasts = {};
#endif // _DEBUG



private:
	// Runtime ���� �ʿ��� ���� ���� �غ�.
	void Bind_Resources();

	// �ʱ� ���� ���� �غ�.
	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);

public:
	static		CRover* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

