#include "EditorPch.h"
#include "Particle_Controller.h"

CParticle_Controller::CParticle_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CParticle_Controller::Initialize()
{
    //?띿뒪泥?遺덈윭?ㅺ린
    //Texture_Loading("TEST1", TEXT("../../Client/Bin/Resource/Effect/Texture/T_Spark_300012.png"));

    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/Texture");

    return S_OK;
}

void CParticle_Controller::Update()
{
    Particle_Tab();
}

void CParticle_Controller::Render()
{

}

//void CParticle_Controller::Texture_Loading(const char* TextureName, const _tchar* pFilePath)
//{
//    PARTICLE_TEXTURE Desc{};
//    CTexture* pTexture = {};
//   
//    Desc.szName = TextureName;
//   
//    _char szDefault[MAX_PATH];
//   
//    strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
//   
//    strcat_s(szDefault, Desc.szName);
//   
//    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);
//   
//    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
//        pTexture = CTexture::Create(m_pDevice, m_pContext, pFilePath, 1));
//   
//    Desc.pTexture = pTexture;
//    Safe_AddRef(pTexture);
//   
//    m_Textures.push_back(Desc);
//}

void CParticle_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
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
                PARTICLE_TEXTURE Desc{};
                CTexture* pTexture = {};

                // ?뺤옣???쒖쇅???뚯씪紐?
                _string strTextureTag = entry.path().stem().string();

                //?뚯씪紐낆쑝濡??띿뒪泥??대쫫 吏??
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //?뚯씪紐낆쑝濡??띿뒪泥?而댄룷?뚰듃 ?대쫫 吏??
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                //?뚯씪寃쎈줈 wstring 蹂??
                _wstring wstrFilePath = StringToWString(filePath);
   
                //?띿뒪泥?而댄룷?뚰듃 ?앹꽦
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                //?앹꽦???띿뒪泥?二쇱냼 ?깅줉, 誘몃━蹂닿린 ?꾩슱?ㅻ㈃ 二쇱냼濡?SRV媛?몄??쇳빐????ν빐以섏빞??
                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

void CParticle_Controller::Particle_Tab()
{
    if (m_bSelectedParticle)
    {
        if (ImGui::Begin("Particle Info"))
        {
       
                //?뚰떚???ㅼ젙媛?VIBuffer
                if (ImGui::CollapsingHeader("VIBuffer", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Loop", &(m_pSelectedVBDesc->IsLoop));

                    ImGui::Text("NumInstance");
                    ImGui::PushItemWidth(100);
                    ImGui::DragInt("##NumInstance", (int*)&(m_pSelectedVBDesc->iNumInstance));
                    ImGui::PopItemWidth();

                    ImGui::PushItemWidth(200);

                    ImGui::Text("SpreadWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##SpreadW", &(m_pSelectedVBDesc->fSpreadWeight), 0.1f ,0.f, 1.f);

                    ImGui::Text("DropWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##DorpW", &(m_pSelectedVBDesc->fDropWeight), 0.1f, 0.f, 1.f);

                    ImGui::Text("RotationWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##RotationW", &(m_pSelectedVBDesc->fRotationWeight), 0.1f, 0.f, 1.f);

                    ImGui::Text("Gravity");
                    ImGui::SameLine();
                    ImGui::DragFloat("##Gravity", &(m_pSelectedVBDesc->fGravity), 0.1f, 0.f, 9.8f);
     
                    ImGui::PopItemWidth();

                    ImGui::Text("Center");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##CenterX", &(m_pSelectedVBDesc->vCenter.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##CenterY", &(m_pSelectedVBDesc->vCenter.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##CenterZ", &(m_pSelectedVBDesc->vCenter.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Size");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##SizeX", &(m_pSelectedVBDesc->vSize.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##SizeY", &(m_pSelectedVBDesc->vSize.y));
                    ImGui::PopItemWidth();

                    ImGui::Text("Range");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RangeX", &(m_pSelectedVBDesc->vRange.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RangeY", &(m_pSelectedVBDesc->vRange.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RangeZ", &(m_pSelectedVBDesc->vRange.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Pivot");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##PivotX", &(m_pSelectedVBDesc->vPivot.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##PivotY", &(m_pSelectedVBDesc->vPivot.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##PivotZ", &(m_pSelectedVBDesc->vPivot.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Speed");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##SppedX", &(m_pSelectedVBDesc->vSpeed.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##SppedY", &(m_pSelectedVBDesc->vSpeed.y));
                    ImGui::PopItemWidth();

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##LifeTimeX", &(m_pSelectedVBDesc->vLifeTime.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##LifeTimeY", &(m_pSelectedVBDesc->vLifeTime.y));
                    ImGui::PopItemWidth();
                }

                if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen))
                {
              /*      ImGui::Checkbox("Spread", &(m_pSelectedParticleDesc->bSpread));
                    ImGui::Checkbox("Drop", &(m_pSelectedParticleDesc->bDrop));*/

                    ImGui::Text("Size");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticleSizeX", &(m_pSelectedParticleDesc->vSize.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleSizeY", &(m_pSelectedParticleDesc->vSize.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleSizeZ", &(m_pSelectedParticleDesc->vSize.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Position");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticlePosX", &(m_pSelectedParticleDesc->vPos.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticlePosY", &(m_pSelectedParticleDesc->vPos.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticlePosZ", &(m_pSelectedParticleDesc->vPos.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Color");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticleColorX", &(m_pSelectedParticleDesc->vColor.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleColorY", &(m_pSelectedParticleDesc->vColor.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleColorZ", &(m_pSelectedParticleDesc->vColor.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticleLifeTimeX", &(m_pSelectedParticleDesc->vLifeTime.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleLifeTimeY", &(m_pSelectedParticleDesc->vLifeTime.y));
                    ImGui::PopItemWidth();
                }

                //if (ImGui::Button("Apply"))
                //{
                //    //?붿뒪?щ┰?섎쭔 ??ν븯怨? 二쇱냼???곕줈 ??ν븷 ?꾩슂 ?놁쓣 嫄?媛숈쓬.
                //    //??ν븳 ?붿뒪?щ┰?섏쑝濡?媛??섏젙?댁꽌 ?ㅺ퀬 ?덇퀬, Apply 踰꾪듉 ?꾨Ⅴ硫??댁쟾??留뚮뱾?대넃? ?뚰떚???뚭눼 ?쒗궎怨? ?ㅼ떆 ?ъ깮??

                //    //吏湲??좏깮?섏뼱?덈뒗 ?뚰떚?댁쓽 ?쒓렇 ?꾩슂
                //    //吏湲??좏깮?섏뼱 ?덈뒗 ?뚰떚???쒓렇??Desc 2媛??꾩슂.
                //    //踰꾪띁 癒쇱? 留뚮뱾怨? ?뚰떚??留뚮뱾?댁빞??


                //    //?앹꽦?대넃? 踰꾪띁 ?먰삎 ??젣
                //    //Desc???덈뒗 ?뺣낫濡??덈줈??踰꾪띁 ?앹꽦

                //    //?뚰떚???대옒???대줎 ????Desc濡??대줎
                //    //?댁쟾???앹꽦???뚰떚?댁? 洹몃깷 鍮꾪솢?깊솕留??쒖폒以섎룄 ?좉굅 媛숈쓬.
                //    CParticle* pParticle = {};
                //    _wstring ParticleTag = {};

                //    _int iCheckIndex = 0;
                //    for (auto iter = m_Particles.begin(); iter != m_Particles.end();)
                //    {
                //        if (iCheckIndex == m_iSelectedParticle)
                //        {
                //            ParticleTag = iter->first;

                //            m_pSelectedParticleDesc->strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                //            m_pSelectedParticleDesc->strVIBufferTag = TEXT("Prototype_Componenet_VIBuffer_Instance_Point_");
                //            m_pSelectedParticleDesc->strVIBufferTag += ParticleTag;

                //            m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), m_pSelectedParticleDesc->strVIBufferTag);

                //            m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), m_pSelectedParticleDesc->strVIBufferTag,
                //                CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, m_pSelectedVBDesc));

                //            pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, m_pSelectedParticleDesc));
                //            m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Particle"), pParticle);

                //            m_pSelectedParticle->SetActivate(false);
                //            Safe_Release(m_pSelectedParticle);

                //            m_pSelectedParticle = pParticle;

                //            iter->second = pParticle;
                //            Safe_AddRef(pParticle);
                //            break;
                //        }
                //        else
                //        {
                //            ++iCheckIndex;
                //            ++iter;
                //        }
                //    }

                //}

            ImGui::End();
        }
    }

}

void CParticle_Controller::Particle_Base_Tab(CParticle::PARTICLE_DESC& tParticleDesc, _bool& IsCreate)
{
    if (ImGui::Begin("Particle Base"))
    {
        //?띿뒪泥??대?吏 ?ㅼ젙
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
                //湲곕낯踰좎씠?ㅻ줈 ?앹꽦
                _tchar ParticleTag[MAX_PATH] = {};
                CParticle::PARTICLE_DESC ParticleDesc{};
                CVIBuffer_Point_Instance::POINT_INSTANCE_DESC VIBufferDesc{};
                CParticle* pParticle;

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_ParticleTag, strlen(m_ParticleTag), ParticleTag, MAX_PATH);

                ParticleDesc.strMyTag = ParticleTag;
                ParticleDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                ParticleDesc.strVIBufferTag = TEXT("Prototype_Componenet_VIBuffer_Instance_Point_");
                ParticleDesc.strVIBufferTag += ParticleTag;
           
                //踰꾪띁 理쒖냼 ?ㅼ젙 媛?
                VIBufferDesc.iNumInstance = 1;
                VIBufferDesc.vSize = _float2(5.f, 5.f);

                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), ParticleDesc.strVIBufferTag,
                    CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &VIBufferDesc));

                m_tParticleDesc.emplace(ParticleTag, ParticleDesc);
                m_tVBDesc.emplace(ParticleTag, VIBufferDesc);

                tParticleDesc = ParticleDesc;

                //珥덇린??
                m_ParticleTag[0] = _T('\0');

                IsCreate = true;
                m_bTagFlag = false;
            }
        }
        ImGui::End();
    }
}

void CParticle_Controller::UpdateSelected_ParticleFormTag(_wstring ParticleTag)
{

    auto iterParticleDesc = m_tParticleDesc.find(ParticleTag);
    
    if (iterParticleDesc == m_tParticleDesc.end())
    {
        m_pSelectedParticleDesc = nullptr;
    }
    else
    {
        m_pSelectedParticleDesc = &iterParticleDesc->second;
    }

    auto iterVBDesc = m_tVBDesc.find(ParticleTag);

    if (iterVBDesc == m_tVBDesc.end())
    {
        m_pSelectedVBDesc = nullptr;
    }
    else
    {
        m_pSelectedVBDesc = &iterVBDesc->second;
    }

    if(m_pSelectedParticleDesc != nullptr)
        m_bSelectedParticle = true;
}

CParticle::PARTICLE_DESC* CParticle_Controller::Get_ParticleDesc(_wstring& ParticleTag)
{
    auto iter = m_tParticleDesc.find(ParticleTag);
    
    if (iter == m_tParticleDesc.end())
        return nullptr;

    return &iter->second;
}

CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* CParticle_Controller::Get_VBDesc(_wstring& VBTag)
{
    auto iter = m_tVBDesc.find(VBTag);

    if (iter == m_tVBDesc.end())
        return nullptr;

    return &iter->second;
}
void CParticle_Controller::Set_ParticleTag(const _char* szParticleTag)
{
    strcat_s(m_ParticleTag, szParticleTag);

    m_bTagFlag = true;
}
void CParticle_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterParticleDesc = m_tParticleDesc.find(DescTag);

    if (iterParticleDesc != m_tParticleDesc.end())
    {
        //?뱀떆 媛숈? ?대쫫?쇰줈 ?ㅼ떆留뚮뱾?댁??붽굅 ?鍮꾪빐??吏?뚯쨾?쇳븯?? ?꾩슂?놁쓣嫄?媛숈쑝硫?吏?뚮룄 ?좊벏.
        m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), iterParticleDesc->second.strVIBufferTag);

        m_tParticleDesc.erase(iterParticleDesc);
    }

    auto iterVBDesc = m_tVBDesc.find(DescTag);

    if (iterVBDesc != m_tVBDesc.end())
    {
        m_tVBDesc.erase(iterVBDesc);
    }

    //珥덇린??
    m_iSelectedParticle = 0;
    m_bSelectedParticle = false;
    m_pSelectedParticleDesc = nullptr;
    m_pSelectedVBDesc = nullptr;
}

//CParticle::PARTICLE_DESC CParticle_Controller::Find_Particle(_tchar ParticleTag)
//{
//    for (auto iter = m_tParticleDesc.begin(); iter != m_tParticleDesc.end(); )
//    {
//        if (iter->first == &ParticleTag)
//        {
//            return iter->second;
//        }
//        else
//            ++iter;
//    }
//}

CParticle_Controller* CParticle_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CParticle_Controller* pInstance = new CParticle_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CParticle_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CParticle_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    for (auto& pTexture : m_Textures)
        Safe_Release(pTexture.pTexture);
    m_Textures.clear();
}