#include "ClientPch.h"
#include "Wing.h"

CWing::CWing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProp { pDevice, pContext }
{
}

CWing::CWing(const CPartObject& Prototype)
	: CProp (Prototype)
{
}

HRESULT CWing::Initialize_Prototype()
{
	if (FAILED(CProp::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CWing::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CWing::Priority_Update(_float fTimeDelta)
{
}

void CWing::Update(_float fTimeDelta)
{
}

void CWing::Late_Update(_float fTimeDelta)
{
}

void CWing::Render()
{
}

void CWing::Activate(_bool IsActivate)
{
}

CWing* CWing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CWing::Clone(void* pArg)
{
    return nullptr;
}

void CWing::Free()
{
}
