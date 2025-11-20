#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CAugustaGroundQTE final : public CGroundState
{
private:
    enum QTESTATE
    {
		QTE = 0,
		SELECT,
		MOVE,
		LAND,
		FALL,
		END
    };

private:
    explicit CAugustaGroundQTE() = default;
    virtual ~CAugustaGroundQTE() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[QTESTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_QTEAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CAugustaGroundQTE* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
