#include "EditorPch.h"
#include "Level_Effect.h"

#include "Event_Level.h"
#include "Texture.h"

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

    //테스트용 텍스처 불러오기
    Texture_Loading("TEST1", TEXT("../../Client/Bin/Resource/Effect/Texture/T_Spark_300012.png"));

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
            Particle_Tab();

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
}

void CLevel_Effect::Texture_Loading(const char* TextureName, const _tchar* pFilePath)
{
    PARTICLE_TEXTURE Desc{};
    CTexture* pTexture = {};

    Desc.szName = TextureName;

    _char szDefault[MAX_PATH];

    strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");

    strcat_s(szDefault, Desc.szName);

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        pTexture = CTexture::Create(m_pDevice, m_pContext, pFilePath, 1));

    Desc.pTexture = pTexture;
    Safe_AddRef(pTexture);

    m_Textures.push_back(Desc);
}

void CLevel_Effect::Particle_Tab()
{
    //텍스처 이미지 설정
    if (ImGui::BeginCombo("Texture", "Texture")) {
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

    //파티클 태그
    if (ImGui::InputText("ParticleTag", m_ParticleTag, IM_ARRAYSIZE(m_ParticleTag), ImGuiInputTextFlags_EnterReturnsTrue))
        m_bTagFlag = true;

    if (m_bTagFlag)
    {
        if (ImGui::Button("Create"))
        {
            if (m_bTagFlag)
            {
                //기본베이스로 생성
                _tchar ParticleTag[MAX_PATH] = {};
                CParticle::PARTICLE_DESC ParticleDesc{};
                CVIBuffer_Point_Instance::POINT_INSTANCE_DESC VIBufferDesc{};
                CVIBuffer_Point_Instance* pPoint_Instance{};
                CParticle* pParticle;

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_ParticleTag, strlen(m_ParticleTag), ParticleTag, MAX_PATH);

                ParticleDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                ParticleDesc.strVIBufferTag = TEXT("Prototype_Componenet_VIBuffer_Instance_Point_");
                ParticleDesc.strVIBufferTag += ParticleTag;

                //버퍼 최소 설정 값
                VIBufferDesc.iNumInstance = 1;
                VIBufferDesc.vSize = _float2(10.f, 10.f);

                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), ParticleDesc.strVIBufferTag,
                    pPoint_Instance = CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &VIBufferDesc));

                pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, &ParticleDesc));

                m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Particle"), pParticle);

                m_Particles.emplace(ParticleTag, pParticle);
                Safe_AddRef(pParticle);

                m_VIBuffers.emplace(ParticleTag, pPoint_Instance);
                Safe_AddRef(pPoint_Instance);

                m_tParticleDesc.emplace(ParticleTag, ParticleDesc);
                m_tVBDesc.emplace(ParticleTag, VIBufferDesc);

                m_bTagFlag = false;
            }
        }
    }

    if (!m_Particles.empty())
    {
        if (ImGui::Begin("Particle Info"))
        {
        

            ////파티클 설정값 VIBuffer
            // if (ImGui::CollapsingHeader("VIBuffer", ImGuiTreeNodeFlags_DefaultOpen))
            // {
            //     ImGui::Checkbox("Loop", &m_tVBDesc.IsLoop);

            //     ImGui::Text("NumInstance");
            //     ImGui::PushItemWidth(100);
            //     ImGui::DragInt("##NumInstance", (int*)&m_tVBDesc.iNumInstance);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("Center");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##CenterX", &m_tVBDesc.vCenter.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##CenterY", &m_tVBDesc.vCenter.y);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##CenterZ", &m_tVBDesc.vCenter.z);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("Size");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##SizeX", &m_tVBDesc.vSize.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##SizeY", &m_tVBDesc.vSize.y);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("Range");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##RangeX", &m_tVBDesc.vRange.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##RangeY", &m_tVBDesc.vRange.y);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##RangeZ", &m_tVBDesc.vRange.z);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("Pivot");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##PivotX", &m_tVBDesc.vPivot.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##PivotY", &m_tVBDesc.vPivot.y);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##PivotZ", &m_tVBDesc.vPivot.z);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("Speed");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##SppedX", &m_tVBDesc.vSpeed.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##SppedY", &m_tVBDesc.vSpeed.y);
            //     ImGui::PopItemWidth();

            //     ImGui::Text("LifeTime");
            //     ImGui::PushItemWidth(60);
            //     ImGui::InputFloat("##LifeTimeX", &m_tVBDesc.vLifeTime.x);
            //     ImGui::SameLine();
            //     ImGui::InputFloat("##LifeTimeY", &m_tVBDesc.vLifeTime.y);
            //     ImGui::PopItemWidth();
            // }

            //if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen))
            //{
            //    ImGui::Checkbox("Spread", &m_tParticleDesc.bSpread);
            //    ImGui::Checkbox("Drop", &m_tParticleDesc.bDrop);

            //    ImGui::Text("Size");
            //    ImGui::PushItemWidth(60);
            //    ImGui::InputFloat("##ParticleSizeX", &m_tParticleDesc.vSize.x);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticleSizeY", &m_tParticleDesc.vSize.y);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticleSizeZ", &m_tParticleDesc.vSize.z);
            //    ImGui::PopItemWidth();

            //    ImGui::Text("Position");
            //    ImGui::PushItemWidth(60);
            //    ImGui::InputFloat("##ParticlePosX", &m_tParticleDesc.vPos.x);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticlePosY", &m_tParticleDesc.vPos.y);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticlePosZ", &m_tParticleDesc.vPos.z);
            //    ImGui::PopItemWidth();

            //    ImGui::Text("Color");
            //    ImGui::PushItemWidth(60);
            //    ImGui::InputFloat("##ParticleColorX", &m_tParticleDesc.vColor.x);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticleColorY", &m_tParticleDesc.vColor.y);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticleColorZ", &m_tParticleDesc.vColor.z);
            //    ImGui::PopItemWidth();

            //    ImGui::Text("LifeTime");
            //    ImGui::PushItemWidth(60);
            //    ImGui::InputFloat("##ParticleLifeTimeX", &m_tParticleDesc.vLifeTime.x);
            //    ImGui::SameLine();
            //    ImGui::InputFloat("##ParticleLifeTimeY", &m_tParticleDesc.vLifeTime.y);
            //    ImGui::PopItemWidth();
            //}

            ImGui::End();
        }
    }

}

CParticle::PARTICLE_DESC CLevel_Effect::Find_Particle(_tchar ParticleTag)
{
    for (auto iter = m_tParticleDesc.begin(); iter != m_tParticleDesc.end(); )
    {
        if (iter->first == &ParticleTag)
        {
            return iter->second;
        }
        else
            ++iter;
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

    for (auto& pTexture : m_Textures)
        Safe_Release(pTexture.pTexture);

    for (auto& pParticle : m_Particles)
        Safe_Release(pParticle.second);

    for (auto& pVB : m_VIBuffers)
        Safe_Release(pVB.second);
}
