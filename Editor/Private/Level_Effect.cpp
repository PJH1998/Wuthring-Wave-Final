#include "EditorPch.h"
#include "Level_Effect.h"
#include "Event_Level.h"
#include "Particle_Controller.h"

CLevel_Effect::CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Effect::Initialize()
{
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"),
        CParticle::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements));

    m_pParticle_Controller = CParticle_Controller::Create(m_pDevice, m_pContext);


    return S_OK;
}

void CLevel_Effect::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Effect"));

    if(ImGui::Begin("Effect"))
        Effect_MenuBar();
    ImGui::End();
}

void CLevel_Effect::Render()
{

}

void CLevel_Effect::Effect_MenuBar()
{
    if (ImGui::BeginTabBar("Effect"))
    {

        if (ImGui::BeginTabItem("Particle"))
        {
            m_pParticle_Controller->Update();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

CLevel_Effect* CLevel_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Effect* pInstance = new CLevel_Effect(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Effect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Effect::Free()
{
    __super::Free();

    Safe_Release(m_pParticle_Controller);
}
