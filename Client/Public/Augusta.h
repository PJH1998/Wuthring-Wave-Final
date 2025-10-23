#pragma once
#include "Player_Define.h"
#include "AugustaState_Enum.h"


NS_BEGIN(Client)
class CAugusta final : public CCharacter
{
#pragma region STATE 상태 변경에 사용.
private:
	struct StateTransitionContext
	{
		EIdleType  m_eIdleType = EIdleType::END;
		ERunType m_eRunType = ERunType::END;
		ESkillType m_eSkillToPlay = ESkillType::END;

		// 컨텍스트 사용 뒤 초기화
		void Clear()
		{
			m_eSkillToPlay = ESkillType::END;
			m_eIdleType = EIdleType::END;
			m_eRunType = ERunType::END;
		};
	};

	StateTransitionContext m_StateContext;


public:
	// 현재 State에서 호출
	StateTransitionContext& GetStateContextForWrite()
	{
		return m_StateContext;
	};

	// 호출 받는 State
	StateTransitionContext TakeStateContext()
	{
		StateTransitionContext tempCopy = m_StateContext; // 현재 컨텍스트를 복사
		m_StateContext = {}; // 원본 컨텍스트를 즉시 비움 기본값 초기화)
		return tempCopy; // 복사본을 반환
	}

#pragma endregion


public:
	enum PARTTYPE : _uint
	{
		PART_WEAPON = 0,
		PART_SHIELD = 1,
		TYPE_END
	};

#pragma region 기본 함수
protected:
	explicit CAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugusta(const CAugusta& Prototype);
	virtual ~CAugusta() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;
#pragma endregion



private:

	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };

private:
	// Runtime 도중 필요한 값에 대한 준비.
	void Bind_Resources();

	// 초기 값에 대한 준비.
	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);

public:
	static		CAugusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

