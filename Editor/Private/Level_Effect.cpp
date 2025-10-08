#include "EditorPch.h"
#include "Level_Effect.h"

#include "Event_Level.h"

CLevel_Effect::CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Effect::Initialize()
{


    return S_OK;
}

void CLevel_Effect::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Effect"));

    
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
            Particle_Tab();

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
}

void CLevel_Effect::Particle_Tab()
{
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

}
