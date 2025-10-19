#include "EditorPch.h"
#include "Level_Camera.h"

CLevel_Camera::CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Camera::Initialize()
{
    return S_OK;
}

void CLevel_Camera::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Camera"));
}

void CLevel_Camera::Render()
{
}

CLevel_Camera* CLevel_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Camera* pInstance = new CLevel_Camera(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Level_Camera");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Camera::Free()
{
	__super::Free();
}
