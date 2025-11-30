#pragma once
#include "CharacterState.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)
class CInteractionState abstract : public CCharacterState
{
protected:
	explicit CInteractionState() = default;
	virtual ~CInteractionState() = default;

public:
	virtual HRESULT Initialize(CCharacter* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;


protected:
	_uint m_iNotLandFrames = {};


public:
	virtual void Free() override;
};
NS_END

