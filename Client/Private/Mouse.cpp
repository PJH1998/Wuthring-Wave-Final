#include "ClientPch.h"
#include "Mouse.h"

CMouse::CMouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMouse::CMouse(const CMouse& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CMouse::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMouse::Initialize_Clone(void* pArg)
{
	m_iCursorPosX = g_iWinSizeX >> 1;
	m_iCursorPosY = g_iWinSizeY >> 1;

    return S_OK;
}

void CMouse::Priority_Update(_float fTimeDelta)
{
}

void CMouse::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_M) == KEYSTATE::DOWN)
		m_isMouseOn = !m_isMouseOn;

	if(true == m_isMouseOn)
		ShowCursor(true);
	else
	{
		ShowCursor(false);
		SetCursorPos(m_iCursorPosX, m_iCursorPosY);
	}
}

void CMouse::Late_Update(_float fTimeDelta)
{
}

void CMouse::Render()
{
}

CMouse* CMouse::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMouse* pInstance = new CMouse(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Mouse");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMouse::Clone(void* pArg)
{
	CMouse* pClone = new CMouse(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mouse");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMouse::Free()
{
	__super::Free();
}
