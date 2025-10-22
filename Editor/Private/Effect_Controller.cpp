#include "EditorPch.h"
#include "Effect_Controller.h"
#include "Effect_Prefab.h"
#include "Particle_Controller.h"
#include "Mesh_Controller.h"

CEffect_Controller::CEffect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CEffect_Controller::Initialize()
{
    m_pParticle_Controller = CParticle_Controller::Create(m_pDevice, m_pContext);
    m_pMesh_Controller = CMesh_Controller::Create(m_pDevice, m_pContext);

    return S_OK;
}

void CEffect_Controller::Update()
{
    Prefab_Tab();

}

void CEffect_Controller::Render()
{

}

void CEffect_Controller::Prefab_Tab()
{

    if (ImGui::Begin("Prefab_Info"))
    {
        if (ImGui::InputText("PrefabTag", m_PrefabTag, IM_ARRAYSIZE(m_PrefabTag), ImGuiInputTextFlags_EnterReturnsTrue))
            m_bTagFlag = true;

        if (m_bTagFlag)
        {
            if (ImGui::Button("Create Prefab"))
            {
                CEffect_Prefab::PREFAB_DESC pPrefabDesc = {};
                _tchar PrefabTag[MAX_PATH] = {};
                CEffect_Prefab* pPrefab = {};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_PrefabTag, strlen(m_PrefabTag), PrefabTag, MAX_PATH);

                pPrefabDesc.strPrefabTag = PrefabTag;

                pPrefab = static_cast<CEffect_Prefab*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Prefab"), PROTOTYPE::GAMEOBJECT, &pPrefabDesc));

                m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prefab"), pPrefab);

                m_Prefabs.emplace(PrefabTag, pPrefab);
                //Safe_AddRef(pPrefab); 

    
                m_bTagFlag = false;
                m_PrefabTag[0] = _T('\0');
            }
        }

        if (!m_Prefabs.empty())
        {
            vector<_string> strPrefabTag = {};
            vector<const _char*> szPrefabTag = {};

            //wstring -> string
            for (auto iter = m_Prefabs.begin(); iter != m_Prefabs.end(); ++iter)
            {
                _string strTag = WStringToString(iter->first);

                strPrefabTag.push_back(strTag);
            }

            //string -> char
            for (auto& strTag : strPrefabTag)
            {
                szPrefabTag.push_back(strTag.c_str());
            }

            if (ImGui::ListBox("PrefabTag", &m_iSelectedPrefab, szPrefabTag.data(), int(szPrefabTag.size()), int(szPrefabTag.size() + 2)))
            {
                UpdateSelected_PrefabFromIndex();
            }
            
            if (m_pSelectedPrefab != nullptr)
            {
                if (ImGui::Button("Delete Prefab"))
                {
                    auto iter = m_Prefabs.find(m_pSelectedPrefab->Get_MyTag());

                    if (iter != m_Prefabs.end())
                    {
                        iter->second->SetActivate(false);           
                        Safe_Release(iter->second);
                        m_Prefabs.erase(iter);

                        m_pSelectedPrefab = nullptr; 

                        Reset_TabInfo();
                    }
                }
            }

            if (m_pSelectedPrefab != nullptr)
            {
                
                if(ImGui::InputText("ChildrenTag", m_ChildrenTag, IM_ARRAYSIZE(m_ChildrenTag), ImGuiInputTextFlags_EnterReturnsTrue))
                    m_bChildrenTagFlag = true;

                if (m_bChildrenTagFlag)
                {
                   
                    if (ImGui::Button("Particle"))
                    {
                        m_pParticle_Controller->Set_ParticleTag(m_ChildrenTag);

                        m_eChildrenType = EFFECT_TYPE::PARTICLE;
                    }

                    if (ImGui::Button("Mesh"))
                    {
                        m_pMesh_Controller->Set_EffectMeshTag(m_ChildrenTag);
                            
                        m_eChildrenType = EFFECT_TYPE::MESH;
                    }


                    if (m_eChildrenType == EFFECT_TYPE::PARTICLE)
                    {
                        CParticle::PARTICLE_DESC pDesc = {};

                        m_pParticle_Controller->Particle_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc, m_eChildrenType);

                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                            m_eChildrenType == EFFECT_TYPE::END;
                        }
                    }

                    if (m_eChildrenType == EFFECT_TYPE::MESH)
                    {
                        CEffect_Mesh::EFFECTMESH_DESC pDesc = {};

                        m_pMesh_Controller->EffectMesh_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc, m_eChildrenType);

                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                            m_eChildrenType == EFFECT_TYPE::END;
                        }
                    }

                }

                if (m_pSelectedPrefab->Get_Children_Count() > 0)    
                {
                    vector<_string> strChildrenTag = {};
                    vector<const _char*> szChildrenTag = {};

                    for (size_t i = 0; i < m_pSelectedPrefab->Get_Children_Count(); i++)
                    { 
                        _string strTag = WStringToString(m_pSelectedPrefab->Get_Children_Tag(i));

                        strChildrenTag.push_back(strTag);
                    }

                    for (auto& strTag : strChildrenTag)
                        szChildrenTag.push_back(strTag.c_str());
                    
                 
                    if (ImGui::ListBox("Prefab Children", &m_iSelectedChildren, szChildrenTag.data(), int(szChildrenTag.size()), int(szChildrenTag.size() + 2)))
                    {
                        UpdateSelected_ChildrenFromIndex();
                    }

                    if (m_IsParticle)
                        m_pParticle_Controller->Update();

                    if (m_IsMeshEffect)
                        m_pMesh_Controller->Update();
                    
                    if (ImGui::Button("Apply"))
                    {
                       if (m_IsParticle)
                       {
                           CParticle::PARTICLE_DESC* pParticleDesc = m_pParticle_Controller->Get_ParticleDesc(m_strChildrenTag);
                           CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc = m_pParticle_Controller->Get_VBDesc(m_strChildrenTag);
                           
                           m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag);
                                                    
                           m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag,
                               CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, pVBDesc));

                           m_pSelectedPrefab->Remove_Children(pParticleDesc->strMyTag);

                           m_pSelectedPrefab->Add_Children(pParticleDesc, EFFECT_TYPE::PARTICLE);

                       }

                       if (m_IsMeshEffect)
                       {
                           CEffect_Mesh::EFFECTMESH_DESC* pEffectMeshDesc = m_pMesh_Controller->Get_EffectMeshDesc(m_strChildrenTag);
                           CVIBuffer_Mesh::MESH_FXINSTANCE_DESC* pFXVBDesc = m_pMesh_Controller->Get_VBMeshDesc(m_strChildrenTag);

                           m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), pEffectMeshDesc->strVIBufferTag);
               
                           _char szDatPath[MAX_PATH] = {};
                           strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin"); 
                           strcat_s(szDatPath, sizeof(szDatPath), pFXVBDesc->DatFilePath);  // DatFilePath -> "/Resource/.." 

                           _fmatrix DefaultMatrix = XMMatrixIdentity();
                           m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), pEffectMeshDesc->strVIBufferTag,
                               CVIBuffer_Mesh::Create(m_pDevice, m_pContext, szDatPath, DefaultMatrix, pFXVBDesc));

                            //테스트
                           m_pSelectedPrefab->Remove_Children(pEffectMeshDesc->strMyTag);

                           m_pSelectedPrefab->Add_Children(pEffectMeshDesc, EFFECT_TYPE::MESH);
                       }
                    }

                    if (m_IsParticle || m_IsMeshEffect || m_IsTrailMesh)
                    {
                        if (ImGui::Button("Delete Effect"))
                        {
                            if (m_IsParticle)
                            {
                                m_pSelectedPrefab->Remove_Children(m_strChildrenTag);

                                m_pParticle_Controller->Remove_Desc(m_strChildrenTag);

                                Reset_TabInfo();
                            }

                        }
                    }
                  
                }
         
            }
        }

        ImGui::End();
    }
}

void CEffect_Controller::UpdateSelected_PrefabFromIndex()
{
    if (m_pSelectedPrefab != nullptr)
        m_pSelectedPrefab->SetActivate(false);

    _int iCheckIndex = 0;

    for (auto iter = m_Prefabs.begin(); iter != m_Prefabs.end();)
    {
        if (iCheckIndex == m_iSelectedPrefab)
        {
            m_pSelectedPrefab = iter->second;
            iCheckIndex = 0;
            break;
        }
        else
        {
            ++iter;
            ++iCheckIndex;
        }
    }

    m_pSelectedPrefab->SetActivate(true);
}

void CEffect_Controller::UpdateSelected_ChildrenFromIndex()
{
    m_strChildrenTag =  m_pSelectedPrefab->Get_Children_Tag(m_iSelectedChildren);

    if (dynamic_cast<CParticle*>(m_pSelectedPrefab->Get_Children(m_strChildrenTag)))
    {
        m_IsParticle = true;
        m_IsMeshEffect = false;
        m_IsTrailMesh = false;

        m_pParticle_Controller->UpdateSelected_ParticleFormTag(m_strChildrenTag);
    }
    else if (dynamic_cast<CEffect_Mesh*>(m_pSelectedPrefab->Get_Children(m_strChildrenTag)))
    {
        m_IsParticle = false;
        m_IsMeshEffect = true;
        m_IsTrailMesh = false;

        m_pMesh_Controller->UpdateSelected_FXMeshFormTag(m_strChildrenTag);
    }

}

void CEffect_Controller::Reset_TabInfo()
{
    m_ChildrenTag[0] = _T('\0');
    m_bChildrenCreatFlag = false;
    m_bChildrenTagFlag = false;
    m_eChildrenType = EFFECT_TYPE::END;
    m_iSelectedChildren = 0;
    m_strChildrenTag = TEXT("");

    m_IsParticle = false;
    m_IsMeshEffect = false;
    m_IsTrailMesh = false;

}

CEffect_Controller* CEffect_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Controller* pInstance = new CEffect_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CEffect_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    Safe_Release(m_pParticle_Controller);
    Safe_Release(m_pMesh_Controller);

    for (auto& Prefab : m_Prefabs)
        Safe_Release(Prefab.second);
}
