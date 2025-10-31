#include "ClientPch.h"
#include "UI_HUD.h"
#include "Animator_UI.h"

//#define KSTA_UI_COOLDOWNTEST
//#define KSTA_UI_HPBARTEST
//#define KSTA_UI_HPBARBOSSTEST
//#define KSTA_UI_ENERGYBARTEST


/**
*   �׽�Ʈ ����
*   
*   U : ĳ���� ���� ��ȯ (�����ڴ� �������� ����, �ƿ챸��Ÿ�� �ø��� ���� ��)
*   I : ���� ������ �����ϰ� UP
*   O : ���� ü��/�Ƹ� �����ϰ� DOWN
* 
*   (�ƿ챸��Ÿ)
*   J : �ϴ� �� �ڿ� ���� ���� (0 - 1 - 2��)
*   K : �ϴ� ���� �ڿ� �����ϰ� UP
*   L : �ñر� ���� �� Į�� �ٲ� �ڿ� �����ϰ� UP
* 
*   (���극��)
*   Y : ���� ���� ���������� 10�� UP (50�� �ִ�ġ. 5�� ���� �ִ�ġ�� ���ٴ���..)
* 
*   * �� ��ġ�� �ֻ����� ��ũ�θ� �ּ� �����ϸ� cout���� ����
*/


// ��� ������Ʈ �Ŵ����� ������ ����.
// �ڽĵ��� ���� ������ ����. ���� ����.
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
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json";
    Load_ChildObjects(strFilePath);

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {   // �ε��� �ִϸ��̼��� ���⿡ �߰�
        //L"../../Client/Bin/Resource/UI/FJson/UIAnim/statustest.json"
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/PartyFrame_FadeOut.json"

    };
    Load_Animations(vecAnimFilePaths);

	//static_cast<CAnimator_UI*>(Find_ChildObject(L"EnergyBar")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0);
	static_cast<CAnimator_UI*>(Find_ChildObject(L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0);

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
    Update_UI_SkillSection_OnFeedback(fTimeDelta);
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
    //__super::Render();                      // Nothing. �����׷� �߰��� �� ���� ���������� �˾Ƽ� �ڽĵ���� Render ����
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

        // Transform ���� ������ ��, ���ȭ�Ͽ� �ݿ��ϰ�, (�ӽ÷�) �ڽ� ������Ʈ�ν� �߰��Ѵ�.
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
        m_vecChildObjects.push_back(static_cast<CCustom_UI*>(pCustomObj)); // ���ÿ� ����.. 


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

        // ksta del : �׽�Ʈ��
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
    // Ű���带 ������ ��Ÿ���� ���� ���� �׽�Ʈ��.

    // - ����
    // 
    // 1, pass�� Variant (index : 5) �� �Ǿ��־�� �۵���.
    // 
    // 2. �Ʒ� �ڵ带 ���� ��Ÿ�� ������, Custom_UI ��ü���� ���̴��� ���� �� ������, desc�� �������޿� ��� ���� [0][0]�� ����.
    //   �̴� �ν��Ͻ����� ���޵Ǿ�, �ν��Ͻ����� ������ �̷����..
    // 
    // 3. �ش��ϴ� Custom_UI ���� Render �Լ�����, ��ο��� ���� iShaderFlag ��, ���̴� ���������� ������ �־�� ��.
    //   �̴� �� �н� ������ ���� ��쿡 ������Ű�� ���� �� �÷����̸�, Custom_UI�� �������.
    //   hlsl ���� �ֻ�ܿ��� ���� Ȯ�� ���� (���� ��Ÿ�� UI����, �簢������ ��)

    // - Variant ����
    // 
    // 1. ���̴����� Variant Pass �� switch-case ���� ���ϴ� ���̴� ����
    // 
    // 2. �ش� ȿ���� ����� UI�� CCustom_UI::VARIANTREADY_UI_DESC ����
    //   flag ������ ����� ���� matVariantValues �� �����Ͽ� ���� (�ν��Ͻ����� ������ �����ؾ� �ϱ⿡ vector �����̳� ���)
    //
    // 3. pass�� �ݵ�� Variant ��, flag �� �䱸 �ν��Ͻ� ���� �� �������ֱ�


    // ���߿� �����ʿ������� 2~5�� ������ ���������� ��ȭ �� ���ĵǵ��� �ϱ�
    // �ƿ챸��Ÿ ���� ĳ���ʹ� �������� 2���� �ٰ� �׷��ٴ� ��


    enum HUD_CHAR_INDEX     { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };


    // ksta : ���߿� �÷��̾� ���� ���յǸ� �ű�κ��� �޾ƿ� ����
                    m_iSelectedCHIndex;
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
        {
            fSkillCD[m_iSelectedCHIndex][SK_E] = fMaxSkillCD[m_iSelectedCHIndex][SK_E];
            if (m_iSelectedCHIndex == CH_AUGUSTA &&
                m_iEnergyBarMode == 1)
                Add_UI_SkillSection_OnFeedback(1);
            else
                Add_UI_SkillSection_OnFeedback(2);
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
    {
        if (fSkillCD[m_iSelectedCHIndex][SK_R] == 0)
        {
            fSkillCD[m_iSelectedCHIndex][SK_R] = fMaxSkillCD[m_iSelectedCHIndex][SK_R];
            Add_UI_SkillSection_OnFeedback(0);
        }
    }


    // UI���� ĳ���� ������ŭ ����.

    // 1. ��ųUI �� ����� ER / �ƿ� ER / ���� ER ��Ÿ�� �Ҵ�
    // 2. ��üUI �� ����� / �ƿ� / ���� ��Ÿ�� �Ҵ�
    
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

    CCustom_UI* pSkillBGUI = Find_ChildObject(L"Skill_BackgroundImage");    // �ν��Ͻ� 4����

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

void CUI_HUD::Update_UI_SkillSection_OnFeedback(_float fTimeDelta)
{
    // Feedback ��ü�� �ð��� ���� �����ϴ� �Լ�.
    // (�ð��� ���� �� �����Ͽ� �ν��Ͻ����� �Ѱ��ָ�,
    // ������ �ð��� ������ �ش� ���� ���Ҹ� �����Ѵ�.)

    // ������ ���⼭ ������

    CCustom_UI* pFeedbackUI = Find_ChildObject(L"Skill_OnFeedback");    
    const _float2 fDestScale = { 1.2f, 1.2f };
    const _float fStartAlpha = 0.f;     // 0�� ����, 1�� �Ⱥ������� ����.
    const _float fLifeTime = .5f;
    _float4 vColor = { 0.f, 0.f, 0.f, 1.f };


    auto uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;



    // ���ο� �ν��Ͻ��� �߰��Ǿ����� �ð� ������ ���庤�͵� �׸�ŭ �ø�.
    static vector<_float> vecLifeTimeElapsed = {};
    
    // �߰��� ���Ұ� �ִٸ� �׸�ŭ�� �����ϰ� �� �ڸ��� �߰�
    if (uiInstDescs.size() > vecLifeTimeElapsed.size())
    {
        _uint iAddLoopTime = uiInstDescs.size() - vecLifeTimeElapsed.size();
        for (_uint i = 0; i < iAddLoopTime; i++)
            vecLifeTimeElapsed.push_back(0.f);
    }

    // ���̴� ������ ���� ���� ����
    vector<_float4x4> vecVariantMat = {};
    vecVariantMat.resize(uiInstDescs.size());

    for (uint i = 0; i < vecVariantMat.size(); i++)
    {
        *reinterpret_cast<_float2*>(&vecVariantMat[i]._11) = fDestScale;
        *reinterpret_cast<_float*>(&vecVariantMat[i]._13) = fStartAlpha;
        *reinterpret_cast<_float*>(&vecVariantMat[i]._14) = vecLifeTimeElapsed[i] / fLifeTime;
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vColor;
    }

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ACTIVEFEEDBACK),
        true
    };

    pFeedbackUI->Set_VariantUIDesc(tVariantDesc);

    // �ð��� ������ ���Ұ� �ִٸ� ����
    for (_uint i = 0; i < vecLifeTimeElapsed.size(); i++)
    {
        if (vecLifeTimeElapsed[i] >= fLifeTime)
        {
            vecLifeTimeElapsed.erase(vecLifeTimeElapsed.begin() + i);
            uiInstDescs.erase(uiInstDescs.begin() + i);

            uiDesc.vecInstanceDescs = uiInstDescs; // ����
            pFeedbackUI->Set_UIDesc(uiDesc);
            i--;
        }
        else
        {
            vecLifeTimeElapsed[i] += fTimeDelta;
        }
    }
}

void CUI_HUD::Add_UI_SkillSection_OnFeedback(_uint iSectionIndex)
{
    // Feedback ��ü�� �ν��Ͻ� �߰��ϴ� �Լ�.
    // ��ư�� ������ ���� ���� ȿ���� �����ϱ� ����.
    // ��ġ�� �Ʒ��� vStartPos �� fDeltaPosX �� ���� ������.
    // 
    // �ð� ������ ���� Ŀ����, �ð��� �� ���� �� �ν��Ͻ��� �����ϴ� ���� 
    // Update_UI_SkillSection_OnFeedback ���� ����.

    CCustom_UI* pFeedbackUI = Find_ChildObject(L"Skill_OnFeedback");

    const _float4 vStartPos = { 850.f, -415.f, 0.f, 1.f };
    const _float fDeltaPosX = -100.f;

    vector<_float4> vecPosIndex = {};

    for (_uint i = 0; i < 5; i++)
    {
        _float4 vPos = vStartPos;
        vPos.x = vPos.x + fDeltaPosX * (i);
        vecPosIndex.push_back(vPos);
    }

    auto uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;



    CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
    tDesc.vSInstRight   = { 80.f, 0.f, 0.f, 0.f };
    tDesc.vSInstUp      = { 0.f, 80.f, 0.f, 0.f };
    tDesc.vSInstLook    = { 0.f, 0.f, 1.f, 0.f };
    tDesc.vSInstTrans   = vecPosIndex[iSectionIndex];

    uiInstDescs.push_back(tDesc);
    uiDesc.vecInstanceDescs = uiInstDescs;
    pFeedbackUI->Set_UIDesc(uiDesc);
}

void CUI_HUD::Update_UI_PlayerHPBar(_float fTimeDelta)
{
    // �÷��̾��� HP �ٸ� �����մϴ�.

    // 1. �ڵ������ ü�¹ٱ��� �����Ͽ� �ν��Ͻ��� 2������ �����.
    // 2. ������ ���̴��� ����, ���� ü�¹ٿ� �ڵ������ ü�¹� 2����, ���� 2���� ���� ����Ͽ� �׶���Ʈ�ǵ��� ����

    // ksta : ���߿� �÷��̾� ���� ���յǸ� �ű��κ��� �޾ƿ� ����
    static _float fPlayerHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    static _float fPlayerBackHP[CH_END] = { fPlayerHP[0], fPlayerHP[1], fPlayerHP[2] };
    const _float fPlayerMaxHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;
    

    _float fPlayerHPRatio = fPlayerHP[m_iSelectedCHIndex] / fPlayerMaxHP[m_iSelectedCHIndex];
    static _float fPlayerHPBackRatio = fPlayerHPRatio;

    _float4 vHPColor        = { 1.f, 1.f, 1.f, 1.f };
    _float4 vHPBackColor    = { 1.f, 0.f, 0.f, 1.f };

    const _float fHPReduceTime = 0.5f;          // �پ��� �ҿ�ð��� 0.5������?

    const auto targetUI = Find_ChildObject(L"Inst_HPBar");


    
    if (fHPReduceLeftTime > 0)
    {
        _float diff = fPlayerHPBackRatio - fPlayerHPRatio;              // ü�� ���� ����

        if (diff > 0.f)
        {
            _float delta = diff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� ü�� ����

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

        // HP�� ��� ����
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
    


    // ksta : ���߿� ���� ���� ���յǸ� �ű��κ��� �޾ƿ� ����
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

    const _float fHPReduceTime = 0.5f;          // �پ��� �ҿ�ð��� 0.5������?

    const auto targetUI = Find_ChildObject(L"Inst_BossHPBar");
    const auto targetSAUI = Find_ChildObject(L"Inst_BossSABar");


    if (fHPReduceLeftTime > 0)
    {
        _float fHPDiff = fBossHPBackRatio - fBossHPRatio;              // ü�� ���� ����
        _float fSADiff = fBossSABackRatio - fBossSARatio;              // �Ƹ� ���� ����

        if (fHPDiff > 0.f)
        {
            _float fHPDelta = fHPDiff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� ü�� ����

            fBossHPBackRatio -= fHPDelta;
            if (fBossHPBackRatio < fBossHPRatio)
                fBossHPBackRatio = fBossHPRatio;
        }
        if (fSADiff > 0.f)
        {
            _float fSADelta = fSADiff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� �Ƹ� ����

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

        // HP�� ��� ����
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
    // ĳ���Ϳ� ���� Ű ���̵� ���̱� ���� �б�
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


    



    // �Ӽ� ������ ���� ����
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

    
    
    // �Ӽ� ������ �ֺ� ���� �� ����
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

    // ksta : ���� ����ȸ�� ��ġ ���߿� �޾ƿ� ��
    m_fPlayerEnergy;
    m_fPlayerMaxEnergy;

    _float fCurPlayerEnergy         = m_fPlayerEnergy[m_iSelectedCHIndex];
    _float fCurPlayerMaxEnergy      = m_fPlayerMaxEnergy[m_iSelectedCHIndex];
    
    static _float fGalbEchoEnergy   = 0.f;
    const _float fGalbMaxEchoEnergy = 50.f;

    _float fCurPlayerEnergyRatio    = fCurPlayerEnergy / fCurPlayerMaxEnergy;
    

    _float4     vSingleColor[2] = {};   // for Gradiant
    _float4     vExtraColor[2] = {};    // for Galbrena. �Ӹ��� ȥ�� ����ȸ�ο� �÷� �¿��� �ΰ���
    _bool       isSingleVisible = {};   //
    //_float      fSingleHeight = {};   // vSpectrumHeights, vBackSpectrumHeights

    _float4     vBackColor[2] = {};
    _bool       isBackVisible = {};
    _float      fBackHeight = {};

    const auto targetUI = Find_ChildObject(L"Inst_EnergyItems");



    // �׽�Ʈ�� �Է��� ���� �� ����
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
    enum HUD_PLAYER_ENCOLOR {           // ���������� ���� ���п� ������
        ENCL_ROVER_NORMAL, 
        ENCL_AUGUSTA_NORMAL,
        ENCL_AUGUSTA_ULT,
        ENCL_GALBRENA_NORMAL_L,
        ENCL_GALBRENA_NORMAL_R,
        ENCL_GALBRENA_ULT,

        ENCL_STATIC,
        ENCL_END
    };
    const _uint iNumSpectrums = 41;     // ����Ʈ���� �� ���� �� ������ �ִ� 41���� ����


    // �÷� ������ ����
    vector<array<_float4, 2>> vecColorPreset;       // { start color (bottom), end color (top) }
    vecColorPreset.resize(ENCL_END);

    vecColorPreset[ENCL_ROVER_NORMAL]       = { _float4{0.961f, 0.192f, 0.502f, 1.f}, _float4{0.961f, 0.192f, 0.502f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_NORMAL]     = { _float4{0.769f, 0.631f, 0.933f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_ULT]        = { _float4{1.000f, 0.953f, 0.722f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_L]  = { _float4{0.894f, 0.573f, 0.525f, 1.f}, _float4{0.922f, 0.490f, 0.486f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_R]  = { _float4{0.682f, 0.769f, 0.980f, 1.f}, _float4{0.553f, 0.557f, 0.878f, .8f} };
    vecColorPreset[ENCL_GALBRENA_ULT]       = { _float4{0.482f, 0.412f, 0.878f, 1.f}, _float4{0.867f, 0.824f, 0.957f, .8f} };

    vecColorPreset[ENCL_STATIC]             = { _float4(1.f, .8f, .8f, .35f), _float4(1.f, .8f, .8f, .35f) }; // ��á�� �� ����



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

    // �߰��� �ڿ��� ������ ���� ����Ʈ���� ����, �̴� ���� ���� �������� ���� �ܼ��� ����ŷ�� �� ��. �׷��Ƿ� ���� 1.f ����.
    // ��, �ν��Ͻ� 41���� ���� ��ü 3���� �׸��ų�, ������ ���ļ� �ѹ��� 41*3���� �׸��ų� �� ��

    // ����, ���� �κ� ������ ���� �������� ĳ���� ������ ����
    // ksta : 1) ���극�� ���� �����°� �Ʒ� ���� �����ÿ� �б��� ���� �� (��)
    // ksta : 2) �������� ������ ���� ���� �÷��̾�� �ٸ�. �̴� �ش� �б⿡�� ����.
    // ksta : 3) ĳ���Ϳ� ���� �߰��ڿ��� �ִ� ���쵵 ����.(�ƿ챸��Ÿ �߰���, Į. ���� �ް�����)
    //          �̴� ���� �Լ� ���� �ļ� �ű⼭ ����.

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

    // ���� ��ġ �Ҵ�
    for (_uint i = 0; i < iNumSpectrums; ++i)
    {
        // ��ǥġ�� õõ�� ���� (�ε巴�� ����)
        vSpectrumHeights[VALUE][i] += (vSpectrumHeights[TARGET][i] - vSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;
        vBackSpectrumHeights[VALUE][i] += (vBackSpectrumHeights[TARGET][i] - vBackSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;

        // ���� Ȯ���� ���ο� ��ǥ�� ����
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


    // ���̴� ���޿� ���Ŀ� �� ����
    for (uint i = 0; i < vecVariantMat.size(); i++)         // front spectrum. �տ��� �� �������� �����̴� �װ�.
    {
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSingleColor[0];
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vSingleColor[1];
        vecVariantMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        vecVariantMat[i]._32 = vSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantBackMat.size(); i++)     // back spectrum.  �ڿ��� ���� ������ ä�� �����̴� �װ�.
    {
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._11) = vBackColor[0];
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._21) = vBackColor[1];
        vecVariantBackMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        //vecVariantBackMat[i]._31 = false;5
        vecVariantBackMat[i]._32 = vBackSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantStaticMat.size(); i++)   // static spectrum. �տ��� ���������� �� á�� �� �������� �ʴ� �װ�.
    {
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._11) = vecColorPreset[ENCL_STATIC][0];   // �⺻�� ���� ����.
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._21) = vecColorPreset[ENCL_STATIC][1];   // �⺻�� ���� ����.
        vecVariantStaticMat[i]._31 = static_cast<_float>(vIsVisibleStatic[i]);                  // �������� ���� �ʾ� �׷����� �ʴ� �κи� �׸�.
        //vecVariantStaticMat[i]._31 = false;                                                     // �������� ���� �ʾ� �׷����� �ʴ� �κи� �׸�.
        vecVariantStaticMat[i]._32 = 1.f;                                                       // ũ�� 1�� ����
    }

    // ���극�� ����ó�� - �¿� ���� ����
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
    
    targetUI->Set_VariantUIDesc(tVariantDesc);  // �ν��Ͻ� 123���� ���� �ѹ��� �׸��Բ� ����

#ifdef KSTA_UI_ENERGYBARTEST
    std::cout << "[UI_HUD][Update_UI_PlayerEnergyBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_PlayerEnergyBar] fPlayerEnergy : " << m_fPlayerEnergy[m_iSelectedCHIndex] << std::endl;
#endif // KSTA_UI_ENERGYBARTEST

}

void CUI_HUD::Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta)
{
    if (m_iSelectedCHIndex != CH_AUGUSTA)
        return;


    // �߾� ������, Į�� �� �� ����
    // ksta : ���߿� �޾ƿ;� ��

    static _uint iSwordEnergy = 0;            // mAX = 2

    static _float fPointEnergy = 0.f;
    const _float fMaxPointEnergy = 100.f;

    static _float fUltBladeEnergy = 0.f;
    const _float fMaxUltBladeEnergy = 100.;


    CCustom_UI* pBladeUI    = Find_ChildObject(L"Frame_Augusta_Inst_SwordEnergy");          // �ν��Ͻ� 0���� ����, 1���� ������ Į �ڿ�.
    CCustom_UI* pPointUI    = Find_ChildObject(L"Frame_Augusta_Inst_CenterPointEnergy");    // �ϴܺ� ���� �ڿ�

    CCustom_UI* pUltBladeUI = Find_ChildObject(L"Frame_Augusta_Inst_UltModeEnergy");        // �ñر� �� �����ϴ� �ڿ�. ����. 



    // Į �ڿ�
    if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
    {
        iSwordEnergy++;
        if (iSwordEnergy > 2) iSwordEnergy = 0;
    }

    // ���� �ڿ�
    if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
    {
        if (fPointEnergy == 100) fPointEnergy = 0;
        else
        {
            fPointEnergy += m_pGameInstance->Rand(10.f, 40.f);
            if (fPointEnergy >= 100) fPointEnergy = 100;
        }
    }

    // �ñر� �ڿ�
    if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
    {
        if (fUltBladeEnergy == 100) fUltBladeEnergy = 0;
        else
        {
            //fUltBladeEnergy += m_pGameInstance->Rand(10.f, 40.f);
            fUltBladeEnergy += 100.f / 7.f;
            if (fUltBladeEnergy >= 99.9f) fUltBladeEnergy = 100;
        }
    }


    if (m_iEnergyBarMode == 0)
    {
        // �Ϲ� UI
        pBladeUI    ->Set_Active(true);
        pPointUI    ->Set_Active(true);
        pUltBladeUI ->Set_Active(false);
    }

    else if (m_iEnergyBarMode == 1)
    {
        // �� UI
        pBladeUI    ->Set_Active(false);
        pPointUI    ->Set_Active(false);
        pUltBladeUI ->Set_Active(true);
    }




    auto bladeDesc = pBladeUI->Get_UIDesc();

    auto ultBladeDesc = pUltBladeUI->Get_UIDesc();
    
    // Į �ڿ�
    // �׳� ������ ���� ������ ���� �����ֱ�. alpha pass �̿�.
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
    



    // ���� �ڿ�
    // ������ ��Ÿ�ӿ� ���̴� Ȱ��. vatiant pass �̿�.

    // ! �ݵ��� pass ���� �ʿ�
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




    // �ñر� �ڿ�
    // ���� ���̴��� �������� �� Ȱ��. alpha pass �̿�.
    _float ultRatio = fUltBladeEnergy / fMaxUltBladeEnergy;
    static _float fPreUltRatio = 0.f;

    if (ultRatio > fPreUltRatio) fPreUltRatio += fTimeDelta * 1.5f;
    if (ultRatio <= fPreUltRatio) fPreUltRatio = ultRatio;

    ultBladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, fPreUltRatio };

    pUltBladeUI->Set_UIDesc(ultBladeDesc);

}

void CUI_HUD::Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta)
{
    // ���� ���ڹٴ� Update_UI_PlayerEnergyBar ���� ó��
    // 
    // ��ȭ������ �� �ϴ� ������ �� ����


    // ��ȭ ���� ����
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
