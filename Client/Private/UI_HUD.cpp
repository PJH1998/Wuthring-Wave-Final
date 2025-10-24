#include "ClientPch.h"
#include "UI_HUD.h"

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

    // Load from json
    _wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Test9SecInstanceTree2.json";
    Load_ChildObjects(strFilePath);

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
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_HUD::Render()
{
    __super::Render();                      // Nothing.
}

HRESULT CUI_HUD::Load_ChildObjects(_wstring strFilePath)
{
    //_uint iDestLevel =  m_pGameInstance->Get_CurrentLevel();
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);


    ifstream file(strFilePath);
    json jUITreeData = {};
    if (file.is_open()) {
        file >> jUITreeData;
    }

    // json load
    CUSTOM_UITREE_DESC tLoadTreeDesc = {};
    from_json(jUITreeData, tLoadTreeDesc);

    // 그 뒤 로드..
    vector<CGameObject*> vecLoadObjects = {};
    for (auto& loadDesc : tLoadTreeDesc.vecUIInfoDescs)
    {
        UI_INFO_DESC tLoadUIInfoDesc = loadDesc;

        // Transform 값을 가져온 뒤, 행렬화하여 반영하고, 자식 오브젝트로써 추가한다.
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
        m_vecChildObjects.push_back(static_cast<CCustom_UI*>(pCustomObj)); // 로컬에 저장.. 근데 자식인 줄은 어찌 알고? 일단 추가한 뒤, 전부 부모 재정리하고, 그 뒤에 부모관계 아닌 애들만 걸러내기?
        


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
