#include "ClientPch.h"
#include "UI_HUD.h"
#include "Animator_UI.h"

// 얘는 오브젝트 매니저의 통제를 받음.
// 자식들은 얘의 통제를 받음. 삭제 포함.
CUI_HUD::CUI_HUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
{
}

CUI_HUD::CUI_HUD(const CUI_HUD& Prototype)
    :CCustom_UI(Prototype)
{
}

HRESULT CUI_HUD::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_HUD::Initialize_Clone(void* pArg)
{
    //__super::Initialize_Clone(pArg);

    CGameObject::Initialize_Clone(pArg);
    m_vecCachedUITransform.resize(1);
    Ready_Components(pArg);
    __super::Ready_Events();

    // Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
    _wstring strFilePath = 
        //L"../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json";
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD.json";
    Load_ChildObjects(strFilePath);

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {   // 로드할 애니메이션은 여기에 추가
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/HUD_HPBar_Effect.json"
    };
    Load_Animations(vecAnimFilePaths);


    //Find_ChildObject(L"UI_ParentTest")->Set_Active(false);

    return S_OK;
}

void CUI_HUD::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_HUD::Update(_float fTimeDelta)
{
    Update_Trigger(fTimeDelta);

    Update_CombinedMatrix();

    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_HUD::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_HUD::Render()
{
    //__super::Render();                      // Nothing. 렌더그룹 추가한 뒤 부터 렌더러에서 알아서 자식들까지 Render 돌림
}

HRESULT CUI_HUD::Load_ChildObjects(_wstring strFilePath)
{
    const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
    //const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

    // parse json
    ifstream file(strFilePath);
    json jUITreeData = {};
    if (file.is_open()) { file >> jUITreeData; }
    CUSTOM_UITREE_DESC tLoadTreeDesc = {};
    from_json(jUITreeData, tLoadTreeDesc);

    // load objects
    vector<CGameObject*> vecLoadObjects = {};
    for (auto& loadDesc : tLoadTreeDesc.vecUIInfoDescs)
    {
        UI_INFO_DESC tLoadUIInfoDesc = loadDesc;

        // Transform 값을 가져온 뒤, 행렬화하여 반영하고, (임시로) 자식 오브젝트로써 추가한다.
        _float3 vCurObjPos = tLoadUIInfoDesc.vPos;
        _float3 vCurObjRot = tLoadUIInfoDesc.vRot;
        _float3 vCurObjSca = tLoadUIInfoDesc.vSca;

        CGameObject* pCustomObj = nullptr;
        switch (tLoadUIInfoDesc.tUIDesc.iUIType)
        {
        case ENUM_CLASS(UI_TYPE::NONE):   pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));  break;
        case ENUM_CLASS(UI_TYPE::BUTTON): pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc)); break;
        default:            break;
        }
        m_vecChildObjects.push_back(static_cast<CCustom_UI*>(pCustomObj)); // 로컬에 저장.. 


        HIERARCHY_OBJ_DESC tObjDesc = { };
        tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
        tObjDesc.strObjName = tLoadUIInfoDesc.tUIDesc.strUIName;

        _matrix matScale = XMMatrixScaling(vCurObjSca.x, vCurObjSca.y, vCurObjSca.z);
        _matrix matRotX = XMMatrixRotationX(DegreesToRadians(vCurObjRot.x));
        _matrix matRotY = XMMatrixRotationY(DegreesToRadians(vCurObjRot.y));
        _matrix matRotZ = XMMatrixRotationZ(DegreesToRadians(vCurObjRot.z));
        _matrix matRot = matRotZ * matRotY * matRotX;
        _matrix matTrans = XMMatrixTranslation(vCurObjPos.x, vCurObjPos.y, vCurObjPos.z);

        _matrix matWorld = matScale * matRot * matTrans;
        static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);
    }
    
    // re-define childs of objects
    for (auto& child : m_vecChildObjects)
    {
        CUSTOM_UI_DESC tChildDesc = child->Get_UIDesc();
        for (auto& otherChild : m_vecChildObjects)
        {
            CUSTOM_UI_DESC tOtherChildDesc = otherChild->Get_UIDesc();

            for (auto& childName : tChildDesc.vecChildNames)
            {
                if (childName == tOtherChildDesc.strUIName)
                    child->Add_Child(otherChild);
            }
        }
    }

    // re-define childs of this(container)
    vector<CCustom_UI*> vecTrueChildObjects = {};
    for (auto& child : m_vecChildObjects)
    {
        if (child->Get_UIDesc().strParentName.empty())
            vecTrueChildObjects.push_back(child);
    }
    
    m_vecChildObjects = move(vecTrueChildObjects);

    return S_OK;
}

HRESULT CUI_HUD::Load_Animations(vector<_wstring> vecAnimFilePath)
{
    for (auto& animPath : vecAnimFilePath)
    {
        // parse json
        ifstream file(animPath);
        json jUIAnimData = {};
        if (file.is_open()) { file >> jUIAnimData; }
        CAnimator_UI::UI_ANIM_DESC tLoadAnimDesc = {};
        from_json(jUIAnimData, tLoadAnimDesc);

        CCustom_UI* pTargetObject = Find_ChildObject(tLoadAnimDesc.tUIDesc.strUIName);
        
        if (!pTargetObject)
            CRASH("Cannot find targetobject");
        CAnimator_UI* pTargetAnimator = dynamic_cast<CAnimator_UI*>(pTargetObject->Get_Component(L"Com_Animator_UI"));

        pTargetAnimator->Insert_Animation(tLoadAnimDesc);

        // ksta del : 테스트용
        pTargetAnimator->Change_Animation(L"TestHUDAnim3");
    }

    return S_OK;
}

HRESULT CUI_HUD::Ready_Components(void* pArg)
{
    return S_OK;
}

void CUI_HUD::Update_Trigger(_float fTimeDelta)
{
    // 키보드를 눌러서 쿨타임이 도는 것을 테스트함.

    // - 조건
    // 
    // 1, pass가 Variant (index : 5) 로 되어있어야 작동함.
    // 
    // 2. 아래 코드를 통해 쿨타임 정보가, Custom_UI 객체에서 셰이더로 전달 될 예정인, desc의 정보전달용 행렬 내의 [0][0]에 담음.
    //   이는 인스턴스별로 전달되어, 인스턴스별로 갱신이 이루어짐..
    // 
    // 3. 해당하는 Custom_UI 내의 Render 함수에서, 드로우콜 전에 iShaderFlag 를, 셰이더 전역변수로 지정해 주어야 함.
    //   이는 한 패스 내에서 여러 경우에 대응시키기 위해 준 플래그이며, Custom_UI가 들고있음.
    //   hlsl 내의 최상단에서 종류 확인 가능 (원형 쿨타임 UI인지, 사각형인지 등)


    enum HUD_CHAR_INDEX     { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };
    enum HUD_SKILL_INDEX    { SK_E, SK_R, SK_END };
    enum UIFLAG             { UIFLAG_ERR, UIFLAG_COOLDOWN_CIRCLE, UIFLAG_COOLDOWN_RECT, UIFLAG_END };

    static _uint    iSelectedCHIndex = 0;
    static _float   fSkillCD[CH_END][SK_END] = {};                                                  // left cooldown
    static _float   fChangeCD[CH_END] = {};                                                         // left cooldown

    const _float    fMaxSkillCD[CH_END][SK_END] = { {15.f, 20.f}, {15.f, 20.f}, {15.f, 20.f} };     // const cooldown
    const _float    fMaxChangeCD[CH_END] = { 2.f, 2.f, 2.f };                                       // const cooldown

    CCustom_UI* pSkillUI[CH_END] = {                  // Skill Indicator UI per Character.
        Find_ChildObject(L"Skill_Rover"),
        Find_ChildObject(L"Skill_Auguata"),
        Find_ChildObject(L"Skill_Galbrena")
    };

    CCustom_UI* pChangeUI[CH_END] = {                 // PartyFrame UI per Character.
        Find_ChildObject(L"Icon_Rover"),
        Find_ChildObject(L"Icon_Augusta"),
        Find_ChildObject(L"Icon_Galbrena")
    };


    for (auto& chCD : fSkillCD)                             // update cooldown
    {
        for (auto& cd : chCD)
        {
            cd -= fTimeDelta;
            if (cd <= 0) cd = 0;
        }
    }

    for (auto& cd : fChangeCD)
    {
        cd -= fTimeDelta;
        if (cd <= 0) cd = 0;
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_1) == KEYSTATE::DOWN)           // trigger cooldowns
    {
        if (fChangeCD[0] == 0)
        {
            iSelectedCHIndex = 0;
            fChangeCD[0] = fMaxChangeCD[0];
            pSkillUI[0]->Set_Active(true);
            pSkillUI[1]->Set_Active(false);
            pSkillUI[2]->Set_Active(false);
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_2) == KEYSTATE::DOWN)
    {
        if (fChangeCD[1] == 0)
        {
            iSelectedCHIndex = 1;
            fChangeCD[1] = fMaxChangeCD[1];
            pSkillUI[0]->Set_Active(false);
            pSkillUI[1]->Set_Active(true);
            pSkillUI[2]->Set_Active(false);
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_3) == KEYSTATE::DOWN)
    {
        if (fChangeCD[2] == 0)
        {
            iSelectedCHIndex = 2;
            fChangeCD[2] = fMaxChangeCD[2];
            pSkillUI[0]->Set_Active(false);
            pSkillUI[1]->Set_Active(false);
            pSkillUI[2]->Set_Active(true);
        }
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_E) == KEYSTATE::DOWN)
    {
        if (fSkillCD[iSelectedCHIndex][SK_E] == 0)
            fSkillCD[iSelectedCHIndex][SK_E] = fMaxSkillCD[iSelectedCHIndex][SK_E];
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
    {
        if (fSkillCD[iSelectedCHIndex][SK_R] == 0)
            fSkillCD[iSelectedCHIndex][SK_R] = fMaxSkillCD[iSelectedCHIndex][SK_R];
    }


    // UI별로 캐릭터 갯수만큼 존재.

    // 1. 스킬UI 에 방랑자 ER / 아우 ER / 갈브 ER 쿨타임 할당
    // 2. 교체UI 에 방랑자 / 아우 / 갈브 쿨타임 할당
    
    // skill
    for (_uint i = 0; i < CH_END; i++)                                  // Apply cooldown values
    {
        _float fCooldown_E = fSkillCD[i][SK_E];
        _float fCooldown_R = fSkillCD[i][SK_R];
        auto targetUI = pSkillUI[i];

        vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
        vecVariantMat[0].m[0][0] = fCooldown_E / fMaxSkillCD[i][SK_E];
        vecVariantMat[1].m[0][0] = fCooldown_R / fMaxSkillCD[i][SK_R];

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            UIFLAG_COOLDOWN_CIRCLE,
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);
    }

    // change
    for (_uint i = 0; i < CH_END; i++)
    {
        _float fCooldown = fChangeCD[i];
        auto targetUI = pChangeUI[i];

        vector<_float4x4> vecVariantMat = { _float4x4() };
        vecVariantMat[0].m[0][0] = 1.f - (fCooldown / fMaxChangeCD[i]);

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            UIFLAG_COOLDOWN_RECT,
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);
    }


    std::cout << "[UI_HUD][Update_Trigger] E : " << fSkillCD[iSelectedCHIndex][SK_E] << std::endl;
    std::cout << "[UI_HUD][Update_Trigger] R : " << fSkillCD[iSelectedCHIndex][SK_R] << std::endl;
}

CUI_HUD* CUI_HUD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_HUD* pInstance = new CUI_HUD(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_HUD");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_HUD::Clone(void* pArg)
{
    CUI_HUD* pInstance = new CUI_HUD(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_HUD");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_HUD::Free()
{
    for (auto& child : m_vecChildObjects)
        Safe_Release(child);

    __super::Free();
}
