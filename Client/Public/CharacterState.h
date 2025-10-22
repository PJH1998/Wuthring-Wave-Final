#pragma once
#include "State.h"

NS_BEGIN(Client)

// 모든 캐릭터 State의 최상위 부모 클래스
class CCharacterState abstract : public CState
{
protected:
    explicit CCharacterState() = default;
    virtual ~CCharacterState() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

protected:
    _bool Play_Animation(class CCharacter* pCharacter, _float fTimeDelta);

protected:
    // 입력 키 ( State 마다 사용할 변수)
    _uint m_iMoveKey = {};

public:
    virtual void Free() override;
};

NS_END
