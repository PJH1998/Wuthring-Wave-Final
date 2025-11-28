#include "ClientPch.h"
#include "AugustaCapture.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaCapture::Initialize(CGameObject* pOwner)
{
	if (FAILED(CCaptureState::Initialize(pOwner)))
		return E_FAIL;


	m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
	ASSERT_CRASH(m_pAugusta);


	// 1. 애니메이션 설정
	Setup_Animations();
    return S_OK;
}

void CAugustaCapture::OnEnter(void* pArg)
{
}

void CAugustaCapture::OnUpdate(_float fTimeDelta)
{
}

void CAugustaCapture::OnExit()
{
}

void CAugustaCapture::Handle_Input()
{
}

void CAugustaCapture::Update_RopeAnimation(_float fTimeDelta)
{
}

void CAugustaCapture::Check_Physics(_float fTimeDelta)
{
}

void CAugustaCapture::Check_StateTransition(_float fTimeDelta)
{
}

void CAugustaCapture::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::CAPTURED), "Captured", 1.5f, 20.f);
}

void CAugustaCapture::State_Reset()
{
}

CAugustaCapture* CAugustaCapture::Create(CGameObject* pOwner)
{
	return nullptr;
}

void CAugustaCapture::Free()
{
}
