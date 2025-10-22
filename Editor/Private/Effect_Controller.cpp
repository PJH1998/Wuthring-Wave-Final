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
    // ?꾨━???대쫫 吏?뺥빐??湲곕낯 踰좎씠?ㅻ줈 ?앹꽦.
    // ?앹꽦???꾨━??map????ν빐??吏?뺥븳 ?대쫫?쇰줈 ?묎렐.
    // ?좏깮???꾨━?뱀뿉 ?뚰떚??異붽??댁꽌 ?먯떇?쇰줈 ?깅줉.
    // ?깅줉???먯떇? ?ㅼ젙?????덉뼱?쇳븿. (?뚰떚??而⑦듃濡??댁슜?댁꽌 ?댁빞?좉굅 媛숈쓬)

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

                //珥덇린??
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
                //?대쾲 ?꾨젅?꾩뿉 ?좏깮???몃뜳?ㅺ? 諛붾뚯뿀?쇰㈃ true 諛섑솚
                UpdateSelected_PrefabFromIndex();
            }
            
            if (m_pSelectedPrefab != nullptr)
            {
                if (ImGui::Button("Delete Prefab"))
                {
                    //?꾨━?뱀쓽 ?먯떇????젣.
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
            //?꾩옱 ?좏깮?섏뼱?덈뒗 ?꾨━?뱀쓽 ?먯떇???뺣낫 ?꾩슦??
            if (m_pSelectedPrefab != nullptr)
            {
                //?ш린???뚰떚??踰꾪듉, 留ㅼ돩踰꾪듉, ?대윴嫄?留뚮뱾?댁꽌
                //?뚮윭吏?踰꾪듉???곕씪 ?쇰떒 湲곗큹踰좎씠?ㅼ쓽 ?뚰떚???앹꽦?????덇쾶 ?댁쨾?쇳븯???
                //1. 嫄??뚮윭吏?踰꾪듉???곕씪 湲곕낯 踰좎씠?ㅼ쓽 ?뚰떚???앹꽦?섍퀬
                // -> ?앹꽦?????덉쑝硫??댄뵆?쇱씠 踰꾪듉 ?쒖꽦???댁꽌, ?섏젙?????덇쾶 ?댁＜湲?
                
                if(ImGui::InputText("ChildrenTag", m_ChildrenTag, IM_ARRAYSIZE(m_ChildrenTag), ImGuiInputTextFlags_EnterReturnsTrue))
                    m_bChildrenTagFlag = true;

                if (m_bChildrenTagFlag)
                {
                    if (ImGui::Button("Particle"))
                    {
                        //?ㅼ젙???쒓렇 踰꾪듉 ??낆뿉 留욌뒗 而⑦듃濡ㅻ윭?먭쾶 ?꾨떖?댁꽌 ?먯떇 湲곕낯 踰좎씠??李??꾩슦湲?
                        m_pParticle_Controller->Set_ParticleTag(m_ChildrenTag);

                        //?꾩옱 ?꾨Ⅸ ???
                        m_eChildrenType = EFFECT_TYPE::PARTICLE;
                    }

                    if (m_eChildrenType == EFFECT_TYPE::PARTICLE)
                    {
                        CParticle::PARTICLE_DESC pDesc = {};

                        m_pParticle_Controller->Particle_Base_Tab(pDesc, m_bChildrenCreatFlag);

                        if (m_bChildrenCreatFlag)
                        {
                            m_pSelectedPrefab->Add_Children(&pDesc);

                            //珥덇린??
                            m_ChildrenTag[0] = _T('\0');
                            m_bChildrenCreatFlag = false;
                            m_bChildrenTagFlag = false;
                        }
                    }
                }

                // ?꾨━?뱀씠 ?먯떇???덉쓣 ??洹??먯떇???쒓렇 媛?몄??쇳븷嫄?媛숈쓬.
                // 媛?몄????꾩? ?숈씪?섍쾶 ?꾩옱 ?먯떇 ??留뚰겮 ?쒓렇 戮묒븘??諛깊꽣???댁븘以섏빞?좉굅 媛숈쓬.
                if (m_pSelectedPrefab->Get_Children_Count() > 0)    //?먯떇???덉쓣 ??
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
                    
                    //?좏깮???먯떇???꾧뎄?몄? ?뚯븘?쇳븿. 洹멸구 ?뚯븘???뚰떚?댁빻?몃·?щ줈 ?뺣낫 ?섏젙?????덈뒗 李??꾩슱 ???덉쓣 ??
                    //洹몃━怨??좏깮???먯떇????낆쓣 ?뚯븘?쇳븷 ??ex) ?뚰떚?? 留ㅼ돩, ?몃젅?쇰ℓ??
                    if (ImGui::ListBox("Prefab Children", &m_iSelectedChildren, szChildrenTag.data(), int(szChildrenTag.size()), int(szChildrenTag.size() + 2)))
                    {
                        //紐뉖쾲吏??먯떇, 洹??먯떇????? 洹몄옄?앹쓽 ?쒓렇媛 ?꾩슂??
                        UpdateSelected_ChildrenFromIndex();
                    }

                    if (m_IsParticle)
                        m_pParticle_Controller->Update();
                    
                    //?ㅻⅨ?섎뱾?대㈃ ?ㅻⅨ?섎뱾 ?낅뜲?댄듃 ?뚮젮二쇱옄, ?꾩쭅 ?뚰떚??而⑦듃濡ㅻ윭 諛뽰뿉 ?놁쓬.
                    
                    //?댄뵆?쇱씠 踰꾪듉 ?섎굹濡??대뼡嫄곕깘???곕씪 ?꾨━?뱀뿉 ?섍꺼以??뺣낫 ?ㅼ젙?섎뒗 諛⑸쾿???놁쓣源? 議곌툑留??앷컖?대낫湲?
                    if (ImGui::Button("Apply"))
                    {
                       if (m_IsParticle)
                       {
                           CParticle::PARTICLE_DESC* pParticleDesc = m_pParticle_Controller->Get_ParticleDesc(m_strChildrenTag);
                           CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc = m_pParticle_Controller->Get_VBDesc(m_strChildrenTag);
                           
                           //?댁쟾??留뚮뱾?댁졇?덈뜕 踰꾪띁 ?먰삎 ??젣
                           m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag);
                                                    
                           //?댁쟾??留뚮뱾?댁졇?덈뜕 踰꾪띁 ?먰삎 ?대쫫怨?媛숈? ?대쫫?쇰줈 ?섏젙??VBDesc濡??좉퇋 ?앹꽦
                           m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), pParticleDesc->strVIBufferTag,
                               CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, pVBDesc));

                           //?꾩옱 ?좏깮???꾨━?뱀씠 ?댁쟾??媛吏怨??덈뜕 ?먯떇 ??젣
                           m_pSelectedPrefab->Remove_Children(pParticleDesc->strMyTag);

                           //?댁쟾??媛吏怨??덈뜕 ?먯떇怨??숈씪???대쫫, ?ㅻⅨ Desc濡??좉퇋 ?먯떇 ?앹꽦
                           m_pSelectedPrefab->Add_Children(pParticleDesc);

                       }
                    }

                    if (m_IsParticle || m_IsMeshEffect || m_IsTrailMesh)
                    {
                        if (ImGui::Button("Delete Effect"))
                        {
                            if (m_IsParticle)
                            {
                                //?꾨━?뱀씠 ?ㅺ퀬?덈뒗 ?먯떇 ??젣
                                m_pSelectedPrefab->Remove_Children(m_strChildrenTag);

                                //?뚰떚??而⑦듃濡ㅻ윭媛 媛吏怨??덈뒗 Desc ??젣
                                m_pParticle_Controller->Remove_Desc(m_strChildrenTag);

                                Reset_TabInfo();
                            }

                        }
                    }
                    
                    //?댄뵆?쇱씠 踰꾪듉 ?뚮윭???대떦 而⑦듃濡ㅻ윭??Desc 戮묒븘?ㅺ퀬,
                    //戮묒븘??Desc瑜??듯빐 ?꾨━?뱀뿉 李⑥씪??異붽?.
                    //異붽??섍린?꾩뿉 ?대? 媛숈? ?대쫫??李⑥씪???덈떎硫? 嫄???젣?쒗궎怨?諛쏆븘??Desc濡??ㅼ떆 留뚮뱾寃??ㅼ젙?댁＜??
                }
          
           

                //?먯떇 ?대쫫? ?ш린???ㅼ젙?댁＜怨?而⑦듃濡ㅻ윭???섍꺼??洹멸굅 踰좎씠?ㅻ줈 留뚮뱾?댁＜??
                //if (ImGui::InputText("ChildrenTag", )

                //?낅젰???쒓렇媛믪? ?숈씪?섍쾶 媛?멸??? 踰꾪듉 3媛?留뚮뱾?댁꽌 ?뚰떚??留뚮뱾吏, 留ㅼ돩 留뚮뱾吏, ?몃젅??留ㅼ돩 留뚮뱾吏 ?뚯븘?쇳븷 ??
                // if (ImGui::Button("Particle"))
                // {
                //  // ?ш린??bool 媛믪쑝濡??꾩옱 ?좏깮?섏뼱?덈뒗 ??낆씠 萸붿? ?뺤씤?댁꽌 嫄붾쭔 true濡??쒖꽦???댁쨾?쇳븷嫄?媛숈쓬.
                // }

                //?쒖꽦??????泥댄겕?댁꽌 ?쒖꽦?붾맂 ?댄럺?몄쓽 而⑦듃濡ㅻ윭 ?낅뜲?댄듃
                
                //?쒖꽦????bool 泥댄겕?댁꽌 Apply ?뚮????? 而⑦듃濡ㅻ윭 泥댄겕?댁꽌 而⑦듃濡ㅻ윭濡?Desc媛?몄??쇳븿.
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
    //?쒓렇???????덈뒗????낆쓣 ?대뼸寃??뚯븘?쇳븯?붽? ?
    //?꾨━?뱀뿉???먯떇????낆쓣 媛?몄삱 ???녿뒗媛 ?
    //?쇰떒 ?뱀옣 ?앷컖?섎뒗 ?쇰ℓ??諛⑹떇? 罹먯뒪??泥댄겕 諛뽰뿉 ?녿뒗?? ?댁씠?덇퉴 愿쒖갖吏 ?딆쓣源??롢뀕

    if (dynamic_cast<CParticle*>(m_pSelectedPrefab->Get_Children(m_strChildrenTag)))
    {
        m_IsParticle = true;
        m_IsMeshEffect = false;
        m_IsTrailMesh = false;

        //?뚰떚???좏깮?먯쑝???꾩옱 ?뚰떚???쒖꽦???댁쨾?쇳븿.
        m_pParticle_Controller->UpdateSelected_ParticleFormTag(m_strChildrenTag);
    }
}

void CEffect_Controller::Reset_TabInfo()
{
    //?뱀떆 ?댁쟾???꾨━?뱀쓽 ?먯떇 ?좏깮?대넧?쇰㈃ ?뚮옒洹??꾧린
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
