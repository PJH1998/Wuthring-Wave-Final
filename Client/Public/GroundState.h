#pragma once
#include "CharacterState.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)

// 모든 캐릭터가 공유하는 지상 상태 (카테고리 State)
class CGroundState abstract : public CCharacterState
{
protected:
    explicit CGroundState() = default;
    virtual ~CGroundState() = default;

public:
    virtual HRESULT Initialize(CCharacter* pOwner);
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


protected:
	_uint m_iNotLandFrames = {};
	const _uint MAX_NOT_LAND_FRAMES = 2;  // 3프레임 여유 // 공중에 15프레임 까진..

protected:
    void Apply_Gravity(_float fTimeDelta);
    void Check_GroundCollision();



    

public:
    //void Change_SubState(const _string& strSubStateName, class CStateMachine* pStateMachine);

    virtual void Free() override;
};

NS_END
