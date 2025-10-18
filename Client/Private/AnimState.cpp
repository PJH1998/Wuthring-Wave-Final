#include "ClientPch.h"
#include "AnimState.h"
#include "Model.h"
#include "AnimMacnine.h"

HRESULT CAnimState::Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	m_StateData = StateDesc;
	m_strAnimationTag = strAnimationTag;
    return S_OK;
}

void CAnimState::Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag)
{
}

void CAnimState::Update(_float fTimeDelta, CAnimMachine* pAnimMachine, _uint* pOwnerState, _string* pCurrentAnimTag, ANIMSTATE_DESC& StateData)
{
}

void CAnimState::Exit(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag)
{
}

CAnimState* CAnimState::Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(strAnimationTag)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CAnimState::Free()
{
    __super::Free();
}
