#include "ClientPch.h"
#include "UI_Dialog.h"

CUI_Dialog::CUI_Dialog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI( pDevice, pContext )
{
}

CUI_Dialog::CUI_Dialog(const CUI_Dialog& Prototype)
	: CCustom_UI( Prototype )
{
}

HRESULT CUI_Dialog::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Dialog::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CUI_Dialog::Priority_Update(_float fTimeDelta)
{
}

void CUI_Dialog::Update(_float fTimeDelta)
{
}

void CUI_Dialog::Late_Update(_float fTimeDelta)
{
}

void CUI_Dialog::Render()
{
}

void CUI_Dialog::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

HRESULT CUI_Dialog::Ready_Components(void* pArg)
{
    return S_OK;
}

void CUI_Dialog::PreAssign_ChildUIs()
{
}

void CUI_Dialog::PreAssign_Presets()
{
}

void CUI_Dialog::Create_ChildText()
{
}

CUI_Dialog* CUI_Dialog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CUI_Dialog::Clone(void* pArg)
{
    return nullptr;
}

void CUI_Dialog::Free()
{
}
