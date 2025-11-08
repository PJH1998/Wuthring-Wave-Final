#include "EditorPch.h"
#include "Decal_Controller.h"

CDecal_Controller::CDecal_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CDecal_Controller::Initialize()
{

    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/Decal/Texture");

    return S_OK;
}

void CDecal_Controller::Update()
{
    Decal_Tab();
}

void CDecal_Controller::Render()
{

}

void CDecal_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png")
            {
                DECAL_TEXTURE Desc{};
                CTexture* pTexture = {};

                _string strTextureTag = entry.path().stem().string();

                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                _wstring wstrFilePath = StringToWString(filePath);
   
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

void CDecal_Controller::Decal_Tab()

{
    if (m_bSelectedDecal)
    {
        if (ImGui::Begin("Decal Info"))
        {
           
                if (ImGui::CollapsingHeader("Decal", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Root", &(m_pSelectedDecalDesc->IsRootOn));

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##DecalLifeTime", &(m_pSelectedDecalDesc->LifeTime));
                    ImGui::PopItemWidth();

                    if (ImGui::ColorEdit4("Color", m_fColor, 
						ImGuiColorEditFlags_NoOptions          // 설정 메뉴 비활성화 (HSV 등 변환 방지)
						| ImGuiColorEditFlags_NoInputs         // 텍스트 입력 비활성 (정확히 선택한 색 유지)
						| ImGuiColorEditFlags_DisplayRGB       // 항상 RGB로 표시
						| ImGuiColorEditFlags_InputRGB         // RGB 입력값으로 유지
						| ImGuiColorEditFlags_AlphaBar         // 알파 바 표시
						| ImGuiColorEditFlags_AlphaPreview))   // 알파 미리보기
                    {
                        m_pSelectedDecalDesc->vColor = _float4(m_fColor[0], m_fColor[1], m_fColor[2], m_fColor[3]);
                    }

                    ImGui::Separator();
                  
                }

                if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::BeginCombo("Texture", "")) {
                        for (size_t i = 0; i < m_Textures.size(); i++)
                        {
                            bool IsSelected = (m_iSelectedTexture == i);
                            if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                            {
                                m_iSelectedTexture = i;
                            }
                            if (IsSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::Separator();
                    if (m_iSelectedTexture >= 0) {
                        ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
                    }
                }

            ImGui::End();
        }
    }

}

void CDecal_Controller::Decal_Base_Tab(CEffect_Decal::DECAL_DESC& tDecalDesc, _bool& IsCreate)
{
 /*   if (ImGui::Begin("Decal Base"))
    {
        if (ImGui::BeginCombo("Texture", "")) {
            for (size_t i = 0; i < m_Textures.size(); i++)
            {
                bool IsSelected = (m_iSelectedTexture == i);
                if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                    m_iSelectedTexture = i;

                if (IsSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        if (m_iSelectedTexture >= 0) {
            ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

        if (m_bTagFlag)
        {
            if (ImGui::Button("Create"))
            {
                _tchar DecalTag[MAX_PATH] = {};
                CEffect_Decal::FXDecal_DESC DecalDesc{};
                CEffect_Decal* pDecal;

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_DecalTag, strlen(m_DecalTag), DecalTag, MAX_PATH);

                DecalDesc.strMyTag = DecalTag;
                DecalDesc.eMyType = EFFECT_TYPE::Decal;
                DecalDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;

                m_tDecalDesc.emplace(DecalTag, DecalDesc);

                tDecalDesc = DecalDesc;

                m_DecalTag[0] = _T('\0');

                IsCreate = true;
                m_bTagFlag = false;
            }
        }
        ImGui::End();
    }*/
}

void CDecal_Controller::UpdateSelected_DecalFormTag(_wstring DecalTag)
{

    auto iterDecalDesc = m_tDecalDesc.find(DecalTag);
    
    if (iterDecalDesc == m_tDecalDesc.end())
    {
        m_pSelectedDecalDesc = nullptr;
    }
    else
    {
        m_pSelectedDecalDesc = &iterDecalDesc->second;
    }


    if (m_pSelectedDecalDesc != nullptr)
    {
        m_bSelectedDecal = true;

        m_fColor[0] = m_pSelectedDecalDesc->vColor.x;
        m_fColor[1] = m_pSelectedDecalDesc->vColor.y;
        m_fColor[2] = m_pSelectedDecalDesc->vColor.z;
        m_fColor[3] = m_pSelectedDecalDesc->vColor.w;
    }
}

CEffect_Decal::DECAL_DESC* CDecal_Controller::Get_DecalDesc(_wstring& DecalTag)
{
    auto iter = m_tDecalDesc.find(DecalTag);
    
    if (iter == m_tDecalDesc.end())
        return nullptr;

    return &iter->second;
}

void CDecal_Controller::Set_DecalDesc(_wstring& DecalTag, CEffect_Decal::DECAL_DESC& DecalDesc)
{
	CEffect_Decal::DECAL_DESC Desc = {};
    Desc = DecalDesc;

    m_tDecalDesc.emplace(DecalTag, Desc);
}

void CDecal_Controller::Set_DecalTag(const _char* szDecalTag)
{
    strcat_s(m_DecalTag, szDecalTag);

    m_bTagFlag = true;
}
void CDecal_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterDecalDesc = m_tDecalDesc.find(DescTag);

    if (iterDecalDesc != m_tDecalDesc.end())
    {
        m_tDecalDesc.erase(iterDecalDesc);
    }


    m_iSelectedDecal = 0;
    m_bSelectedDecal = false;
    m_pSelectedDecalDesc = nullptr;
   
    m_fColor[0] = 1.f;
    m_fColor[1] = 1.f;
    m_fColor[2] = 1.f;
    m_fColor[3] = 1.f;
}


CDecal_Controller* CDecal_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDecal_Controller* pInstance = new CDecal_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CDecal_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CDecal_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();
}