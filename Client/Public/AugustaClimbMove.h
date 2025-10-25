#pragma once
#include "ClimbState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CAugustaClimbMove final : public CClimbState
{
private:
    explicit CAugustaClimbMove() = default;
    virtual ~CAugustaClimbMove() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;
    

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_IsSecondStep = { false }; 
    _bool m_IsClimbed = { false };
    _bool m_IsClimbStop = { false }; // 키입력이 없는 상황이면 Climb Stop

private:
    virtual void Handle_Input() override;
    void Update_PhysicsCheck(_float fTimeDelta);
    void Update_ClimbAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();



public:
    static CAugustaClimbMove* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
