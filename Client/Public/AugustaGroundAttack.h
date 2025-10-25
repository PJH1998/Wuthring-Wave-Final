#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaGroundAttack final : public CGroundState
{
private:
    explicit CAugustaGroundAttack() = default;
    virtual ~CAugustaGroundAttack() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _uint m_iComboCount = { 0 };  // 현재 콤보 단계 (0~3)
    _bool m_bCanCombo = { false };  // 콤보 입력 가능 여부

    void Update_ComboChain();
    void Check_StateTransition();

public:
    static CAugustaGroundAttack* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
