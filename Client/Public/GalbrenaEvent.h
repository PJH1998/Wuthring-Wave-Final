#pragma once
#include "InteractionState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CGalbrenaEvent final : public CInteractionState
{
private:
    enum EventSTATE
    {
        RUN = 0,
		EXIT,
        END
    };

private:
    explicit CGalbrenaEvent() = default;
    virtual ~CGalbrenaEvent() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[EventSTATE::END] = {};
	_bool m_IsStopOnce = { false } ;

private:
    virtual void Handle_Input() override;
    void Update_EventAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CGalbrenaEvent* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
