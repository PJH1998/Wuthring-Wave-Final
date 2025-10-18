#include "EditorPch.h"
#include "Level_Shader.h"

#include "Event_Level.h"

CLevel_Shader::CLevel_Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Shader::Initialize()
{
    return S_OK;
}

void CLevel_Shader::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Shader"));
}

void CLevel_Shader::Render()
{

}

CLevel_Shader* CLevel_Shader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Shader* pInstance = new CLevel_Shader(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Shader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Shader::Free()
{
    __super::Free();

}
