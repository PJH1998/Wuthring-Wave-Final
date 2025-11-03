#include "EditorPch.h"
#include "SQ_Camera_Edit.h"

CSQ_Camera_Edit::CSQ_Camera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSQ_Item_Edit { pDevice, pContext }
{
}

CSQ_Camera_Edit::CSQ_Camera_Edit(const CSQ_Camera_Edit& Prototype)
	: CSQ_Item_Edit { Prototype }
{
}

HRESULT CSQ_Camera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_Camera_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Actor_Edit")

    return S_OK;
}

void CSQ_Camera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Render()
{
}

void CSQ_Camera_Edit::Render_Shadow()
{
}

void CSQ_Camera_Edit::Render_OutLine()
{
}

void CSQ_Camera_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

CSQ_Camera_Edit* CSQ_Camera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_Camera_Edit* pInstance = new CSQ_Camera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_Camera_Edit")

    return pInstance;
}

CGameObject* CSQ_Camera_Edit::Clone(void* pArg)
{
	CSQ_Camera_Edit* pClone = new CSQ_Camera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_Camera_Edit (Clone)")

	return pClone;
}

void CSQ_Camera_Edit::Free()
{
	__super::Free();
}
