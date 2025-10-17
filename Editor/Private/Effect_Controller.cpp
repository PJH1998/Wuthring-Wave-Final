#include "EditorPch.h"
#include "Effect_Controller.h"
#include "Effect_Prefab.h"
#include "Particle_Controller.h"

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
    // 프리팹 이름 지정해서 기본 베이스로 생성.
    // 생성한 프리팹 map에 저장해서 지정한 이름으로 접근.
    // 선택한 프리팹에 파티클 추가해서 자식으로 등록.
    // 등록된 자식은 설정할 수 있어야함. (파티클 컨트롤 이용해서 해야할거 같음)

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
                Safe_AddRef(pPrefab);

                //초기화
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
                //이번 프레임에 선택된 인덱스가 바뀌었으면 true 반환
                UpdateSelected_PrefabFromIndex();
            }
            
            if (m_pSelectedPrefab != nullptr)
            {
                if (ImGui::Button("Delete Prefab"))
                {
                    //프리팹의 자식들 삭제.
                    auto iter = m_Prefabs.find(m_pSelectedPrefab->Get_MyTag());

                    if (iter != m_Prefabs.end())
                    {
                        Safe_Release(iter->second);
                        m_Prefabs.erase(iter);

                        m_pSelectedPrefab = nullptr; 

                        Reset_TabInfo();
                    }
                }
            }
            //현재 선택되어있는 프리팹의 자식들 정보 띄우자
            if (m_pSelectedPrefab != nullptr)
            {
                //여기서 파티클 버튼, 매쉬버튼, 이런거 만들어서
                //눌러진 버튼에 따라 일단 기초베이스의 파티클 생성할 수 있게 해줘야하나 ?
                //1. 걍 눌러진 버튼에 따라 기본 베이스의 파티클 생성하고
                // -> 생성된 얘 있으면 어플라이 버튼 활성화 해서, 수정할 수 있게 해주기
                
                if(ImGui::InputText("ChildrenTag", m_ChildrenTag, IM_ARRAYSIZE(m_ChildrenTag), ImGuiInputTextFlags_EnterReturnsTrue))
                    m_bChildrenTagFlag = true;

                if (m_bChildrenTagFlag)
                {
                    if (ImGui::Button("Particle"))
                    {
                        //설정한 태그 버튼 타입에 맞는 컨트롤러에게 전달해서 자식 기본 베이스 창 띄우기
                        m_pParticle_Controller->Set_ParticleTag(m_ChildrenTag);

                        //현재 누른 타입
                        m_eChildrenType = EFFECT_TYPE::PARTICLE;
                    }

                    if (m_eChildrenType == EFFECT_TYPE::PARTICLE)
                    {
                        CParticle::PARTICLE_DESC pDesc = {};

                        m_pParticle_Controller->Particle_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc);

                            //초기화
                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                        }
                    }
                }

                // 프리팹이 자식이 있을 때 그 자식의 태그 가져와야할거 같음.
                // 가져와서 위와 동일하게 현재 자식 수 만큼 태그 뽑아서 백터에 담아줘야할거 같음.
                if (m_pSelectedPrefab->Get_Children_Count() > 0)    //자식이 있을 때
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
                    
                    //선택된 자식이 누구인지 알아야함. 그걸 알아야 파티클컨트롤러로 정보 수정할 수 있는 창 띄울 수 있을 듯.
                    //그리고 선택된 자식의 타입을 알아야할 듯 ex) 파티클, 매쉬, 트레일매쉬
                    if (ImGui::ListBox("Prefab Children", &m_iSelectedChildren, szChildrenTag.data(), int(szChildrenTag.size()), int(szChildrenTag.size() + 2)))
                    {
                        //몇번째 자식, 그 자식의 타입, 그자식의 태그가 필요함.
                        UpdateSelected_ChildrenFromIndex();
                    }

                    if (m_IsParticle)
                        m_pParticle_Controller->Update();
                    
                    //다른얘들이면 다른얘들 업데이트 돌려주자, 아직 파티클 컨트롤러 밖에 없음.
                    
                    //어플라이 버튼 하나로 어떤거냐에 따라 프리팹에 넘겨줄 정보 설정하는 방법이 없을까? 조금만 생각해보기.
                    if (ImGui::Button("Apply"))
                    {
                       if (m_IsParticle)
                       {
                           CParticle::PARTICLE_DESC* pParticleDesc = m_pParticle_Controller->Get_ParticleDesc(m_strChildrenTag);
                           CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc = m_pParticle_Controller->Get_VBDesc(m_strChildrenTag);
                           
                           //이전에 만들어져있던 버퍼 원형 삭제
                           m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag);
                                                    
                           //이전에 만들어져있던 버퍼 원형 이름과 같은 이름으로 수정된 VBDesc로 신규 생성
                           m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag,
                               CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, pVBDesc));

                           //현재 선택한 프리팹이 이전에 가지고 있던 자식 삭제
                           m_pSelectedPrefab->Remove_Children(pParticleDesc->strMyTag);

                           //이전에 가지고 있던 자식과 동일한 이름, 다른 Desc로 신규 자식 생성
                           m_pSelectedPrefab->Add_Children(pParticleDesc);

                       }
                    }

                    if (m_IsParticle || m_IsMeshEffect || m_IsTrailMesh)
                    {
                        if (ImGui::Button("Delete Effect"))
                        {
                            if (m_IsParticle)
                            {
                                //프리팹이 들고있는 자식 삭제
                                m_pSelectedPrefab->Remove_Children(m_strChildrenTag);

                                //파티클 컨트롤러가 가지고 있는 Desc 삭제
                                m_pParticle_Controller->Remove_Desc(m_strChildrenTag);

                                Reset_TabInfo();
                            }

                        }
                    }
                    
                    //어플라이 버튼 눌러서 해당 컨트롤러의 Desc 뽑아오고,
                    //뽑아온 Desc를 통해 프리팹에 차일드 추가.
                    //추가하기전에 이미 같은 이름의 차일드 있다면, 걔 삭제시키고 받아온 Desc로 다시 만들게 설정해주자.
                }
          
           

                //자식 이름은 여기서 설정해주고 컨트롤러에 넘겨서 그거 베이스로 만들어주자.
                //if (ImGui::InputText("ChildrenTag", )

                //입력된 태그값은 동일하게 가져가되, 버튼 3개 만들어서 파티클 만들지, 매쉬 만들지, 트레일 매쉬 만들지 알아야할 듯.
                // if (ImGui::Button("Particle"))
                // {
                //  // 여기서 bool 값으로 현재 선택되어있는 타입이 뭔지 확인해서 걔만 true로 활성화 해줘야할거 같음.
                // }

                //활성화 된 얘 체크해서 활성화된 이펙트의 컨트롤러 업데이트
                
                //활성화 된 bool 체크해서 Apply 눌렀을 때, 컨트롤러 체크해서 컨트롤러로 Desc가져와야함.
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
    //태그는 알 수 있는데 타입을 어떻게 알아야하는가 ?
    //프리팹에서 자식의 타입을 가져올 수 없는가 ?
    //일단 당장 생각나는 야매는 방식은 캐스팅 체크 밖에 없는데, 툴이니까 괜찮지 않을까?ㅎㅎ

    if (dynamic_cast<CParticle*>(m_pSelectedPrefab->Get_Children(m_strChildrenTag)))
    {
        m_IsParticle = true;
        m_IsMeshEffect = false;
        m_IsTrailMesh = false;

        //파티클 선택됐으니 현재 파티클 활성화 해줘야함.
        m_pParticle_Controller->UpdateSelected_ParticleFormTag(m_strChildrenTag);
    }
}

void CEffect_Controller::Reset_TabInfo()
{
    //혹시 이전에 프리팹의 자식 선택해놨으면 플래그 끄기
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

    for (auto& Prefab : m_Prefabs)
        Safe_Release(Prefab.second);
}
