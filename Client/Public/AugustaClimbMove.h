#pragma once
#include "ClimbState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CAugustaClimbMove final : public CClimbState
{
private:
    enum CLIMBSTATE
    {
        IS_CLIMBED = 0,
        MOVE,
        U,
        D,
        R,
        L,
        WALL,
        ONTOP,
        VAULT,
        BACKJUMP,
        LAND,
        END
    };

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
    _bool m_States[CLIMBSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_ClimbAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Adjust_To_Wall(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();



public:
    static CAugustaClimbMove* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
