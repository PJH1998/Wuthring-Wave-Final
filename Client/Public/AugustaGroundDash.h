#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Sprint State - Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R 처리
class CAugustaGroundDash final : public CGroundState
{
private:
    enum DASHSTATE
    {
        JUMP = 0,
        MOVE,
        END
    };

private:
    explicit CAugustaGroundDash() = default;
    virtual ~CAugustaGroundDash() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SprintAnimation(_float fTimeDelta);
    void LockOnCheck_StateTransition(_float fTimeDelta);  
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CAugustaGroundDash* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
