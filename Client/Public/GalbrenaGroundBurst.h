#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Skill State - , Burst01 처리
// Default Skill State
class CGalbrenaGroundBurst final : public CGroundState
{
private:
    enum BURSTSTATE
    {
	   ATTACK = 0,
	   MOVE,
	   JUMP,
	   LAND,
	   NEXT_SKILL,
       END
    };

private:
    explicit CGalbrenaGroundBurst() = default;
    virtual ~CGalbrenaGroundBurst() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CGalbrena* m_pGalbrena = { nullptr };
    _bool m_States[BURSTSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SkillAnimations(_float fTimeDelta);
    void Check_Physcis(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta) ;
    
    void SetUp_Animations();
    void State_Reset();

public:
    static CGalbrenaGroundBurst* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
