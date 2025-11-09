#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Rover Skill State - , Burst01 처리
// Default Skill State
class CRoverGroundBurst final : public CGroundState
{
private:
    enum BURSTSTATE
    {
       BURST01 = 0,
	   ATTACK,
	   MOVE,
	   JUMP,
	   LAND,
       END
    };

private:
    explicit CRoverGroundBurst() = default;
    virtual ~CRoverGroundBurst() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CRover* m_pRover = { nullptr };
    _bool m_States[BURSTSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SkillAnimations(_float fTimeDelta);
    void Check_Physcis(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta) ;
    
    void SetUp_Animations();
    void State_Reset();

public:
    static CRoverGroundBurst* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
