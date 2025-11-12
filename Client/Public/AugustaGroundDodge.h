#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Dodge State - Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R 처리
class CAugustaGroundDodge final : public CGroundState
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
    explicit CAugustaGroundDodge() = default;
    virtual ~CAugustaGroundDodge() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SprintAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CAugustaGroundDodge* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
