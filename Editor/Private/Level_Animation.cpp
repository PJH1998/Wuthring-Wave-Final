#include "EditorPch.h"
#include "Level_Animation.h"

#include "Event_Level.h"
#include "AnimationTool.h"

CLevel_Animation::CLevel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Animation::Initialize()
{
    m_pAnimationTool = CAnimationTool::Create(m_pDevice, m_pContext, m_eCurLevel);

    /* 임시 쉐이더 추가. */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }

    return S_OK;
}

void CLevel_Animation::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Anim"));

}

void CLevel_Animation::Render()
{
    m_pAnimationTool->Render();
}

CLevel_Animation* CLevel_Animation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Animation* pInstance = new CLevel_Animation(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Animation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Animation::Free()
{
    __super::Free();
    Safe_Release(m_pAnimationTool);

}
