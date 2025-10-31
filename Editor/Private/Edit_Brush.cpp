#include "EditorPch.h"
#include "Edit_Brush.h"
#include"Edit_MapObject_Instance.h"
CEdit_Brush::CEdit_Brush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice, pContext)

{
}

CEdit_Brush::CEdit_Brush(const CEdit_Brush& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CEdit_Brush::Initialize_Prototype()
{
    __super::Initialize_Clone(nullptr);

    Ready_Components();
    m_fRange = 100.f;
    m_iNumInstance = 10.f;
    m_iMinNum = 1;
    m_iMaxNum = 100;

    return S_OK;
}

HRESULT CEdit_Brush::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CEdit_Brush::Priority_Update(_float fTimeDelta)
{
    //ImGui::Begin("Set Foliage Info");

    ImGui::InputFloat("Range : ", &m_fRange);
    ImGui::SliderFloat("Range Slider : ", &m_fRange, 1.f, 4000.f, "%.1f");
    ImGui::InputScalar("Instance Num Value : ", ImGuiDataType_U32, &m_iNumInstance);
    ImGui::SliderScalar("Instance Num Value Slider : ", ImGuiDataType_U32, &m_iNumInstance, &m_iMinNum, &m_iMaxNum, "%d");

    ImGui::SliderFloat("Min Degree", &m_vMinRotation, 0.0f, 359.9f, "%.1f");
    ImGui::SliderFloat("Max Degree", &m_vMaxRotation, 0.0f, 360.f, "%.1f");
    //ImGui::End();
}

void CEdit_Brush::Update(_float fTimeDelta)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        Foliage();
    }

//#ifdef _DEBUG
//    m_pGameInstance->Add_Render_Object(RENDERGROUP::RD_DEBUG, this);
//#endif
}

void CEdit_Brush::Late_Update(_float fTimeDelta)
{
}

void CEdit_Brush::Render()
{
    return;

    Bind_Resources();
    m_pShaderCom->Begin(0);
    m_pVIBufferCom->Bind_Resources();
    m_pVIBufferCom->Render();
}

void CEdit_Brush::Set_ModelName(const _wstring& pModelName)
{
    lstrcpy(m_ModelName, pModelName.c_str());
}

void CEdit_Brush::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShaderCom, "g_DepthTexture")))
        CRASH("Render Fail")

    m_pShaderCom->Bind_Value("g_fRange", &m_fRange, sizeof(_float));
    m_pShaderCom->Bind_Value("g_iNumInstance", &m_iNumInstance, sizeof(_uint));
}

void CEdit_Brush::Ready_Components()
{
    //__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_Brush"),
    //    TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

    __super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_VIBuffer_Point"),
        TEXT("Com_VIBufferCom"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr);
}

void CEdit_Brush::Foliage()
{
    if (wcslen(m_ModelName) == 0)
        return;

    if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
    {
        vector<_float4> m_Points;
        _uint iNumPixels = {};
        _float4 MousePos = {};
        if (m_pGameInstance->Get_Points(m_fRange, m_Points, &iNumPixels,&MousePos))
        {
            _float4x4* pTransformMatrix = new _float4x4[m_iNumInstance];
            for (_uint i = 0; i < m_iNumInstance; ++i)
            {
                _uint RandNum = {};
                _float fRotation = {};
                do {
                    RandNum = m_pGameInstance->Rand(0, m_Points.size() - 1);
                    fRotation = m_pGameInstance->Rand(m_vMinRotation, m_vMaxRotation);
                } while (m_Points[RandNum].w == 0);

                if(m_vMaxRotation==0.0f)
                    XMStoreFloat4x4(&pTransformMatrix[i], XMMatrixTranslationFromVector(XMLoadFloat4(&m_Points[RandNum])));
                else
                {
                    
                    _vector RotationQuat = XMQuaternionRotationNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fRotation));
                    XMStoreFloat4x4(&pTransformMatrix[i], XMMatrixRotationQuaternion(RotationQuat) * XMMatrixTranslationFromVector(XMLoadFloat4(&m_Points[RandNum])));
                }
            }

            CEdit_MapObject_Instance::MAP_LOAD Desc;
            Desc.WorldMatrix = pTransformMatrix;
            Desc.iNumInstance = m_iNumInstance;
            strcpy_s(Desc.ModelName, WStringToString(m_ModelName).c_str());
            Desc.m_WolrdPos = MousePos;
            m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_GameObject_MapObject_Instance")
                , ENUM_CLASS(LEVEL::MAP), TEXT("Layer_Instance"), &Desc);
            Safe_Delete_Array(pTransformMatrix);
        }
    }
}

CEdit_Brush* CEdit_Brush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_Brush* pInstance = new CEdit_Brush(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : Edit_Brush");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_Brush::Free()
{
    __super::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
}
