#include "EditorPch.h"
#include "Level_Shader.h"

#include "Event_Level.h"
#include "Shader_Interface.h"

CLevel_Shader::CLevel_Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Shader::Initialize()
{
    if (FAILED(Ready_Interface()))
        CRASH("Failed Interface");

    return S_OK;
}

void CLevel_Shader::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Shader"));
    m_pShader_Interface->Update_Shadow();
}

void CLevel_Shader::Render()
{

}

HRESULT CLevel_Shader::Ready_Interface()
{
    m_pShader_Interface = CShader_Interface::Create(m_pDevice, m_pContext);
    ASSERT_CRASH(m_pShader_Interface);

    return S_OK;
}

HRESULT CLevel_Shader::Ready_TestObjects()
{
    return E_NOTIMPL;
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

    Safe_Release(m_pShader_Interface);
}
