#include "EditorPch.h"
#include "Effect_Controller.h"
#include "Effect_Prefab.h"
#include "Particle_Controller.h"
#include "Mesh_Controller.h"
#include "AnimationActor.h"

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
    m_pTrailMesh_Controller = CTrailMesh_Controller::Create(m_pDevice, m_pContext);
    m_pLoad_Controller = CLoad_Controller::Create(m_pDevice, m_pContext, LEVEL::EFFECT);

    return S_OK;
}

void CEffect_Controller::Update()
{
    Prefab_Tab();

    if (m_AnimActorDesc.pAnimActor != nullptr && m_pSelectedPrefab != nullptr)
        PrefabBinding_Tab();
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

        m_pLoad_Controller->Prefab_Load_Tab(&m_IsLoad);

        if (m_IsLoad)
        {
            Load_Prefab();
            m_IsLoad = false;
            m_pLoad_Controller->Reset_Load();
        }

        ImGui::SameLine(0.f, 30.f);

        if (m_bTagFlag)
        {
            if (ImGui::Button("Create Prefab"))
            {
                CEffect_Prefab::PREFAB_DESC pPrefabDesc = {};
                CEffect_Prefab::FRAME_DESC pPrefabFrameDesc = {};
                _tchar PrefabTag[MAX_PATH] = {};
                CEffect_Prefab* pPrefab = {};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_PrefabTag, strlen(m_PrefabTag), PrefabTag, MAX_PATH);

                pPrefabDesc.strPrefabTag = PrefabTag;

                pPrefab = static_cast<CEffect_Prefab*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Prefab"), PROTOTYPE::GAMEOBJECT, &pPrefabDesc));

                m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prefab"), pPrefab);

                //컨트롤 해주기 위해 Prefab 주소 저장
                m_Prefabs.emplace(PrefabTag, pPrefab);

                //Desc 저장
                pPrefabDesc.strPrefabTag = PrefabTag;

                m_PrefabDesc.emplace(PrefabTag, pPrefabDesc);
                //Safe_AddRef(pPrefab); 
    
                m_bTagFlag = false;
                m_PrefabTag[0] = _T('\0');
            }
        }

        ImGui::NewLine();

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
                    Remove_Prefab();
                }
            }


            if (m_pSelectedPrefab != nullptr)
            {
                
                if(ImGui::InputText("ChildrenTag", m_ChildrenTag, IM_ARRAYSIZE(m_ChildrenTag), ImGuiInputTextFlags_EnterReturnsTrue))
                    m_bChildrenTagFlag = true;

                if (m_bChildrenTagFlag)
                {
                   
                    // 생성 버튼
                    if (ImGui::Button("Particle"))
                    {
                        m_pParticle_Controller->Set_ParticleTag(m_ChildrenTag);

                        m_eChildrenType = EFFECT_TYPE::PARTICLE;
                    }
                    ImGui::SameLine(0.f, 20.f);

                    if (ImGui::Button("Mesh"))
                    {
                        m_pMesh_Controller->Set_EffectMeshTag(m_ChildrenTag);
                            
                        m_eChildrenType = EFFECT_TYPE::MESH;
                    }
                    ImGui::SameLine(0.f, 20.f);
                    if (ImGui::Button("Trail"))
                    {
                        m_pTrailMesh_Controller->Set_TrailMeshTag(m_ChildrenTag);

                        m_eChildrenType = EFFECT_TYPE::TRAIL;
                    }
                    //위에서 정해진 타입에 따라 컨트롤러 활성화
                    if (m_eChildrenType == EFFECT_TYPE::PARTICLE)
                    {
                        CParticle::PARTICLE_DESC pDesc = {};

                        m_pParticle_Controller->Particle_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc, m_eChildrenType);

                            //선택된 프리팹 Desc 저장, 프리팹 내부에서 자식 추가할 때 본인 Frame은 저장하고있음. 이 Desc는 추후 데이터 뽑기용.
                            CEffect_Prefab::FRAME_DESC Prefab_FrameDesc = {};
                            Prefab_FrameDesc.strChildrenTag = pDesc.strMyTag;
                            Prefab_FrameDesc.eChildrenType = EFFECT_TYPE::PARTICLE;
                            
                            m_pSelectedPrefabDesc->ChildrenCount += 1;
                            m_pSelectedPrefabDesc->FrameDesc.push_back(Prefab_FrameDesc);

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

                            CEffect_Prefab::FRAME_DESC Prefab_FrameDesc = {};
                            Prefab_FrameDesc.strChildrenTag = pDesc.strMyTag;
                            Prefab_FrameDesc.eChildrenType = EFFECT_TYPE::MESH;

                            m_pSelectedPrefabDesc->ChildrenCount += 1;
                            m_pSelectedPrefabDesc->FrameDesc.push_back(Prefab_FrameDesc);

                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                            m_eChildrenType == EFFECT_TYPE::END;
                        }
                    }

                    if (m_eChildrenType == EFFECT_TYPE::TRAIL)
                    {
                        CTrail_Mesh::TRAILMESH_DESC pDesc = {};

                        m_pTrailMesh_Controller->TrailMesh_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc, m_eChildrenType);

                            CEffect_Prefab::FRAME_DESC Prefab_FrameDesc = {};
                            Prefab_FrameDesc.strChildrenTag = pDesc.strMyTag;
                            Prefab_FrameDesc.eChildrenType = EFFECT_TYPE::TRAIL;

                            m_pSelectedPrefabDesc->ChildrenCount += 1;
                            m_pSelectedPrefabDesc->FrameDesc.push_back(Prefab_FrameDesc);

                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                            m_eChildrenType == EFFECT_TYPE::END;
                        }

                    }

                }

                if (ImGui::Button("Load Children"))
                {
                    IGFD::FileDialogConfig config;
                    config.path = "../../Client/Bin/Resource/Effect/Prefab";
                    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

                    ImGuiFileDialog::Instance()->OpenDialog("Load Effect", "Import", ".json", config);
                }
                Load_Children_To_Json();

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

                    if (m_IsTrailMesh)
                        m_pTrailMesh_Controller->Update();
                    
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
                           CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pFXVBDesc = m_pMesh_Controller->Get_VBMeshDesc(m_strChildrenTag);

                           m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), pEffectMeshDesc->strVIBufferTag);
               
                           _char szDatPath[MAX_PATH] = {};
                           strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin"); 
                           strcat_s(szDatPath, sizeof(szDatPath), pFXVBDesc->DatFilePath);  // DatFilePath -> "/Resource/.." 

                           _fmatrix DefaultMatrix = XMMatrixIdentity();
                           m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), pEffectMeshDesc->strVIBufferTag,
                               CVIBuffer_FXMesh_Instance::Create(m_pDevice, m_pContext, szDatPath, DefaultMatrix, pFXVBDesc));

                            //테스트
                           m_pSelectedPrefab->Remove_Children(pEffectMeshDesc->strMyTag);

                           m_pSelectedPrefab->Add_Children(pEffectMeshDesc, EFFECT_TYPE::MESH);
                       }

                       if (m_IsTrailMesh)
                       {
                           CTrail_Mesh::TRAILMESH_DESC* pTrailMeshDesc = m_pTrailMesh_Controller->Get_TrailMeshDesc(m_strChildrenTag);

                           m_pSelectedPrefab->Remove_Children(pTrailMeshDesc->strMyTag);

                           m_pSelectedPrefab->Add_Children(pTrailMeshDesc, EFFECT_TYPE::TRAIL);
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

                                Reset_ChildrenInfo();
                            }

                            if (m_IsMeshEffect)
                            {
                                m_pSelectedPrefab->Remove_Children(m_strChildrenTag);

                                m_pMesh_Controller->Remove_Desc(m_strChildrenTag);

                                Reset_ChildrenInfo();
                            }

                            if (m_IsTrailMesh)
                            {
                                m_pSelectedPrefab->Remove_Children(m_strChildrenTag);

                                m_pTrailMesh_Controller->Remove_Desc(m_strChildrenTag);

                                Reset_ChildrenInfo();
                            }
                        }
                    }
                  
                }
         
            }
        }

        Selected_Prefab_Info();

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

    //m_pSelectedPrefab->SetActivate(true);

    //현재 선택된 프리팹의 Desc
    auto iterDesc = m_PrefabDesc.find(m_pSelectedPrefab->Get_MyTag());

    if (iterDesc == m_PrefabDesc.end())
        return;

    m_pSelectedPrefabDesc = &iterDesc->second;

    Reset_ChildrenInfo();
}

void CEffect_Controller::UpdateSelected_ChildrenFromIndex()
{
    m_strChildrenTag = m_pSelectedPrefab->Get_Children_Tag(m_iSelectedChildren);

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
    else if (dynamic_cast<CTrail_Mesh*>(m_pSelectedPrefab->Get_Children(m_strChildrenTag)))
    {
        m_IsParticle = false;
        m_IsMeshEffect = false;
        m_IsTrailMesh = true;

        m_pTrailMesh_Controller->UpdateSelected_TrailMeshFormTag(m_strChildrenTag);
    }

    //프리팹이 들고 있는 구조체에서 자식과 동일한 프레임 찾기
    for (auto& Frame : m_pSelectedPrefabDesc->FrameDesc)
    {
        if (Frame.strChildrenTag == m_strChildrenTag)
            m_pSelectedPrefabFrame = &Frame;
    }
}

void CEffect_Controller::Selected_Prefab_Info()
{
    //선택 되어있는 프리팹에 자식들 설정값 넣어줘야하고, 저장하기 위한 Desc에도 넣어줘야할거 같음.
    //Desc 에 들어갈 정보, 자식의 태그 이름 -> 이건 자식들 생서할때, 프리팹과 Desc에 생성한 자식 이름 같이 넣어주자.
    //자식의 활성화 시점 -> 현재 선택한 프리팹의 자식이 있을 경우, 활성화 타임 지정할 수 있는 Info 띄우고 세팅해주면, 미리 넣어놓은 자식 태그로 Desc에저장, 프리팹애 던져주자.
    if (m_pSelectedPrefabDesc != nullptr && m_pSelectedPrefabFrame != nullptr)
    {
        ImGui::Text("ActivateTime");
        ImGui::PushItemWidth(60);
        ImGui::InputFloat("##ActivateTime", &(m_pSelectedPrefabFrame->fActivateTime));
        ImGui::PopItemWidth();

        if (ImGui::Button("Frame Apply"))
        {
            //여기서 현재 프리팹에 Frame 수정해줘야할거 같은데 
            m_pSelectedPrefab->Set_FrameDesc(m_pSelectedPrefabFrame);
        }
        ImGui::SameLine(0.f, 30.f);
        if (ImGui::Button("Reset Frame"))
        {
            m_pSelectedPrefab->Reset_Prefab_Info();
            m_pSelectedPrefab->SetActivate(true);
        }
    }

    if (m_pSelectedPrefabDesc != nullptr && m_pSelectedPrefab->Get_Children_Count() > 0)
    {
        if (ImGui::Button("Save Prefab"))
        {
            IGFD::FileDialogConfig config;

            config.path = "../../Client/Bin/Resource/Test/";
            config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

            ImGuiFileDialog::Instance()->OpenDialog("Save Prefab", "Export", ".json", config);
        }

        if (ImGuiFileDialog::Instance()->Display("Save Prefab", ImGuiWindowFlags_NoCollapse))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

                Prefab_To_Json(strFilePath);
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    ImGui::SameLine(0.f, 20.f);

    if (m_IsParticle || m_IsMeshEffect || m_IsTrailMesh)
    {
        Save_SelectedChildren_To_Json();
    }
}

void CEffect_Controller::Reset_ChildrenInfo()
{
    //자식들 정보 초기화
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

void CEffect_Controller::Reset_PrefabInfo()
{
    //프리팹 정보 초기화
    m_iSelectedPrefab = 0;
    m_bSelectedPrefab = false;
    m_pSelectedPrefab = nullptr;

    m_pSelectedPrefab = nullptr;
    m_pSelectedPrefabDesc = nullptr;
    m_pSelectedPrefabFrame = nullptr;
}

void CEffect_Controller::Remove_Prefab()
{
    //Desc 삭제
    auto PrefabDesc = m_PrefabDesc.find(m_pSelectedPrefab->Get_MyTag());

    if (PrefabDesc != m_PrefabDesc.end())
    {
        m_PrefabDesc.erase(PrefabDesc);
    }

    //프리팹 삭제
    auto Prefab = m_Prefabs.find(m_pSelectedPrefab->Get_MyTag());

    if (Prefab != m_Prefabs.end())
    {
        Prefab->second->SetActivate(false);
        Safe_Release(Prefab->second);
        m_Prefabs.erase(Prefab);
    }

    m_pSelectedPrefab = nullptr;
    Reset_PrefabInfo();
    Reset_ChildrenInfo();
}

void CEffect_Controller::Prefab_To_Json(const _string& strFilePath)
{
    //선택한 프리팹과 자식들까지 Json 으로 뽑기.

    auto Prefab = m_PrefabDesc.find(m_pSelectedPrefab->Get_MyTag());

    if (Prefab == m_PrefabDesc.end())
        return;

    ofstream jsonStream(strFilePath.c_str());

    json PrefabJson;

    PrefabJson["Prefab_Name"] = WStringToString(Prefab->second.strPrefabTag);
    PrefabJson["Children_Number"] = Prefab->second.ChildrenCount;
    
    //배열 저장할려면 array() 사용해야됨.
    json LifeTimeJson = json::array();
    LifeTimeJson.push_back(Prefab->second.vLifeTime.x);
    LifeTimeJson.push_back(Prefab->second.vLifeTime.y);

    PrefabJson["Prefab_LifeTime"] = LifeTimeJson;

    //Frame 배열 저장
    json FrameJson = json::array();

    for (size_t i = 0; i < Prefab->second.FrameDesc.size(); i++)
    {
        CEffect_Prefab::FRAME_DESC FrameDesc = Prefab->second.FrameDesc[i];

        json Frame;

        Frame["Children_Name"] = WStringToString(FrameDesc.strChildrenTag);
        Frame["Children_Type"] = FrameDesc.eChildrenType;
        Frame["Activate_Time"] = FrameDesc.fActivateTime;
        Frame["Activated"] = FrameDesc.bActivated;

        FrameJson.push_back(Frame);
    }

    PrefabJson["Frames"] = FrameJson;

    jsonStream << PrefabJson.dump(2);
    jsonStream.close();

    //현재 선택한 프리팹 파일 경로 + 파일 이름
    filesystem::path FilePath(strFilePath);

    //현재 선택한 프리팹 파일 경로만 뽑기
    filesystem::path FolderPath = FilePath.parent_path();

    // "../../Client/Bin/Resource/Effect/Prefab" , 이후 자식 타입에 따라 += Particle or Mesh or Trail 해주고자함.
    _string DefaultPath = FolderPath.string();

    for (size_t i = 0; i < Prefab->second.FrameDesc.size(); i++)
    {
        if (Prefab->second.FrameDesc[i].eChildrenType == EFFECT_TYPE::PARTICLE)
        {
            //파티클 버퍼 뽑기.
            _string ParticleVBPath = {};
            _string ParticlePath = {};
            CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc = {};

            pVBDesc = m_pParticle_Controller->Get_VBDesc(Prefab->second.FrameDesc[i].strChildrenTag);

            ParticleVBPath = DefaultPath;
            ParticleVBPath += "/ParticleVB/";
            ParticleVBPath += WStringToString(Prefab->second.FrameDesc[i].strChildrenTag);
            ParticleVBPath += ".json";
            ofstream josnVBStream(ParticleVBPath);

            json ParticleVBJson;

            Particle_VB_To_Json(ParticleVBJson, pVBDesc);

            josnVBStream << ParticleVBJson.dump(2);
            josnVBStream.close();

            
            //파티클 오브젝트 뽑기
            CParticle::PARTICLE_DESC* pParticleDesc = {};

            pParticleDesc = m_pParticle_Controller->Get_ParticleDesc(Prefab->second.FrameDesc[i].strChildrenTag);

            ParticlePath = DefaultPath;
            ParticlePath += "/Particle/";
            ParticlePath += WStringToString(Prefab->second.FrameDesc[i].strChildrenTag);
            ParticlePath += ".json";

            ofstream josnStream(ParticlePath);

            json ParticleJson;

            Particle_OB_To_Json(ParticleJson, pParticleDesc);
    
            josnStream << ParticleJson.dump(2);
            jsonStream.close();
        }
        if (Prefab->second.FrameDesc[i].eChildrenType == EFFECT_TYPE::MESH)
        {
            _string MeshVBPath = {};
            _string MeshPath = {};
            CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pVBDesc = {};
            CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc = {};

            //VB저장
            pVBDesc = m_pMesh_Controller->Get_VBMeshDesc(Prefab->second.FrameDesc[i].strChildrenTag);

            MeshVBPath = DefaultPath;
            MeshVBPath += "/MeshVB/";
            MeshVBPath += WStringToString(Prefab->second.FrameDesc[i].strChildrenTag);
            MeshVBPath += ".json";
           
            ofstream jsonVBStream(MeshVBPath);

            json MeshVBJson;

            Mesh_VB_To_Json(MeshVBJson, pVBDesc);

            jsonVBStream << MeshVBJson.dump(2);
            jsonVBStream.close();
 
            //매쉬오브젝트 저장
            pMeshDesc = m_pMesh_Controller->Get_EffectMeshDesc(Prefab->second.FrameDesc[i].strChildrenTag);

            MeshPath = DefaultPath;
            MeshPath += "/Mesh/";
            MeshPath += WStringToString(Prefab->second.FrameDesc[i].strChildrenTag);
            MeshPath += ".json";

            ofstream jsonOBStream(MeshPath);

            json MeshJson;

            Mesh_OB_To_Json(MeshJson, pMeshDesc);

            jsonOBStream << MeshJson.dump(2);
            jsonOBStream.close();
        }
        if (Prefab->second.FrameDesc[i].eChildrenType == EFFECT_TYPE::TRAIL)
        {
            _string TrailPath = {};
            CTrail_Mesh::TRAILMESH_DESC* pTrailDesc = {};

            //VB저장
            pTrailDesc = m_pTrailMesh_Controller->Get_TrailMeshDesc(Prefab->second.FrameDesc[i].strChildrenTag);

            TrailPath = DefaultPath;
            TrailPath += "/TrailMesh/";
            TrailPath += WStringToString(Prefab->second.FrameDesc[i].strChildrenTag);
            TrailPath += ".json";

            ofstream jsonStream(TrailPath);

            json TrailMeshJson;

            TrailMesh_To_Json(TrailMeshJson, pTrailDesc);

            jsonStream << TrailMeshJson.dump(2);
            jsonStream.close();
        }
    }
}

void CEffect_Controller::Particle_VB_To_Json(json& ParticleVBJson, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc)
{
    ParticleVBJson["NumInstance"] = pVBDesc->iNumInstance;

    json VBCenterJson = json::array();
    VBCenterJson.push_back(pVBDesc->vCenter.x);
    VBCenterJson.push_back(pVBDesc->vCenter.y);
    VBCenterJson.push_back(pVBDesc->vCenter.z);
    ParticleVBJson["Center"] = VBCenterJson;

    json VBRangeJson = json::array();
    VBRangeJson.push_back(pVBDesc->vRange.x);
    VBRangeJson.push_back(pVBDesc->vRange.y);
    VBRangeJson.push_back(pVBDesc->vRange.z);
    ParticleVBJson["Range"] = VBRangeJson;

    json VBSize = json::array();
    VBSize.push_back(pVBDesc->vSize.x);
    VBSize.push_back(pVBDesc->vSize.y);
    ParticleVBJson["Size"] = VBSize;

    //Pivot 저장
    json PivotJson = json::array();
    PivotJson.push_back(pVBDesc->vPivot.x);
    PivotJson.push_back(pVBDesc->vPivot.y);
    PivotJson.push_back(pVBDesc->vPivot.z);
    ParticleVBJson["Pivot"] = PivotJson;

    //Speed저장
    json SpeedJson = json::array();
    SpeedJson.push_back(pVBDesc->vSpeed.x);
    SpeedJson.push_back(pVBDesc->vSpeed.y);
    ParticleVBJson["Speed"] = SpeedJson;

    json VBLifeTime = json::array();
    VBLifeTime.push_back(pVBDesc->vLifeTime.x);
    VBLifeTime.push_back(pVBDesc->vLifeTime.y);
    ParticleVBJson["LifeTime"] = VBLifeTime;

    //Loop저장
    ParticleVBJson["Loop"] = pVBDesc->IsLoop;

    //스트레치 빌보드시 옵션 저장
    ParticleVBJson["Stretch"] = pVBDesc->IsStretch;

    ParticleVBJson["Stretch_Weight"] = pVBDesc->fStretchWeight;

    json StretchRang = json::array();
    StretchRang.push_back(pVBDesc->fStretchRange.x);
    StretchRang.push_back(pVBDesc->fStretchRange.y);
    ParticleVBJson["Stretch_Range"] = StretchRang;
    

    ParticleVBJson["Sprite"] = pVBDesc->IsSprite;

    ParticleVBJson["Sprite_Weight"] = pVBDesc->fSpriteWeight;
    ParticleVBJson["Sprite_DefulatSpeed"] = pVBDesc->fDefualtSpeed;
    

    ParticleVBJson["Delay"] = pVBDesc->IsDelay;

    json DelayTimeJson = json::array();
    DelayTimeJson.push_back(pVBDesc->fDelay.x);
    DelayTimeJson.push_back(pVBDesc->fDelay.y);
    ParticleVBJson["Delay_Time"] = DelayTimeJson;

    ParticleVBJson["SpreadWeight"] = pVBDesc->fSpreadWeight;
    ParticleVBJson["DropWeight"] = pVBDesc->fDropWeight;
    ParticleVBJson["RotationWeight"] = pVBDesc->fRotationWeight;
    ParticleVBJson["Gravity"] = pVBDesc->fGravity;
}

void CEffect_Controller::Particle_OB_To_Json(json& ParticleJson, CParticle::PARTICLE_DESC* pParticleDesc)
{
    ParticleJson["MyTag"] = WStringToString(pParticleDesc->strMyTag);
    ParticleJson["MyType"] = pParticleDesc->eMyType;
    ParticleJson["Root"] = pParticleDesc->IsRootOn;

    ParticleJson["TextureTag"] = WStringToString(pParticleDesc->strTextureTag);
    ParticleJson["VIBufferTag"] = WStringToString(pParticleDesc->strVIBufferTag);

    ParticleJson["ShaderPass"] = pParticleDesc->fShaderPass;


    json OBSizeJson = json::array();

    OBSizeJson.push_back(pParticleDesc->vSize.x);
    OBSizeJson.push_back(pParticleDesc->vSize.y);
    OBSizeJson.push_back(pParticleDesc->vSize.z);
    ParticleJson["Size"] = OBSizeJson;

    json PosJson = json::array();

    PosJson.push_back(pParticleDesc->vPos.x);
    PosJson.push_back(pParticleDesc->vPos.y);
    PosJson.push_back(pParticleDesc->vPos.z);
    ParticleJson["Position"] = PosJson;

    json ColorJson = json::array();

    ColorJson.push_back(pParticleDesc->vColor.x);
    ColorJson.push_back(pParticleDesc->vColor.y);
    ColorJson.push_back(pParticleDesc->vColor.z);
    ColorJson.push_back(pParticleDesc->vColor.w);
    ParticleJson["Color"] = ColorJson;

    json OBLifeTimeJson = json::array();

    OBLifeTimeJson.push_back(pParticleDesc->vLifeTime.x);
    OBLifeTimeJson.push_back(pParticleDesc->vLifeTime.y);
    ParticleJson["LifeTime"] = OBLifeTimeJson;

    ParticleJson["Sprite"] = pParticleDesc->IsSprite;
    ParticleJson["Row"] = pParticleDesc->iRows;
    ParticleJson["Col"] = pParticleDesc->iCols;

}

void CEffect_Controller::Mesh_VB_To_Json(json& MeshVBJson, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pVBDesc)
{
    MeshVBJson["NumInstance"] = pVBDesc->iNumInstance;

    json VBCenterJson = json::array();
    VBCenterJson.push_back(pVBDesc->vCenter.x);
    VBCenterJson.push_back(pVBDesc->vCenter.y);
    VBCenterJson.push_back(pVBDesc->vCenter.z);

    MeshVBJson["Center"] = VBCenterJson;

    json VBRangeJson = json::array();
    VBRangeJson.push_back(pVBDesc->vRange.x);
    VBRangeJson.push_back(pVBDesc->vRange.y);
    VBRangeJson.push_back(pVBDesc->vRange.z);

    MeshVBJson["Range"] = VBRangeJson;

    json VBSize = json::array();
    VBSize.push_back(pVBDesc->vSize.x);
    VBSize.push_back(pVBDesc->vSize.y);

    MeshVBJson["Size"] = VBSize;

    MeshVBJson["DatPath"] = string(pVBDesc->DatFilePath);

    //Pivot 저장
    json PivotJson = json::array();
    PivotJson.push_back(pVBDesc->vPivot.x);
    PivotJson.push_back(pVBDesc->vPivot.y);
    PivotJson.push_back(pVBDesc->vPivot.z);

    MeshVBJson["Pivot"] = PivotJson;

    //Speed저장
    json SpeedJson = json::array();
    SpeedJson.push_back(pVBDesc->vSpeed.x);
    SpeedJson.push_back(pVBDesc->vSpeed.y);

    MeshVBJson["Speed"] = SpeedJson;

    MeshVBJson["LifeTime"] = pVBDesc->fLifeTime;

    MeshVBJson["Loop"] = pVBDesc->IsLoop;

    MeshVBJson["SpawnBox"] = pVBDesc->IsSpawnBox;

    MeshVBJson["SpawnRing"] = pVBDesc->IsSpawnRing;

    MeshVBJson["RingAngle"] = pVBDesc->IsRingAngle;

    MeshVBJson["RingAngle_Min"] = pVBDesc->fRmin;
    MeshVBJson["RingAngle_Max"] = pVBDesc->fRmax;

    json DegreeJson = json::array();
    DegreeJson.push_back(pVBDesc->fDegreeAngle.x);
    DegreeJson.push_back(pVBDesc->fDegreeAngle.y);

    MeshVBJson["DegreeAngle"] = DegreeJson;

    MeshVBJson["InWard"] = pVBDesc->IsInWard;
    MeshVBJson["OutWard"] = pVBDesc->IsOutWard;

    MeshVBJson["Pitch"] = pVBDesc->fPitch;

    MeshVBJson["SpreadWeight"] = pVBDesc->fSpreadWeight;
    MeshVBJson["DropWeight"] = pVBDesc->fDropWeight;
    MeshVBJson["RotationWeight"] = pVBDesc->fRotationWeight;
}

void CEffect_Controller::Mesh_OB_To_Json(json& MeshJson, CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc)
{
    MeshJson["MyTag"] = WStringToString(pMeshDesc->strMyTag);
    MeshJson["MyType"] = pMeshDesc->eMyType;
    MeshJson["Root"] = pMeshDesc->IsRootOn;

    MeshJson["TextureTag"] = WStringToString(pMeshDesc->strTextureTag);
    MeshJson["VIBufferTag"] = WStringToString(pMeshDesc->strVIBufferTag);

    MeshJson["ShaderPass"] = pMeshDesc->fShaderPass;


    json OBSizeJson = json::array();

    OBSizeJson.push_back(pMeshDesc->vSize.x);
    OBSizeJson.push_back(pMeshDesc->vSize.y);
    OBSizeJson.push_back(pMeshDesc->vSize.z);
    MeshJson["Size"] = OBSizeJson;

    json PosJson = json::array();

    PosJson.push_back(pMeshDesc->vPos.x);
    PosJson.push_back(pMeshDesc->vPos.y);
    PosJson.push_back(pMeshDesc->vPos.z);
    MeshJson["Position"] = PosJson;

    json ColorJson = json::array();

    ColorJson.push_back(pMeshDesc->vColor.x);
    ColorJson.push_back(pMeshDesc->vColor.y);
    ColorJson.push_back(pMeshDesc->vColor.z);
    MeshJson["Color"] = ColorJson;

    json OBLifeTimeJson = json::array();

    OBLifeTimeJson.push_back(pMeshDesc->vLifeTime.x);
    OBLifeTimeJson.push_back(pMeshDesc->vLifeTime.y);
    MeshJson["LifeTime"] = OBLifeTimeJson;
}

void CEffect_Controller::TrailMesh_To_Json(json& TrailMesh, CTrail_Mesh::TRAILMESH_DESC* pTrailDesc)
{
    TrailMesh["MyTag"] = WStringToString(pTrailDesc->strMyTag);
    TrailMesh["MyType"] = pTrailDesc->eMyType;
    TrailMesh["Root"] = pTrailDesc->IsRootOn;

    TrailMesh["TextureTag"] = WStringToString(pTrailDesc->strTextureTag);
    TrailMesh["ColorTextureTag"] = WStringToString(pTrailDesc->strColorTextureTag);
    TrailMesh["VIBufferTag"] = WStringToString(pTrailDesc->strVIBufferTag);

    TrailMesh["ShaderPass"] = pTrailDesc->iShaderPass;

    TrailMesh["SweepSpeed"] = pTrailDesc->fSweep;
    TrailMesh["SweepWitdh"] = pTrailDesc->fSweepWitdh;

    TrailMesh["DirFlag"] = pTrailDesc->iDirFlag;

    json SizeJson = json::array();
    SizeJson.push_back(pTrailDesc->vSize.x);
    SizeJson.push_back(pTrailDesc->vSize.y);
    SizeJson.push_back(pTrailDesc->vSize.z);
    TrailMesh["Size"] = SizeJson;

    json PosJson = json::array();
    PosJson.push_back(pTrailDesc->vPos.x);
    PosJson.push_back(pTrailDesc->vPos.y);
    PosJson.push_back(pTrailDesc->vPos.z);
    TrailMesh["Position"] = PosJson;

    json LifeTimeJson = json::array();
    LifeTimeJson.push_back(pTrailDesc->vLifeTime.x);
    LifeTimeJson.push_back(pTrailDesc->vLifeTime.y);
    TrailMesh["LifeTime"] = LifeTimeJson;
}

void CEffect_Controller::Load_Prefab()
{
    CEffect_Prefab::PREFAB_DESC PrefabDesc = {};
    CEffect_Prefab* pPrefab = { nullptr };

    m_pLoad_Controller->Get_Prefab_Desc(PrefabDesc);

    //프리팹 Desc로 기본베이스 생성 후 아래 읽고 자식들 추가 해주는 작업 진행해줘야함.
   pPrefab = static_cast<CEffect_Prefab*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Prefab"),
        PROTOTYPE::GAMEOBJECT, &PrefabDesc));

   m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prefab"), pPrefab);

   m_Prefabs.emplace(PrefabDesc.strPrefabTag, pPrefab);
   m_PrefabDesc.emplace(PrefabDesc.strPrefabTag, PrefabDesc);

   m_pSelectedPrefab = pPrefab;

   auto iter = m_PrefabDesc.find(PrefabDesc.strPrefabTag);
   if (iter != m_PrefabDesc.end())
       m_pSelectedPrefabDesc = &iter->second;

    for (size_t i = 0; i < PrefabDesc.ChildrenCount; i++)
    {
        CEffect_Prefab::FRAME_DESC FrameDesc = {};

        FrameDesc = PrefabDesc.FrameDesc[i];

        if (FrameDesc.eChildrenType == EFFECT_TYPE::PARTICLE)
        {
            //VB, OB 둘다 처리
            Load_Particle(FrameDesc.strChildrenTag);
        }

        if (FrameDesc.eChildrenType == EFFECT_TYPE::MESH)
        {
            Load_FXMesh(FrameDesc.strChildrenTag);
        }

        if (FrameDesc.eChildrenType == EFFECT_TYPE::TRAIL)
        {
            Load_TrailMesh(FrameDesc.strChildrenTag);
        }
    }
}

void CEffect_Controller::Load_Particle(const _wstring& ParticleTag)
{
    CVIBuffer_Point_Instance::POINT_INSTANCE_DESC ParticleVBDesc = {};
    CParticle::PARTICLE_DESC ParticleDesc = {};
    _wstring Tag = ParticleTag;

    m_pLoad_Controller->Get_Particle_VB_Desc(Tag, ParticleVBDesc);
    m_pLoad_Controller->Get_Particle_OB_Desc(Tag, ParticleDesc);

    //VB 생성, Prefab 자식 추가
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), ParticleDesc.strVIBufferTag,
        CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &ParticleVBDesc));

    m_pSelectedPrefab->Add_Children(&ParticleDesc, EFFECT_TYPE::PARTICLE);

    //컨트롤러에 Desc 저장
    m_pParticle_Controller->Set_ParticleDesc(Tag, ParticleDesc);
    m_pParticle_Controller->Set_VBDesc(Tag, ParticleVBDesc);

}

void CEffect_Controller::Load_FXMesh(const _wstring& FXMeshTag)
{
    CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC FXMeshVBDesc = {};
    CEffect_Mesh::EFFECTMESH_DESC FXMeshDesc = {};
    _wstring Tag = FXMeshTag;

    m_pLoad_Controller->Get_FXMesh_VB_Desc(Tag, FXMeshVBDesc);
    m_pLoad_Controller->Get_FXMesh_OB_Desc(Tag, FXMeshDesc);

    _matrix PreTansformMatrix = XMMatrixIdentity();

    //VB생성
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), FXMeshDesc.strVIBufferTag,
        CVIBuffer_FXMesh_Instance::Create(m_pDevice, m_pContext, FXMeshVBDesc.DatFilePath, PreTansformMatrix, &FXMeshVBDesc));

    //Prefab 자식으로 추가
    m_pSelectedPrefab->Add_Children(&FXMeshDesc, EFFECT_TYPE::MESH);

    //컨트롤에 Desc 저장
    m_pMesh_Controller->Set_EffectMeshDesc(Tag, FXMeshDesc);
    m_pMesh_Controller->Set_MeshVBDesc(Tag, FXMeshVBDesc);
}

void CEffect_Controller::Load_TrailMesh(const _wstring& TrailMeshTag)
{
    CTrail_Mesh::TRAILMESH_DESC TrailDesc = {};
    _wstring Tag = TrailMeshTag;

    m_pLoad_Controller->Get_TrailMesh_Desc(Tag, TrailDesc);

    m_pSelectedPrefab->Add_Children(&TrailDesc, EFFECT_TYPE::TRAIL);

    m_pTrailMesh_Controller->Set_TrailMeshDesc(Tag, TrailDesc);
}

void CEffect_Controller::Save_SelectedChildren_To_Json()
{
    if (ImGui::Button("Save Children"))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/Effect/Prefab/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        ImGuiFileDialog::Instance()->OpenDialog("Save Effect", "Export", ".json", config);
    }
    if (ImGuiFileDialog::Instance()->Display("Save Effect", ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            _string strFilePath = {};
            _string strFolderPath = {};

            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");

            //마지막 문자열 빼고 폴더 경로만 가져오기. 
            if (lastSlashPos != string::npos) {
                strFolderPath += strFilePath.substr(0, lastSlashPos);
            }

            if (m_IsParticle)
            {
                //파티클 저장
                CParticle::PARTICLE_DESC* pParticleDesc = {};
                CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pParticleVBDesc = {};

                pParticleDesc = m_pParticle_Controller->Get_ParticleDesc(m_strChildrenTag);
                pParticleVBDesc = m_pParticle_Controller->Get_VBDesc(m_strChildrenTag);

                //VB저장
                _string ParticleVBPath = {};
                ParticleVBPath = strFolderPath;
                ParticleVBPath += "/ParticleVB/";
                ParticleVBPath += WStringToString(m_strChildrenTag);
                ParticleVBPath += ".json";

                ofstream VBjsonStream(ParticleVBPath);
                json ParticleVBJson;

                Particle_VB_To_Json(ParticleVBJson, pParticleVBDesc);

                VBjsonStream << ParticleVBJson.dump(2);
                VBjsonStream.close();

                //OB 저장
                _string ParticlePath = {};
                ParticlePath = strFolderPath;
                ParticlePath += "/Particle/";
                ParticlePath += WStringToString(m_strChildrenTag);
                ParticlePath += ".json";

                ofstream OBjsonStream(ParticlePath);
                json ParticleJson;

                Particle_OB_To_Json(ParticleJson, pParticleDesc);

                OBjsonStream << ParticleJson.dump(2);
                OBjsonStream.close();
            }
            else if (m_IsMeshEffect)
            {
                //매쉬 저장
                CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc = {};
                CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pMeshVBDesc = {};

                pMeshDesc = m_pMesh_Controller->Get_EffectMeshDesc(m_strChildrenTag);
                pMeshVBDesc = m_pMesh_Controller->Get_VBMeshDesc(m_strChildrenTag);

                //VB저장
                _string MeshVBPath = {};
                MeshVBPath = strFolderPath;
                MeshVBPath += "/MeshVB/";
                MeshVBPath += WStringToString(m_strChildrenTag);
                MeshVBPath += ".json";

                ofstream VBjsonStream(MeshVBPath);
                json MeshVBJson;

                Mesh_VB_To_Json(MeshVBJson, pMeshVBDesc);

                VBjsonStream << MeshVBJson.dump(2);
                VBjsonStream.close();

                //OB저장
                _string MeshPath = {};
                MeshPath = strFolderPath;
                MeshPath += "/Mesh/";
                MeshPath += WStringToString(m_strChildrenTag);
                MeshPath += ".json";

                ofstream OBjsonStream(MeshPath);
                json MeshJson;

                Mesh_OB_To_Json(MeshJson, pMeshDesc);

                OBjsonStream << MeshJson.dump(2);
                OBjsonStream.close();
            }
            else if (m_IsTrailMesh)
            {
                //트레일저장
                CTrail_Mesh::TRAILMESH_DESC* pTrailDesc = {};

                pTrailDesc = m_pTrailMesh_Controller->Get_TrailMeshDesc(m_strChildrenTag);

                _string TrailPath = {};
                TrailPath = strFolderPath;
                TrailPath += "/TrailMesh/";
                TrailPath += WStringToString(m_strChildrenTag);
                TrailPath += ".json";

                ofstream JsonStream(TrailPath);
                json TrailJson;

                TrailMesh_To_Json(TrailJson, pTrailDesc);

                JsonStream << TrailJson.dump(2);
                JsonStream.close();
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }
}

void CEffect_Controller::Load_Children_To_Json()
{
    if (ImGuiFileDialog::Instance()->Display("Load Effect", ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            _string strFilePath = {};
            _string strFolderPath = {};

            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");

            //마지막 문자열 빼고 폴더 경로만 가져오기. 
            if (lastSlashPos != string::npos) {
                strFolderPath += strFilePath.substr(0, lastSlashPos);
            }

            //폴더 경로 한번 더 빼줘야함.
            lastSlashPos = strFolderPath.find_last_of("\\");
            
            if (lastSlashPos != string::npos) {
                strFolderPath = strFolderPath.substr(0, lastSlashPos);
            }

            ifstream JsonStream(strFilePath.c_str());

            if (!JsonStream.is_open())
                return;

            json ChilderenJson;
            JsonStream >> ChilderenJson;
            JsonStream.close();

            //읽은 파일이 파티클, 매쉬, 트레일인지 먼저 체크해줘야함.
            _string strChildrenTag = {};
            EFFECT_TYPE eChildrenType = {};
            
            if (ChilderenJson.contains("MyTag"))
                strChildrenTag = ChilderenJson["MyTag"].get<_string>();

            if (ChilderenJson.contains("MyType"))
                eChildrenType = static_cast<EFFECT_TYPE>(ChilderenJson["MyType"].get<double>());

            //파티클 읽기.
            if (eChildrenType == EFFECT_TYPE::PARTICLE)
            {
                //VB 읽고 로더 저장함.
                _string ParticleVBPath = {};
                ParticleVBPath = strFolderPath;
                ParticleVBPath += "/ParticleVB/";
                ParticleVBPath += strChildrenTag;
                ParticleVBPath += ".json";

                _wstring ParticleVBTag = StringToWString(strChildrenTag);

                m_pLoad_Controller->Load_Particle_VB_FromJson(ParticleVBPath, ParticleVBTag);

                //OB 읽고 로더에 저장
                _string ParticleOBPath = {};
                ParticleOBPath = strFolderPath;
                ParticleOBPath += "/Particle/";
                ParticleOBPath += strChildrenTag;
                ParticleOBPath += ".json";

                _wstring ParticleOBTag = StringToWString(strChildrenTag);

                m_pLoad_Controller->Load_Particle_OB_FromJson(ParticleOBPath, ParticleOBTag);

                //내부에서 로더에 저장한 Desc읽고 컨트롤러에 저장, 프리팹에 자식으로 생성
                Load_Particle(ParticleOBTag);

                //로더에 저장된
                m_pLoad_Controller->Reset_Load();

                //프리팹 Desc 갱신 필요
                Load_Children_To_PrefabDesc(ParticleOBTag, eChildrenType);
            }
           
            //매쉬 읽기
            if (eChildrenType == EFFECT_TYPE::MESH)
            {
                //VB 읽고 로더 저장
                _string FXMeshVBPath = {};
                FXMeshVBPath = strFolderPath;
                FXMeshVBPath += "/MeshVB/";
                FXMeshVBPath += strChildrenTag;
                FXMeshVBPath += ".json";

                _wstring FXMeshVBTag = StringToWString(strChildrenTag);

                m_pLoad_Controller->Load_FXMesh_VB_FromJson(FXMeshVBPath, FXMeshVBTag);

                //OB 읽고 로더 저장
                _string FXMeeshOBPath = {};
                FXMeeshOBPath = strFolderPath;
                FXMeeshOBPath += "/Mesh/";
                FXMeeshOBPath += strChildrenTag;
                FXMeeshOBPath += ".json";

                _wstring FXMeshOBTag = StringToWString(strChildrenTag);

                m_pLoad_Controller->Load_FXMesh_OB_FromJson(FXMeeshOBPath, FXMeshOBTag);

                //로더에서 저장한 Desc 읽고 컨트롤러에 저장,
                Load_FXMesh(FXMeshOBTag);

                m_pLoad_Controller->Reset_Load();

                Load_Children_To_PrefabDesc(FXMeshOBTag, eChildrenType);
            }

            //트레일 읽기
            if (eChildrenType == EFFECT_TYPE::TRAIL)
            {
                _string TrailMesh = {};
                TrailMesh = strFolderPath;
                TrailMesh += "/TrailMesh/";
                TrailMesh += strChildrenTag;
                TrailMesh += ".json";

                _wstring TrailMeshTag = StringToWString(strChildrenTag);

                m_pLoad_Controller->Load_TrailMesh_FromJson(TrailMesh, TrailMeshTag);

                Load_TrailMesh(TrailMeshTag);

                m_pLoad_Controller->Reset_Load();

                Load_Children_To_PrefabDesc(TrailMeshTag, eChildrenType);
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

void CEffect_Controller::Load_Children_To_PrefabDesc(_wstring& ChildrenTag, EFFECT_TYPE eChildrenType)
{
    CEffect_Prefab::FRAME_DESC Desc = {};
    Desc.eChildrenType = eChildrenType;
    Desc.strChildrenTag = ChildrenTag;

    m_pSelectedPrefabDesc->ChildrenCount += 1;

    m_pSelectedPrefabDesc->FrameDesc.push_back(Desc);
}

void CEffect_Controller::Import_AnimationData(const EFFECTACTOR_DESC& effectActorDesc)
{
    if (nullptr == effectActorDesc.pAnimActor)
    {
        MSG_BOX("Anim Actor nullptr");
        return;
    }

    // 있으면 정보를 채워준다.
    m_AnimActorDesc.pAnimActor = effectActorDesc.pAnimActor;
    m_AnimActorDesc.pModelCom = effectActorDesc.pAnimActor->Get_ModelCom();
    m_AnimActorDesc.strAnimName = effectActorDesc.pAnimActor->Get_CurrentAnimationNames();
    m_AnimActorDesc.fDuration = effectActorDesc.fDuration;
}

void CEffect_Controller::PrefabBinding_Tab()
{
    if (ImGui::Begin("Prefab_Binding"))
    {
        if (ImGui::InputText("Bone Name", m_BoneName, IM_ARRAYSIZE(m_BoneName), ImGuiInputTextFlags_EnterReturnsTrue))
            m_bBoneFlag = true;
 
        if(ImGui::DragFloat("TrackPosition", &m_fTrackPosition, 0.1f, 0.f, m_AnimActorDesc.fDuration));


        if (m_bBoneFlag)
        {
            if (ImGui::Button("Binding"))
            {;
                _string BoneName = m_BoneName;

                m_AnimActorDesc.pBoneMatrix = m_AnimActorDesc.pAnimActor->Get_BoneMatrix(BoneName);

                m_bBoneFlag = false;
                m_BoneName[0] = _T('\0');
            }
        }

        if (ImGui::Button("Reset"))
        {
            //초기화
            m_AnimActorDesc.fDuration = 0.f;
            m_AnimActorDesc.pAnimActor = nullptr;
            m_AnimActorDesc.pModelCom = nullptr;
            m_AnimActorDesc.strAnimName = "";
            m_AnimActorDesc.pBoneMatrix = nullptr;
            m_bBoneFlag = false;
            m_BoneName[0] = _T('\0');

            m_pSelectedPrefab->SetActivate(false);
            m_pSelectedPrefab->Reset_SpawnMatrix();
        }
    
        ImGui::SameLine(0.f, 20.f);

        if (ImGui::Button("Action"))
            m_bTest = true;

        if (m_bTest)
        {
            if (m_fTrackPosition > -1.f)
            {
                _float fCurrentTrackPos = *m_AnimActorDesc.pAnimActor->Get_TrackPositionPtr(m_AnimActorDesc.strAnimName);

                if (m_fTrackPosition >= fCurrentTrackPos)
                {
                    _float4x4 BoneMatrix = *m_AnimActorDesc.pBoneMatrix;

                    m_pSelectedPrefab->Set_SpawnMatrix(BoneMatrix);
                    m_pSelectedPrefab->Reset_Prefab_Info();
                    m_bTest = false;
                }
            }
        }

        ImGui::End();
    }
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
    Safe_Release(m_pTrailMesh_Controller);
    Safe_Release(m_pLoad_Controller);

    for (auto& Prefab : m_Prefabs)
        Safe_Release(Prefab.second);
}
