#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Rover Dodge State - Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R 처리
class CRoverGroundDodge final : public CGroundState
{
private:
    enum DASHSTATE
    {
        JUMP = 0,
        MOVE,
		LOCKON_W,
		LOCKON_S,
		LOCKON_A,
		LOCKON_D,
		LAND,
		LOCKON,
        END
    };

private:
    explicit CRoverGroundDodge() = default;
    virtual ~CRoverGroundDodge() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };
    _bool m_States[END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SprintAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CRoverGroundDodge* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
