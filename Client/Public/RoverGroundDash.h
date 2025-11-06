#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Rover Sprint State - Sprint_F
class CRoverGroundDash final : public CGroundState
{
private:
    enum DASHSTATE
    {
        JUMP = 0,
        MOVE,
		LAND,
        END
    };

private:
    explicit CRoverGroundDash() = default;
    virtual ~CRoverGroundDash() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };
    _bool m_States[END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SprintAnimation(_float fTimeDelta);
    void LockOnCheck_StateTransition(_float fTimeDelta);  
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CRoverGroundDash* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
