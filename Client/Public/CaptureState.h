#pragma once
#include "CharacterState.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)
class CCaptureState abstract : public CCharacterState
{
protected:
	explicit CCaptureState() = default;
	virtual ~CCaptureState() = default;

public:
	virtual HRESULT Initialize(class CGameObject* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;


public:
	virtual void Free() override;

};
NS_END

