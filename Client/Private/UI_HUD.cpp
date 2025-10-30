#include "ClientPch.h"
#include "UI_HUD.h"
#include "Animator_UI.h"

//#define KSTA_UI_COOLDOWNTEST
//#define KSTA_UI_HPBARTEST
//#define KSTA_UI_HPBARBOSSTEST
#define KSTA_UI_ENERGYBARTEST


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
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD.json";
    Load_ChildObjects(strFilePath);

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {   // �ε��� �ִϸ��̼��� ���⿡ �߰�
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
    Update_UI_Cooldown(fTimeDelta);
    Update_UI_PlayerHPBar(fTimeDelta);
    Update_UI_BossHPBar(fTimeDelta);
    Update_UI_PlayerEnergyBar(fTimeDelta);

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

void CUI_HUD::Update_UI_Cooldown(_float fTimeDelta)
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
    enum HUD_SKILL_INDEX    { SK_E, SK_R, SK_END };

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
        if (fChangeCD[0] == 0)
        {
            m_iSelectedCHIndex = 0;
            fChangeCD[0] = fMaxChangeCD[0];
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_2) == KEYSTATE::DOWN)
    {
        if (fChangeCD[1] == 0)
        {
            m_iSelectedCHIndex = 1;
            fChangeCD[1] = fMaxChangeCD[1];
        }
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_3) == KEYSTATE::DOWN)
    {
        if (fChangeCD[2] == 0)
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


    // UI���� ĳ���� ������ŭ ����.

    // 1. ��ųUI �� ����� ER / �ƿ� ER / ���� ER ��Ÿ�� �Ҵ�
    // 2. ��üUI �� ����� / �ƿ� / ���� ��Ÿ�� �Ҵ�
    
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
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
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
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_RECT),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);
    }


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
    // �÷��̾��� HP �ٸ� �����մϴ�.

    // 1. �ڵ������ ü�¹ٱ��� �����Ͽ� �ν��Ͻ��� 2������ �����.
    // 2. ������ ���̴��� ����, ���� ü�¹ٿ� �ڵ������ ü�¹� 2����, ���� 2���� ���� ����Ͽ� �׶���Ʈ�ǵ��� ����

    enum HUD_CHAR_INDEX { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };
    enum HUD_PLAYER_HPBAR { PLHP_BACK, PLHP_NORMAL, PLHP_END };

    // ksta : ���߿� �÷��̾� ���� ���յǸ� �ű�κ��� �޾ƿ� ����
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
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._11) = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._11)    = vHPBackColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._21) = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._21)    = vHPBackColor;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_NORMAL]._31)       = fPlayerHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_BACK]._31)     = fPlayerHPBackRatio;

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




    enum HUD_BOSS_HPBAR{ BOHP_BACK, BOHP_NORMAL, BOHP_END };
    enum HUD_BOSS_SABAR{ BOSA_BACK, BOSA_NORMAL, BOSA_END };

    // ksta : ���߿� ���� ���� ���յǸ� �ű�κ��� �޾ƿ� ����
    static _float fBossHP = { 10000.f };            // boss hitpoint
    static _float fBossBackHP = fBossBackHP;
    const _float fBossMaxHP = { 10000.f };
    
    static _float fBossSA = { 4000.f };          // boss superarmor
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
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._11)    = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._11)  = vSABackColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._21)    = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._21)  = vSABackColor;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_NORMAL]._31)     = fBossSARatio;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_BACK]._31)   = fBossSABackRatio;


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

void CUI_HUD::Update_UI_PlayerEnergyBar(_float fTimeDelta)
{
    static _bool isFirstUpdate = true;
    
    // �Ʒ��͵� �������ְ� �Ѱ���� ��
    // �����Ϳ��� ���� �� �׸�ŭ�� ���� ������ִ� �� ��������
    _float4     vSingleColor = { };
    _bool       isSingleVisible = {};
    _float      fSingleHeight = {};

    _float4     vBackColor = { };
    _bool       isBackVisible = {};
    _float      fBackHeight = {};


    if (isFirstUpdate)
    {
        isFirstUpdate = false;

        // 1. �������� ĳ���Ϳ� �´°ɷ� ��ü, ���� ��ü


        // 2. �ܷ��̴� ���� �����ؼ� �����̳ʷ� ����� ���� ���� ������

        
        // 3. �ݵ��!!!!! �ν��Ͻ� ����, variant flag ����� �� �� �´��� Ȯ���ϱ�


        // 4. ���� ������ ���� ���̴��ܿ��� ���� �ȼ��� ������ �ʿ� = ������ pixel shader ���� �ʿ�
    }

    vector<_float4x4> vecVariantMat = {};
    vecVariantMat.resize(41);
    vector<_float4x4> vecVariantBackMat = {};
    vecVariantMat.resize(41);

    for (uint i = 0; i < vecVariantMat.size(); i++)
    {
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSingleColor;
        vecVariantMat[i]._21 = static_cast<_float>(isSingleVisible);
        vecVariantMat[i]._22 = fSingleHeight;
    }
    for (uint i = 0; i < vecVariantBackMat.size(); i++)
    {
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._11) = vBackColor;
        vecVariantBackMat[i]._21 = static_cast<_float>(isBackVisible);
        vecVariantBackMat[i]._22 = fBackHeight;
    }

    vecVariantMat.insert(vecVariantMat.end(),           // combine two vector. -> size = 41 + 41 = 82
        make_move_iterator(vecVariantBackMat.begin()),
        make_move_iterator(vecVariantBackMat.end()));


    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };

#ifdef KSTA_UI_ENERGYBARTEST

#endif // KSTA_UI_ENERGYBARTEST


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
