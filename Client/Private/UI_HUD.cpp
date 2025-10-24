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
    Ready_Components(pArg);
    __super::Ready_Events();

    // Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
    _wstring strFilePath = 
        L"../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json";
    Load_ChildObjects(strFilePath);

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {   // 로드할 애니메이션은 여기에 추가
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/TestHUDAnim3.json"
    };
    Load_Animations(vecAnimFilePaths);

    return S_OK;
}

void CUI_HUD::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_HUD::Update(_float fTimeDelta)
{
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
    __super::Render();                      // Nothing.
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
        _bool isChild = false;

        const CUSTOM_UI_DESC& tChildDesc = child->Get_UIDesc();
        for (auto& otherChild : m_vecChildObjects)
        {
            const CUSTOM_UI_DESC& tOtherChildDesc = otherChild->Get_UIDesc();

            // child를 자식으로 가졌는가?
            for (auto& otherChildName : tOtherChildDesc.vecChildNames)
            {
                // 가졌다면, 자식으로 판정, 즉시 break.
                if (otherChildName == tChildDesc.strUIName)
                    isChild = true; break;
            }
            if (isChild)  break;
        }

        // 아무도 자식으로 가지지 않았다면, 컨테이너 UI의 부모로 판단.
        if (!isChild)
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
