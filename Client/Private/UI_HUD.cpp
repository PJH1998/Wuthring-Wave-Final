#include "ClientPch.h"
#include "UI_HUD.h"
#include "Animator_UI.h"

//#define KSTA_UI_COOLDOWNTEST
//#define KSTA_UI_HPBARTEST
//#define KSTA_UI_HPBARBOSSTEST
//#define KSTA_UI_ENERGYBARTEST


/**
*   테스트 방법
*   
*   U : 캐릭터 모드 전환 (방랑자는 서지모드 여부, 아우구스타는 궁모드 여부 등)
*   I : 공명 게이지 랜덤하게 UP
*   O : 보스 체력/아머 랜덤하게 DOWN
* 
*   (아우구스타)
*   J : 하단 검 자원 갯수 변경 (0 - 1 - 2개)
*   K : 하단 원형 자원 랜덤하게 UP
*   L : 궁극기 사용 후 칼로 바뀐 자원 랜덤하게 UP
* 
*   (갈브레나) (WIP)
*   Y : 에코 전용 공명게이지 10씩 UP (50이 최대치. 5번 쓰면 최대치로 찬다던데..)
* 
*   * 각 수치는 최상단의 매크로를 주석 해제하면 cout으로 보임
*/


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
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_1522.json";
    Load_ChildObjects(strFilePath);

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {   // 로드할 애니메이션은 여기에 추가
        //L"../../Client/Bin/Resource/UI/FJson/UIAnim/HUD_HPBar_Effect.json"

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
    Update_UI_SkillSection(fTimeDelta);
    Update_UI_PlayerHPBar(fTimeDelta);
    Update_UI_BossHPBar(fTimeDelta);
    Update_UI_KeyGuide(fTimeDelta);

    Update_UI_PlayerEnergyFrame(fTimeDelta);
    Update_UI_PlayerEnergyBar(fTimeDelta);

    Update_UI_PlayerEnergyBar_Augusta(fTimeDelta);
    Update_UI_PlayerEnergyBar_Galbrena(fTimeDelta);

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

void CUI_HUD::Update_UI_SkillSection(_float fTimeDelta)
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

    // - Variant 사용법
    // 
    // 1. 셰이더에서 Variant Pass 내 switch-case 문에 원하는 셰이더 제작
    // 
    // 2. 해당 효과를 사용할 UI에 CCustom_UI::VARIANTREADY_UI_DESC 만들어서
    //   flag 정보와 사용할 정보 matVariantValues 에 포함하여 던짐 (인스턴스별로 정보를 적용해야 하기에 vector 컨테이너 사용)
    //
    // 3. pass는 반드시 Variant 로, flag 및 요구 인스턴스 갯수 잘 지정해주기


    // 나중에 오른쪽에서부터 2~5개 내에서 유동적으로 변화 및 정렬되도록 하기
    // 아우구스타 같은 캐릭터는 아이콘이 2개로 줄고 그런다는 듯


    enum HUD_CHAR_INDEX     { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };


    // ksta : 나중에 플레이어 정보 통합되면 거기로부터 받아올 정보
                    m_iSelectedCHIndex;
    static _float   fSkillCD[CH_END][SK_END] = {};                                                  // left cooldown
    static _float   fChangeCD[CH_END] = {};                                                         // left cooldown

    const _float    fMaxSkillCD[CH_END][SK_END] = { {15.f, 20.f}, {15.f, 20.f}, {15.f, 20.f} };     // const cooldown
    const _float    fMaxChangeCD[CH_END] = { 2.f, 2.f, 2.f };                                       // const cooldown

    CCustom_UI* pSkillUI[CH_END] = {                  // Skill Indicator UI per Character.
        Find_ChildObject(L"Skill_Rover"),
        Find_ChildObject(L"Skill_Auguata"),
        Find_ChildObject(L"Skill_Galbrena")

        // echo..
    };

    CCustom_UI* pChangeUI[CH_END] = {                 // PartyFrame UI per Character.
        Find_ChildObject(L"Icon_Rover"),
        Find_ChildObject(L"Icon_Augusta"),
        Find_ChildObject(L"Icon_Galbrena")
    };

    static _bool    isFirstUpdate = true;               // Temp
    if (isFirstUpdate)
    {
        isFirstUpdate = false;
        pSkillUI[0]->Set_Active(true);
        pSkillUI[1]->Set_Active(false);
        pSkillUI[2]->Set_Active(false);
    }

    for (auto& chCD : fSkillCD)                         // update cooldown
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
    

    switch (m_iSelectedCHIndex)
    {
    case CH_ROVER:
        pSkillUI[0]->Set_Active(true);
        pSkillUI[1]->Set_Active(false);
        pSkillUI[2]->Set_Active(false);
        break;
    case CH_AUGUSTA:
        pSkillUI[0]->Set_Active(false);
        pSkillUI[1]->Set_Active(true);
        pSkillUI[2]->Set_Active(false);
        break;
    case CH_GALBRENA:
        pSkillUI[0]->Set_Active(false);
        pSkillUI[1]->Set_Active(false);
        pSkillUI[2]->Set_Active(true);
        break;
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_1) == KEYSTATE::DOWN)           // trigger cooldowns
    {
        if (fChangeCD[0] == 0 && m_iSelectedCHIndex != 0)
        {
            m_iSelectedCHIndex = 0;
            fChangeCD[0] = fMaxChangeCD[0];
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_2) == KEYSTATE::DOWN)
    {
        if (fChangeCD[1] == 0 && m_iSelectedCHIndex != 1)
        {
            m_iSelectedCHIndex = 1;
            fChangeCD[1] = fMaxChangeCD[1];
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_3) == KEYSTATE::DOWN)
    {
        if (fChangeCD[2] == 0 && m_iSelectedCHIndex != 2)
        {
            m_iSelectedCHIndex = 2;
            fChangeCD[2] = fMaxChangeCD[2];
        }
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_E) == KEYSTATE::DOWN)
    {
        if (fSkillCD[m_iSelectedCHIndex][SK_E] == 0)
            fSkillCD[m_iSelectedCHIndex][SK_E] = fMaxSkillCD[m_iSelectedCHIndex][SK_E];
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
    {
        if (fSkillCD[m_iSelectedCHIndex][SK_R] == 0)
            fSkillCD[m_iSelectedCHIndex][SK_R] = fMaxSkillCD[m_iSelectedCHIndex][SK_R];
    }


    // UI별로 캐릭터 갯수만큼 존재.

    // 1. 스킬UI 에 방랑자 ER / 아우 ER / 갈브 ER 쿨타임 할당
    // 2. 교체UI 에 방랑자 / 아우 / 갈브 쿨타임 할당
    
    // skill
    for (_uint i = 0; i < CH_END; i++)                                  // Apply cooldown values
    {
        auto targetUI = pSkillUI[i];

        vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };

        vecVariantMat[0].m[0][0] = fSkillCD[i][SK_E] / fMaxSkillCD[i][SK_E];
        vecVariantMat[1].m[0][0] = fSkillCD[i][SK_R] / fMaxSkillCD[i][SK_R];
        vecVariantMat[0].m[0][1] = 0.5f;
        vecVariantMat[1].m[0][1] = 0.5f;
        vecVariantMat[0].m[0][2] = 0.95f;
        vecVariantMat[1].m[0][2] = 0.95f;

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);



        if      (   i == CH_ROVER &&
                    m_iSelectedCHIndex == CH_ROVER)
        {
            auto roverUIDesc = pSkillUI[CH_ROVER]->Get_UIDesc();

            if (m_iEnergyBarMode == 1)
            {
                roverUIDesc.vecInstanceDescs[0].vSInstCoordX = { 0.07142857142f, 0.14285714285f };
            }
            else
            {
                roverUIDesc.vecInstanceDescs[0].vSInstCoordX = { 0.0f, 0.07142857142f };
            }
            pSkillUI[CH_ROVER]->Set_UIDesc(roverUIDesc);
        }

        else if (   i == CH_AUGUSTA &&
                    m_iSelectedCHIndex == CH_AUGUSTA)
        {
            auto augustaUIDesc = pSkillUI[CH_AUGUSTA]->Get_UIDesc();

            if (m_iEnergyBarMode == 1)
            {
                augustaUIDesc.vecInstanceDescs[0].vSInstCoordX = { 0.0f, 0.14285714285f };
                augustaUIDesc.vecInstanceDescs[0].vSInstCoordY = { 0.0f, 0.5f };
                augustaUIDesc.vecInstanceDescs[0].vSInstTrans.x = 750.f;

                augustaUIDesc.vecInstanceDescs[1].vSInstCoordX = { 0.0f, 0.1428571492433548f };
                augustaUIDesc.vecInstanceDescs[1].vSInstCoordY = { 0.5f, 1.0f };
             }
            else
            {
                augustaUIDesc.vecInstanceDescs[0].vSInstCoordX = { 0.7142857313156128f, 0.8571428656578064f };
                augustaUIDesc.vecInstanceDescs[0].vSInstCoordY = { 0.0f, 0.5f };
                augustaUIDesc.vecInstanceDescs[0].vSInstTrans.x = 650.f;

                augustaUIDesc.vecInstanceDescs[1].vSInstCoordX = { 0.1428571492433548f, 0.28571428571f };
                augustaUIDesc.vecInstanceDescs[1].vSInstCoordY = { 0.5f, 1.0f };
            }

            pSkillUI[CH_AUGUSTA]->Set_UIDesc(augustaUIDesc);
        }

        
    }

    // change
    for (_uint i = 0; i < CH_END; i++)
    {
        _float fCooldown = fChangeCD[i];
        auto targetUI = pChangeUI[i];

        vector<_float4x4> vecVariantMat = { _float4x4() };
        vecVariantMat[0].m[0][0] = 1.f - (fCooldown / fMaxChangeCD[i]);
        vecVariantMat[0].m[0][1] = 1.0f;
        vecVariantMat[0].m[0][2] = 0.8f;

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_RECT),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);
    }











    // ==============================
    // * Skill_BackgroundImage
    // =============================='


    // custom var
    static _uint iNumActiveBG = 4;
    _float4 vBGColor = _float4{.5f, .5f, .5f, .3f};

    CCustom_UI* pSkillBGUI = Find_ChildObject(L"Skill_BackgroundImage");    // 인스턴스 4개임

    vector<_float4x4> vecBGVariantMat = {};
    vecBGVariantMat.resize(5);

    for (_uint i = 0; i < iNumActiveBG; i++)
    {
        *reinterpret_cast<_float4*>(&vecBGVariantMat[i]._11) = vBGColor;
        *reinterpret_cast<_float*>(&vecBGVariantMat[i]._21) = (i < iNumActiveBG) ? static_cast<_float>(true) : static_cast<_float>(false);
    }

    CCustom_UI::VARIANTREADY_UI_DESC tBGVariantDesc = {
        vecBGVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLEMASK),
        true
    };

    pSkillBGUI->Set_VariantUIDesc(tBGVariantDesc);






#ifdef KSTA_UI_COOLDOWNTEST
    std::cout << "[UI_HUD][Update_UI_Cooldown] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 1 fChangeCD : " << fChangeCD[CH_ROVER] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 2 fChangeCD : " << fChangeCD[CH_AUGUSTA] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 3 fChangeCD : " << fChangeCD[CH_GALBRENA] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] E fSkillCD  : " << fSkillCD[iSelectedCHIndex][SK_E] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] R fSkillCD  : " << fSkillCD[iSelectedCHIndex][SK_R] << std::endl;
#endif // KSTA_UI_COOLDOWNTEST

}

void CUI_HUD::Update_UI_PlayerHPBar(_float fTimeDelta)
{
    // 플레이어의 HP 바를 갱신합니다.

    // 1. 뒤따라오는 체력바까지 생각하여 인스턴스는 2종으로 사용함.
    // 2. 색상은 셰이더를 통해, 원래 체력바와 뒤따라오는 체력바 2종을, 각각 2가지 색씩 사용하여 그라디언트되도록 구성

    // ksta : 나중에 플레이어 정보 통합되면 거기로부터 받아올 정보
    static _float fPlayerHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    static _float fPlayerBackHP[CH_END] = { fPlayerHP[0], fPlayerHP[1], fPlayerHP[2] };
    const _float fPlayerMaxHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;
    

    _float fPlayerHPRatio = fPlayerHP[m_iSelectedCHIndex] / fPlayerMaxHP[m_iSelectedCHIndex];
    static _float fPlayerHPBackRatio = fPlayerHPRatio;

    _float4 vHPColor        = { 1.f, 1.f, 1.f, 1.f };
    _float4 vHPBackColor    = { 1.f, 0.f, 0.f, 1.f };

    const _float fHPReduceTime = 0.5f;          // 줄어드는 소요시간은 0.5초정도?

    const auto targetUI = Find_ChildObject(L"Inst_HPBar");


    
    if (fHPReduceLeftTime > 0)
    {
        _float diff = fPlayerHPBackRatio - fPlayerHPRatio;              // 체력 비율 차이

        if (diff > 0.f)
        {
            _float delta = diff * (fTimeDelta / fHPReduceLeftTime);     // 줄어들 체력 비율

            fPlayerHPBackRatio -= delta;                               
            if (fPlayerHPBackRatio < fPlayerHPRatio)
                fPlayerHPBackRatio = fPlayerHPRatio;
        }

        fHPReduceLeftTime -= fTimeDelta;
        if (fHPReduceLeftTime < 0)
            fHPReduceLeftTime = 0;
    }
    else
    {
        fPlayerHPBackRatio = fPlayerHPRatio;
    }


    if (m_pGameInstance->Get_DIKeyState(DIK_P) == KEYSTATE::DOWN)       // [Test]
    {
        if (fPlayerHP[m_iSelectedCHIndex] == 0) fPlayerHP[m_iSelectedCHIndex] = fPlayerMaxHP[m_iSelectedCHIndex];
        isHit = true;
    }

    if (isHit == true)
    {

        _float fRandDamage = m_pGameInstance->Rand(100.f, 500.f);       // [Test] External Value

        // HP는 즉시 까임
        fPlayerHP[m_iSelectedCHIndex] -= fRandDamage;
        if (fPlayerHP[m_iSelectedCHIndex] < 0) fPlayerHP[m_iSelectedCHIndex] = 0;

        fHPReduceLeftTime = fHPReduceTime;
    }


    // change
    vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._11)    = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._11)      = vHPBackColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._21)    = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._21)      = vHPBackColor;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_NORMAL]._31)     = fPlayerHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_BACK]._31)       = fPlayerHPBackRatio;

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };
    
    targetUI->Set_VariantUIDesc(tVariantDesc);

    isHit = false;



#ifdef KSTA_UI_HPBARTEST
    std::cout << "[UI_HUD][Update_UI_HPBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_HPBar] 1 fPlayerHP     : " << fPlayerHPRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_HPBar] 2 fPlayerHPBack : " << fPlayerHPBackRatio << std::endl;
#endif // KSTA_UI_HPBARTEST

}

void CUI_HUD::Update_UI_BossHPBar(_float fTimeDelta)
{
    //if (pBoss == nullptr)
    //    return;
    


    // ksta : 나중에 보스 정보 통합되면 거기로부터 받아올 정보
    static _float fBossHP = { 10000.f };            // boss hitpoint
    static _float fBossBackHP = fBossBackHP;
    const _float fBossMaxHP = { 10000.f };
    
    static _float fBossSA = { 4000.f };             // boss superarmor
    static _float fBossBackSA = fBossSA;
    const _float fBossMaxSA = { 4000.f };
    static _bool isSABreak = false;




    static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;

    _float fBossHPRatio = fBossHP / fBossMaxHP;
    _float fBossSARatio = fBossSA / fBossMaxSA;
    static _float fBossHPBackRatio = fBossHPRatio;
    static _float fBossSABackRatio = fBossSARatio;

    const _float4 vHPColor1         = { 1.f, .7f, .1f, 1.f };
    const _float4 vHPColor2         = { 1.f, .2f, .0f, 1.f };
    const _float4 vHPBackColor1     = { .8f, .8f, .8f, 1.f };

    const _float4 vSAColor          = { 1.f, 1.f, 1.f, 1.f };   // before armor break
    const _float4 vSABreakColor     = { .9f, .8f, .3f, 1.f };
    const _float4 vSABackColor      = { 1.f, 1.f, 1.f, .3f };   // after armor break
    //const _float4 vSABreakBackColor = { .2f, .2f, .2f, 1.f };

    const _float fHPReduceTime = 0.5f;          // 줄어드는 소요시간은 0.5초정도?

    const auto targetUI = Find_ChildObject(L"Inst_BossHPBar");
    const auto targetSAUI = Find_ChildObject(L"Inst_BossSABar");


    if (fHPReduceLeftTime > 0)
    {
        _float fHPDiff = fBossHPBackRatio - fBossHPRatio;              // 체력 비율 차이
        _float fSADiff = fBossSABackRatio - fBossSARatio;              // 아머 비율 차이

        if (fHPDiff > 0.f)
        {
            _float fHPDelta = fHPDiff * (fTimeDelta / fHPReduceLeftTime);     // 줄어들 체력 비율

            fBossHPBackRatio -= fHPDelta;
            if (fBossHPBackRatio < fBossHPRatio)
                fBossHPBackRatio = fBossHPRatio;
        }
        if (fSADiff > 0.f)
        {
            _float fSADelta = fSADiff * (fTimeDelta / fHPReduceLeftTime);     // 줄어들 아머 비율

            fBossSABackRatio -= fSADelta;
            if (fBossSABackRatio < fBossSARatio)
                fBossSABackRatio = fBossSARatio;
        }

        fHPReduceLeftTime -= fTimeDelta;
        if (fHPReduceLeftTime < 0)
            fHPReduceLeftTime = 0;
    }
    else
    {
        fBossHPBackRatio = fBossHPRatio;
        fBossSABackRatio = fBossSARatio;
    }


    if (m_pGameInstance->Get_DIKeyState(DIK_O) == KEYSTATE::DOWN)       // [Test]
    {
        if (fBossHP == 0) fBossHP = fBossMaxHP;
        if (fBossSA == 0) fBossSA = fBossMaxSA;
        isHit = true;
    }

    if (isHit == true)
    {
        _float fRandDamage = m_pGameInstance->Rand(100.f, 500.f);       // [Test] External Value
        _float fRandSADamage = fRandDamage * 0.8f;

        // HP는 즉시 까임
        fBossHP -= fRandDamage;
        fBossSA -= fRandSADamage;

        if (fBossHP < 0) fBossHP = 0;
        if (fBossSA < 0) fBossSA = 0;

        fHPReduceLeftTime = fHPReduceTime;
    }

    // change
    vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._11)    = vHPColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._11)      = vHPBackColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._21)    = vHPColor2;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._21)      = vHPBackColor1;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._31)     = fBossHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_BACK]._31)       = fBossHPBackRatio;

    vector<_float4x4> vecVariantMatSA = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._11)  = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._11)    = vSABackColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._21)  = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._21)    = vSABackColor;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_NORMAL]._31)   = fBossSARatio;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_BACK]._31)     = fBossSABackRatio;


    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDescSA = {
        vecVariantMatSA,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };

    targetUI->Set_VariantUIDesc(tVariantDesc);
    targetSAUI->Set_VariantUIDesc(tVariantDescSA);

    isHit = false;



#ifdef KSTA_UI_HPBARBOSSTEST
    std::cout << "[UI_HUD][Update_UI_BossHPBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 1 fBossHP     : " << fBossHPRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 2 fBossHPBack : " << fBossHPBackRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 1 fBossSA     : " << fBossSARatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 2 fBossSABack : " << fBossSABackRatio << std::endl;
#endif // KSTA_UI_HPBARBOSSTEST



}

void CUI_HUD::Update_UI_KeyGuide(_float fTimeDelta)
{
    // 캐릭터에 따른 키 가이드 보이기 여부 분기
    CCustom_UI* pKeyButtonUI = Find_ChildObject(L"Inst_KeyButton");
    auto keyButtonDesc = pKeyButtonUI->Get_UIDesc();

    switch (m_iSelectedCHIndex)
    {
    case Client::CUI_HUD::CH_ROVER:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 0.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 1.0f };
        break;
    case Client::CUI_HUD::CH_AUGUSTA:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 1.0f };        
        break;
    case Client::CUI_HUD::CH_GALBRENA:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 0.0f }; 
        break;
    }
    pKeyButtonUI->Set_UIDesc(keyButtonDesc);
}

void CUI_HUD::Update_UI_PlayerEnergyFrame(_float fTimeDelta)
{
    switch (m_iSelectedCHIndex)
    {
    case Client::CUI_HUD::CH_ROVER:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(true);     // CH change visible
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(false);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(false);

        if      (m_iEnergyBarMode == 0)     
        {
            Find_ChildObject(L"Frame_Rover_Dark")->Set_Active(false);
            // Find_ChildObject(L"Frame_Rover")->Set_Active(true); // nullptr
        } 
        else if (m_iEnergyBarMode == 1)
        {
            Find_ChildObject(L"Frame_Rover_Dark")->Set_Active(true);
            // Find_ChildObject(L"Frame_Rover")->Set_Active(false); // nullptr
        }
                                    
        break;
    case Client::CUI_HUD::CH_AUGUSTA:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(false);
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(true);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(false);

        if      (m_iEnergyBarMode == 0)
        {
            Find_ChildObject(L"Frame_Augusta")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Augusta_OtherEnergy")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Augusta_UltMode")->Set_Active(false);
        }
        else if (m_iEnergyBarMode == 1)
        {
            Find_ChildObject(L"Frame_Augusta")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Augusta_OtherEnergy")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Augusta_UltMode")->Set_Active(true);
        }

        break;
    case Client::CUI_HUD::CH_GALBRENA:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(false);
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(false);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(true);

        if      (m_iEnergyBarMode == 0)
        {
            Find_ChildObject(L"Frame_Galbrena")->Set_Active(true);
            Find_ChildObject(L"Frame_Galbrena_Icon")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Galbrena_RageMode")->Set_Active(false);
        }
        else if (m_iEnergyBarMode == 1)
        {
            Find_ChildObject(L"Frame_Galbrena")->Set_Active(false);
            Find_ChildObject(L"Frame_Galbrena_Icon")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Galbrena_RageMode")->Set_Active(true);
        }

        break;
    }


    



    // 속성 아이콘 색상 적용
    vector<CCustom_UI*> pElementIcons = {
        Find_ChildObject(L"Icon_ElementDark"),
        Find_ChildObject(L"Icon_ElementThunder"),
        Find_ChildObject(L"Icon_ElementFire")
    };
    CCustom_UI* pElementTargetUI = pElementIcons[m_iSelectedCHIndex];
    const vector<_float4> vecElemColors = {
        {0.808f, 0.322f, 0.612f, 1.0f},         // Dark
        {0.969f, 0.451f, 1.0f, 1.0f},         // Thunder
        {1.0f, 0.416f, 0.416f, 1.0f}          // Fire
    };

    vector<_float4x4> vecElementVariantMat = { _float4x4() };
    *reinterpret_cast<_float4*>(&vecElementVariantMat[0]._11) = vecElemColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementVariantMat[0]._21) = static_cast<_float>(true);

    CCustom_UI::VARIANTREADY_UI_DESC tElementVariantDesc = {
        vecElementVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLEMASK),
        true
    };

    pElementTargetUI->Set_VariantUIDesc(tElementVariantDesc);

    
    
    // 속성 아이콘 주변 공명 값 적용
    const vector<_float4> vecElemCircleColors = {
        {0.808f, 0.322f, 0.612f, 1.0f},         // Dark
        {0.969f, 0.451f, 1.0f, 1.0f},         // Thunder
        {1.0f, 0.416f, 0.416f, 1.0f}          // Fire
    };
    static _float fElementAmounts[CH_END] = { 0.f, 0.f ,0.f };
    static _float fMaxElementAmounts[CH_END] = {100.f, 100.f, 100.f};

    // ksta : test 
    fElementAmounts[0] = (fElementAmounts[0] >= 100)? 0 : fElementAmounts[0] + 2.f  * 30.f * fTimeDelta;
    fElementAmounts[1] = (fElementAmounts[1] >= 100)? 0 : fElementAmounts[1] + 1.5f * 30.f * fTimeDelta;
    fElementAmounts[2] = (fElementAmounts[2] >= 100)? 0 : fElementAmounts[2] + 1.f  * 30.f * fTimeDelta;

    

    CCustom_UI* pElementGuageUI = Find_ChildObject(L"Icon_ElementGuage");
    //auto elementGuageDesc = pElementGuageUI->Get_UIDesc();

    vector<_float4x4> vecElementGuageVariantMat = { _float4x4() };
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._11) = (1.f - fElementAmounts[m_iSelectedCHIndex] / fMaxElementAmounts[m_iSelectedCHIndex]);
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._12) = 0.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._13) = 1.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._14) = static_cast<_bool>(true);
    *reinterpret_cast<_float4*>(&vecElementGuageVariantMat[0]._21) = vecElemCircleColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._31) = 90.f;



    CCustom_UI::VARIANTREADY_UI_DESC tElementAmountVariantDesc = {
        vecElementGuageVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
        true
    };

    pElementGuageUI->Set_VariantUIDesc(tElementAmountVariantDesc);

}

void CUI_HUD::Update_UI_PlayerEnergyBar(_float fTimeDelta)
{

    // ksta : 현재 공명회로 수치 나중에 받아올 것
    m_fPlayerEnergy;
    m_fPlayerMaxEnergy;

    _float fCurPlayerEnergy         = m_fPlayerEnergy[m_iSelectedCHIndex];
    _float fCurPlayerMaxEnergy      = m_fPlayerMaxEnergy[m_iSelectedCHIndex];
    
    static _float fGalbEchoEnergy   = 0.f;
    const _float fGalbMaxEchoEnergy = 50.f;

    _float fCurPlayerEnergyRatio    = fCurPlayerEnergy / fCurPlayerMaxEnergy;
    

    _float4     vSingleColor[2] = {};   // for Gradiant
    _float4     vExtraColor[2] = {};    // for Galbrena. 임마는 혼자 공명회로에 컬러 좌우로 두개씀
    _bool       isSingleVisible = {};   //
    //_float      fSingleHeight = {};   // vSpectrumHeights, vBackSpectrumHeights

    _float4     vBackColor[2] = {};
    _bool       isBackVisible = {};
    _float      fBackHeight = {};

    const auto targetUI = Find_ChildObject(L"Inst_EnergyItems");



    // 테스트용 입력을 통한 값 변경
    if (m_pGameInstance->Get_DIKeyState(DIK_I) == KEYSTATE::DOWN)
    {
        _float fRandEnergy = m_pGameInstance->Rand(10.f, 40.f);
        
        if (m_fPlayerEnergy[m_iSelectedCHIndex] == m_fPlayerMaxEnergy[m_iSelectedCHIndex])
            m_fPlayerEnergy[m_iSelectedCHIndex] = 0;
        else if (m_fPlayerEnergy[m_iSelectedCHIndex] < m_fPlayerMaxEnergy[m_iSelectedCHIndex])
            m_fPlayerEnergy[m_iSelectedCHIndex] += fRandEnergy;
        
        if (m_fPlayerEnergy[m_iSelectedCHIndex] > m_fPlayerMaxEnergy[m_iSelectedCHIndex])
            m_fPlayerEnergy[m_iSelectedCHIndex] = m_fPlayerMaxEnergy[m_iSelectedCHIndex];
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_U) == KEYSTATE::DOWN)
    {
        m_iEnergyBarMode++;
        if (m_iEnergyBarMode >= 2)
            m_iEnergyBarMode = 0;
    }

    // [Galbrena]
    if (m_pGameInstance->Get_DIKeyState(DIK_Y) == KEYSTATE::DOWN)
    {
        if (fGalbEchoEnergy == fGalbMaxEchoEnergy) fGalbEchoEnergy = 0;
        fGalbEchoEnergy += 10;
        if (fGalbEchoEnergy >= fGalbMaxEchoEnergy) fGalbEchoEnergy = fGalbMaxEchoEnergy;
    }



    enum HUD_PLAYER_ENERGYBAR { VALUE, TARGET, END };
    enum HUD_PLAYER_ENCOLOR {           // 공명게이지 색상 구분용 프리셋
        ENCL_ROVER_NORMAL, 
        ENCL_AUGUSTA_NORMAL,
        ENCL_AUGUSTA_ULT,
        ENCL_GALBRENA_NORMAL_L,
        ENCL_GALBRENA_NORMAL_R,
        ENCL_GALBRENA_ULT,

        ENCL_STATIC,
        ENCL_END
    };
    const _uint iNumSpectrums = 41;     // 스펙트럼의 각 요소 점 갯수는 최대 41개로 고정


    // 컬러 프리셋 목록
    vector<array<_float4, 2>> vecColorPreset;       // { start color (bottom), end color (top) }
    vecColorPreset.resize(ENCL_END);

    vecColorPreset[ENCL_ROVER_NORMAL]       = { _float4{0.961f, 0.192f, 0.502f, 1.f}, _float4{0.961f, 0.192f, 0.502f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_NORMAL]     = { _float4{0.769f, 0.631f, 0.933f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_ULT]        = { _float4{1.000f, 0.953f, 0.722f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_L]  = { _float4{0.894f, 0.573f, 0.525f, 1.f}, _float4{0.922f, 0.490f, 0.486f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_R]  = { _float4{0.682f, 0.769f, 0.980f, 1.f}, _float4{0.553f, 0.557f, 0.878f, .8f} };
    vecColorPreset[ENCL_GALBRENA_ULT]       = { _float4{0.482f, 0.412f, 0.878f, 1.f}, _float4{0.867f, 0.824f, 0.957f, .8f} };

    vecColorPreset[ENCL_STATIC]             = { _float4(1.f, .8f, .8f, .35f), _float4(1.f, .8f, .8f, .35f) }; // 안찼을 때 색상



    static vector<_float> vSpectrumHeights[END] = {};
    vSpectrumHeights[VALUE].resize(iNumSpectrums);
    vSpectrumHeights[TARGET].resize(iNumSpectrums);
    static vector<_float> vBackSpectrumHeights[END] = {};
    vBackSpectrumHeights[VALUE].resize(iNumSpectrums);
    vBackSpectrumHeights[TARGET].resize(iNumSpectrums);

    vector<_bool> vIsVisible = {};
    vIsVisible.resize(iNumSpectrums);
    vector<_bool> vIsVisibleStatic = {};
    vIsVisibleStatic.resize(iNumSpectrums);

    //vIsVisible.assign(vIsVisible.size(), true);

    // 추가로 뒤에서 가만히 있을 스펙트럼도 존재, 이는 현재 공명 게이지에 따라 단순히 마스킹만 될 것. 그러므로 값은 1.f 고정.
    // 즉, 인스턴스 41개를 가진 객체 3개를 그리거나, 정보를 합쳐서 한번에 41*3개를 그리거나 할 듯

    // 색상, 보일 부분 지정은 현재 선택중인 캐릭터 종류에 따라
    // ksta : 1) 갈브레나 색상 나누는건 아래 행렬 저장시에 분기로 나눌 것 (완)
    // ksta : 2) 게이지가 오르는 방향 또한 플레이어마다 다름. 이는 해당 분기에서 계산.
    // ksta : 3) 캐릭터에 따라 추가자원이 있는 경우도 있음.(아우구스타 중간원, 칼. 갈브 왼게이지)
    //          이는 전용 함수 따로 파서 거기서 계산.

    switch (m_iSelectedCHIndex)
    {
    case CH_ROVER:     
        {
            if      (m_iEnergyBarMode == 0)     /* Normal */  
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1];

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(vIsVisible.begin(), vIsVisible.end() - (41 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
            }
            else if (m_iEnergyBarMode == 1)     /* Ult    */  
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 16.f);      // applying player energy
                fill(vIsVisible.begin() + (16 - iVisibleBarRange), vIsVisible.end() - 25, true);
                fill(vIsVisible.end() - 16, vIsVisible.end() - (16 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
        }break;
    case CH_AUGUSTA:   
        {
            if      (m_iEnergyBarMode == 0 && fCurPlayerEnergyRatio != 1.f)     /* Normal */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_NORMAL][0];       // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_NORMAL][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(vIsVisible.begin(), 
                    (iVisibleBarRange > 16)? vIsVisible.begin() + 16 : vIsVisible.begin() + iVisibleBarRange, true);
                fill(vIsVisible.end() - 16,
                    vIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
            else if (m_iEnergyBarMode == 0 && fCurPlayerEnergyRatio == 1.f)     /* Ult?   */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_ULT][0];          // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_ULT][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(vIsVisible.begin(),
                    (iVisibleBarRange > 16) ? vIsVisible.begin() + 16 : vIsVisible.begin() + iVisibleBarRange, true);
                fill(vIsVisible.end() - 16,
                    vIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
            else if (m_iEnergyBarMode == 1)
            {
                fill(vIsVisibleStatic.begin(), vIsVisibleStatic.end(), false);
            }
        }break;
    case CH_GALBRENA:  
        {
            if      (m_iEnergyBarMode == 0)     /* Normal */  
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][0];   // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][1];  
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                fill(vIsVisible.begin() + 11, vIsVisible.end() - 26, false);    // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 26.f);      // applying player energy
                fill(vIsVisible.end() - 26, vIsVisible.end() - 26 + iVisibleBarRange, true);

                // [Galbrena] applying echo energy
                _uint iVisibleBarRange_Echo = static_cast<_uint>(fGalbEchoEnergy / fGalbMaxEchoEnergy * 11.f);
                fill(vIsVisible.begin() + 11 - iVisibleBarRange_Echo, vIsVisible.begin() + 11, true);


                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 11, vIsVisibleStatic.end() - 26, false);
            }
            else if (m_iEnergyBarMode == 1)     /* Ult    */  
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_ULT][0];        // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_ULT][1];       
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(vIsVisible.begin(), vIsVisible.begin() + iVisibleBarRange, true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
            }
        }break;
    }





    vBackColor[0] = vSingleColor[0];
    vBackColor[1] = vSingleColor[1];
    vBackColor[0].w *= 0.4f;
    vBackColor[1].w *= 0.4f;

    // 랜덤 수치 할당
    for (_uint i = 0; i < iNumSpectrums; ++i)
    {
        // 목표치에 천천히 접근 (부드럽게 변함)
        vSpectrumHeights[VALUE][i] += (vSpectrumHeights[TARGET][i] - vSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;
        vBackSpectrumHeights[VALUE][i] += (vBackSpectrumHeights[TARGET][i] - vBackSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;

        // 일정 확률로 새로운 목표로 변경
        if (m_pGameInstance->Rand(0.f, 100.f) < 3.f)
            vSpectrumHeights[TARGET][i] = m_pGameInstance->Rand(1.f, 2.f);
        if (m_pGameInstance->Rand(0.f, 100.f) < 3.f)
            vBackSpectrumHeights[TARGET][i] = m_pGameInstance->Rand(1.f, 4.f);
    }


    vector<_float4x4> vecVariantMat = {};
    vecVariantMat.resize(41);
    vector<_float4x4> vecVariantBackMat = {};
    vecVariantBackMat.resize(41);
    vector<_float4x4> vecVariantStaticMat = {};
    vecVariantStaticMat.resize(41);


    // 셰이더 전달용 행렬에 값 전달
    for (uint i = 0; i < vecVariantMat.size(); i++)         // front spectrum. 앞에서 색 입혀지고 움직이는 그것.
    {
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSingleColor[0];
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vSingleColor[1];
        vecVariantMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        vecVariantMat[i]._32 = vSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantBackMat.size(); i++)     // back spectrum.  뒤에서 알파 적용된 채로 움직이는 그것.
    {
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._11) = vBackColor[0];
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._21) = vBackColor[1];
        vecVariantBackMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        //vecVariantBackMat[i]._31 = false;5
        vecVariantBackMat[i]._32 = vBackSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantStaticMat.size(); i++)   // static spectrum. 앞에서 공명게이지 덜 찼을 떄 움직이지 않는 그것.
    {
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._11) = vecColorPreset[ENCL_STATIC][0];   // 기본값 색상 사용.
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._21) = vecColorPreset[ENCL_STATIC][1];   // 기본값 색상 사용.
        vecVariantStaticMat[i]._31 = static_cast<_float>(vIsVisibleStatic[i]);                  // 게이지가 차지 않아 그려지지 않는 부분만 그림.
        //vecVariantStaticMat[i]._31 = false;                                                     // 게이지가 차지 않아 그려지지 않는 부분만 그림.
        vecVariantStaticMat[i]._32 = 1.f;                                                       // 크기 1로 고정
    }

    // 갈브레나 예외처리 - 좌우 색상 변경
    if (m_iSelectedCHIndex == CH_GALBRENA)
    {
        if (m_iEnergyBarMode == 0)
        {
            for (uint i = 0; i < vecVariantMat.size(); i++)
            {
                if (i >= 15)
                {
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vExtraColor[0];
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vExtraColor[1];
                }
            }
            for (uint i = 0; i < vecVariantBackMat.size(); i++)
            {
                if (i >= 15)
                {
                    _float4 vExtraBackColor[2];
                    vExtraBackColor[0] = vExtraColor[0];   vExtraBackColor[0].w = 0.5f;
                    vExtraBackColor[1] = vExtraColor[1];   vExtraBackColor[1].w = 0.5f;
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vExtraBackColor[0];
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vExtraBackColor[1];
                }   
            }
        }
    }

    vecVariantBackMat.insert(vecVariantBackMat.end(),           // combine two vector. -> size = 41 + 41 = 82
        make_move_iterator(vecVariantMat.begin()),
        make_move_iterator(vecVariantMat.end()));
    vecVariantBackMat.insert(vecVariantBackMat.end(),           // combine two vector. -> size = 82 + 41 = 123
        make_move_iterator(vecVariantStaticMat.begin()),
        make_move_iterator(vecVariantStaticMat.end()));

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantBackMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_TRANSMIT),
        true
    };
    
    targetUI->Set_VariantUIDesc(tVariantDesc);  // 인스턴스 123개용 정보 한번에 그리게끔 보냄

#ifdef KSTA_UI_ENERGYBARTEST
    std::cout << "[UI_HUD][Update_UI_PlayerEnergyBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_PlayerEnergyBar] fPlayerEnergy : " << m_fPlayerEnergy[m_iSelectedCHIndex] << std::endl;
#endif // KSTA_UI_ENERGYBARTEST

}

void CUI_HUD::Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta)
{
    if (m_iSelectedCHIndex != CH_AUGUSTA)
        return;


    // 중앙 게이지, 칼날 두 개 존재
    // ksta : 나중에 받아와야 함

    static _uint iSwordEnergy = 0;            // mAX = 2

    static _float fPointEnergy = 0.f;
    const _float fMaxPointEnergy = 100.f;

    static _float fUltBladeEnergy = 0.f;
    const _float fMaxUltBladeEnergy = 100.;


    CCustom_UI* pBladeUI    = Find_ChildObject(L"Frame_Augusta_Inst_SwordEnergy");          // 인스턴스 0번이 왼쪽, 1번이 오른쪽 칼 자원.
    CCustom_UI* pPointUI    = Find_ChildObject(L"Frame_Augusta_Inst_CenterPointEnergy");    // 하단부 원형 자원

    CCustom_UI* pUltBladeUI = Find_ChildObject(L"Frame_Augusta_Inst_UltModeEnergy");        // 궁극기 중 사용하는 자원. 단일. 



    // 칼 자원
    if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
    {
        iSwordEnergy++;
        if (iSwordEnergy > 2) iSwordEnergy = 0;
    }

    // 원형 자원
    if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
    {
        if (fPointEnergy == 100) fPointEnergy = 0;
        else
        {
            fPointEnergy += m_pGameInstance->Rand(10.f, 40.f);
            if (fPointEnergy >= 100) fPointEnergy = 100;
        }
    }

    // 궁극기 자원
    if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
    {
        if (fUltBladeEnergy == 100) fUltBladeEnergy = 0;
        else
        {
            fUltBladeEnergy += m_pGameInstance->Rand(10.f, 40.f);
            if (fUltBladeEnergy >= 100) fUltBladeEnergy = 100;
        }
    }


    if (m_iEnergyBarMode == 0)
    {
        // 일반 UI
        pBladeUI    ->Set_Active(true);
        pPointUI    ->Set_Active(true);
        pUltBladeUI ->Set_Active(false);
    }

    else if (m_iEnergyBarMode == 1)
    {
        // 궁 UI
        pBladeUI    ->Set_Active(false);
        pPointUI    ->Set_Active(false);
        pUltBladeUI ->Set_Active(true);
    }




    auto bladeDesc = pBladeUI->Get_UIDesc();

    auto ultBladeDesc = pUltBladeUI->Get_UIDesc();
    
    // 칼 자원
    // 그냥 갯수에 따라 보일지 말지 정해주기. alpha pass 이용.
    switch (iSwordEnergy)
    {
    case 0:     
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 0.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
    break;
    case 1 :    
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
    break;
    case 2 :    
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
    break;
    }

    pBladeUI->Set_UIDesc(bladeDesc);
    



    // 원형 자원
    // 기존의 쿨타임용 셰이더 활용. vatiant pass 이용.

    // ! 반드시 pass 변경 필요
    vector<_float4x4> vecPointVariantMat = { _float4x4() };

    vecPointVariantMat[0].m[0][0] = 1 - fPointEnergy / fMaxPointEnergy;
    vecPointVariantMat[0].m[0][1] = 0.f;
    vecPointVariantMat[0].m[0][2] = 1.f;
    vecPointVariantMat[0].m[0][3] = static_cast<_float>(true);
    *reinterpret_cast<_float4*>(&vecPointVariantMat[0].m[1][0]) = _float4(1.0f, 0.941f, 0.729f, 1.f);

    CCustom_UI::VARIANTREADY_UI_DESC tPointVariantDesc = {
        vecPointVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
        true
    };
    pPointUI->Set_VariantUIDesc(tPointVariantDesc);




    // 궁극기 자원
    // 기존 셰이더에 만들어둔 것 활용. alpha pass 이용.
    _float ultRatio = fUltBladeEnergy / fMaxUltBladeEnergy;
    static _float fPreUltRatio = 0.f;

    if (ultRatio > fPreUltRatio) fPreUltRatio += fTimeDelta * 1.5f;
    if (ultRatio <= fPreUltRatio) fPreUltRatio = ultRatio;

    ultBladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, fPreUltRatio };

    pUltBladeUI->Set_UIDesc(ultBladeDesc);

}

void CUI_HUD::Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta)
{
    // 왼쪽 에코바는 Update_UI_PlayerEnergyBar 에서 처리
    // 
    // 강화상태일 시 하단 게이지 바 존재


    // 강화 상태 대응
    if (m_iSelectedCHIndex != CH_GALBRENA)
        return;

    if (m_iEnergyBarMode != 1)
        return;










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
