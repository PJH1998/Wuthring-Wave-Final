#include"EditorPch.h"
#include "Edit_PreViewModel.h"

CEdit_PreViewModel::CEdit_PreViewModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
{
}

CEdit_PreViewModel::CEdit_PreViewModel(const CEdit_PreViewModel& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CEdit_PreViewModel::Initialize_Prototype()
{
    return S_OK;
}

void CEdit_PreViewModel::Priority_Update(_float fTimeDelta)
{
}

void CEdit_PreViewModel::Update(_float fTimeDelta)
{
}

void CEdit_PreViewModel::Late_Update(_float fTimeDelta)
{
}

void CEdit_PreViewModel::Render(_wstring ModelName)
{
}

void CEdit_PreViewModel::Add_Model(_wstring ModelName)
{
    _wstring ModelCom = TEXT("Com_") + ModelName;
    //CModel* pModel = m_Models[ModelName];
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), ModelName,
        ModelCom, reinterpret_cast<CComponent**>(&m_Models[ModelName]), nullptr)))
        int a = 0;
}

CEdit_PreViewModel* CEdit_PreViewModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_PreViewModel* pInstance = new CEdit_PreViewModel(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CEdit_PreViewModel");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_PreViewModel::Free()
{
    __super::Free();
    for (auto& Pair : m_Models)
        Safe_Release(Pair.second);

    m_Models.clear();
}
