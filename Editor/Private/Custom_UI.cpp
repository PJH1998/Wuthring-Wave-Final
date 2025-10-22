// ==============================
// * ?먮뵒?곗뿉?쒕쭔 ?ъ슜???꾩떆 UI ?ㅻ툕?앺듃
// ==============================

#include "EditorPch.h"
#include "Custom_UI.h"
#include "Animator_UI.h"

#include "VIBuffer_Rect_Instance_UI.h"

CCustom_UI::CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
{
}

CCustom_UI::CCustom_UI(const CCustom_UI& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CCustom_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCustom_UI::Initialize_Clone(void* pArg)
{
    __super::Initialize_Clone(pArg);

    Ready_Prototypes(pArg);
    Ready_Components(pArg);
    
    Bind_Description(pArg);

    __super::Begin();


    // ksta del : �׽�Ʈ��
    for (_uint i = 0; i < 1; i++)
    {
        if (static_cast<CUSTOM_UI_DESC*>(pArg)->isInstance)
        {
            CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC pTestDesc = {};
            pTestDesc.vTexcoordX = _float2{ 0.0f, 1.0f };
            pTestDesc.vTexcoordY = _float2{ 0.0f, 1.0f };

            //_float3 vPos = { 0.f + 15.f * i, 0.f + 15.f * i, 0.f };
            //_float3 vRot = { };
            //_float3 vSca = { 1.f, 1.f, 1.f };
            //_matrix mPos = XMMatrixTranslationFromVector(XMLoadFloat3(&vPos));
            //_matrix mRot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&vRot));
            //_matrix mSca = XMMatrixScalingFromVector(XMLoadFloat3(&vSca));
            //_matrix mResult = mSca * mRot * mPos;
            //_float4x4 mResultStore; XMStoreFloat4x4(&mResultStore, mResult);
            _float4x4 mResultStore; XMStoreFloat4x4(&mResultStore, XMMatrixIdentity());

            pTestDesc.vSInstRight = { mResultStore._11, mResultStore._12, mResultStore._13, mResultStore._14 };
            pTestDesc.vSInstUp    = { mResultStore._21, mResultStore._22, mResultStore._23, mResultStore._24 };
            pTestDesc.vSInstLook  = { mResultStore._31, mResultStore._32, mResultStore._33, mResultStore._34 };
            pTestDesc.vSInstTrans = { mResultStore._41, mResultStore._42, mResultStore._43, mResultStore._44 };

            m_InstanceDescs.push_back(pTestDesc);
        }

    }

	return S_OK;
}

void CCustom_UI::Priority_Update(_float fTimeDelta)
{
}

void CCustom_UI::Update(_float fTimeDelta)
{
    if (m_tUIDesc.isInstance)
    {
        dynamic_cast<CVIBuffer_Rect_Instance_UI*>(m_pVIBufferCom)->Update_Instances(fTimeDelta, m_InstanceDescs);
    }

    m_pAnimator_UICom->Update(fTimeDelta);

    Update_CombinedMatrix();
}

void CCustom_UI::Late_Update(_float fTimeDelta)
{
    //if (!m_isActive)
    //    return;

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::UI, this)))
        return;
}

void CCustom_UI::Render()
{
    //__super::Begin();

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        CRASH("Binding_Matrix_Failed");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
        CRASH("Binding_Matrix_Failed");
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
        CRASH("Binding_Matrix_Failed");

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_Texture", m_iCurTexIndex)))
        CRASH("Binding_Matrix_Failed");
    if (FAILED(m_pShaderCom->Bind_Value("g_InverseScreenDiscard", &m_tUIDesc.isInverseScreenDiscard, sizeof(m_tUIDesc.isInverseScreenDiscard))))
        CRASH("Binding_Value_Failed");
    if (FAILED(m_pShaderCom->Bind_Value("g_CutoutAlphaDiscard", &m_tUIDesc.fCutout, sizeof(m_tUIDesc.fCutout))))
        CRASH("Binding_Value_Failed");

    // �̹��� ũ�� �Ѱ��ֱ�
    if (FAILED(m_pShaderCom->Bind_Value("g_ImageSize", &m_tUIDesc.vecSize[m_iCurTexIndex], sizeof(m_tUIDesc.vecSize[m_iCurTexIndex]))))
        CRASH("Binding_Value_Failed");
    if (FAILED(m_pShaderCom->Bind_Value("g_SectorBorder", &m_tUIDesc.vSectorBorder, sizeof(m_tUIDesc.vSectorBorder))))
        CRASH("Binding_Value_Failed");  
    if (FAILED(m_pShaderCom->Bind_Value("g_UIScale", &m_tUIDesc.fUIScale, sizeof(m_tUIDesc.fUIScale))))
        CRASH("Binding_Value_Failed");





    m_pShaderCom->Begin(m_tUIDesc.iPassType); // Gradient

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

HRESULT CCustom_UI::Ready_Prototypes(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const   _wstring    strFilePath = pDesc->strFilePath;
    const   _wstring	strFileName = pDesc->strFileName;
    const   _uint       iNumFiles   = pDesc->iNumFiles;

    const   _uint       iDestLevel  = ENUM_CLASS(LEVEL::UI);

    // ?띿뒪爾??꾨줈?좏??낇솕
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
        CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");

    return S_OK;
}

HRESULT CCustom_UI::Ready_Components(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const _wstring      strFilePath = pDesc->strFilePath;
    const _wstring	    strFileName = pDesc->strFileName;
    const _uint         iNumFiles   = pDesc->iNumFiles;

    const   _uint       iDestLevel  = ENUM_CLASS(LEVEL::UI);
    const   _bool       isInstance  = pDesc->isInstance;

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Texture_Custom_" + strFileName),
        TEXT("Com_Texture_Custom_") + strFileName, reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (!isInstance) {
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
            TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
            return E_FAIL;
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
            TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
            return E_FAIL;
    }
    else if (isInstance) {
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
            TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
            return E_FAIL;
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
            TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
            return E_FAIL;
    }

    CAnimator_UI::ANIMATOR_UI_DESC tAnimatorUIDesc = { this };
    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Animator_UI"),
        TEXT("Com_Animator_UI"), reinterpret_cast<CComponent**>(&m_pAnimator_UICom), &tAnimatorUIDesc)))
        return E_FAIL;

	return S_OK;
}

HRESULT CCustom_UI::Bind_Description(void* pArg)
{
    ASSERT_CRASH(pArg);

    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);


    m_tUIDesc.strFilePath   = pDesc->strFilePath;
    m_tUIDesc.strFileName   = pDesc->strFileName;
    m_tUIDesc.iNumFiles     = pDesc->iNumFiles;

    m_tUIDesc.strUIName     = ((pDesc->strUIName).empty())? m_tUIDesc.strFileName : pDesc->strUIName; // 鍮꾩뼱?덈떎硫?珥덇린媛믪쑝濡?strFileName ?ъ슜
    m_tUIDesc.iUIType       = pDesc->iUIType;
    m_tUIDesc.strParentName = pDesc->strParentName;
    m_tUIDesc.fCutout       = pDesc->fCutout;
    m_tUIDesc.iPassType     = pDesc->iPassType;		// 0 : Normal, 1 : Cutout, 2 : Transparent, 3 : SimpleGradient
    m_tUIDesc.vecChildNames = pDesc->vecChildNames;

    // ���� �̹��� Size ��������.
    uint iIndex = 0;
    while (true)
    {
        ID3D11ShaderResourceView* pSRV = m_pTextureCom->Get_SRV(iIndex);
        if (pSRV == nullptr) break;
        ID3D11Resource* pResource;
        pSRV->GetResource(&pResource);
        ID3D11Texture2D* pTexture;
        if (pResource)
        {
            HRESULT hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
            if (SUCCEEDED(hr) && pTexture)
            {
                D3D11_TEXTURE2D_DESC desc = {};
                pTexture->GetDesc(&desc);
                m_tUIDesc.vecSize.push_back(_float2{ (_float)desc.Width , (_float)desc.Height });
                pTexture->Release();
            }
            pResource->Release();
        }
        iIndex++;
    }

    m_tUIDesc.vSectorBorder = pDesc->vSectorBorder;
    m_tUIDesc.fUIScale      = pDesc->fUIScale;
    m_tUIDesc.isInstance    = pDesc->isInstance;

    return S_OK;
}

void CCustom_UI::Update_CombinedMatrix()
{
    if (m_tUIDesc.pParentObject)
    {
        CTransform* pParentTransform = dynamic_cast<CTransform*>(m_tUIDesc.pParentObject->Get_Component(L"Com_Transform"));
        XMStoreFloat4x4(&m_CombinedWorldMatrix, m_pTransformCom->Get_WorldMatrix() * pParentTransform->Get_WorldMatrix());
    }
    else
        XMStoreFloat4x4(&m_CombinedWorldMatrix, m_pTransformCom->Get_WorldMatrix());
}

CCustom_UI* CCustom_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCustom_UI* pInstance = new CCustom_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CCustom_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCustom_UI::Clone(void* pArg)
{
    CCustom_UI* pInstance = new CCustom_UI(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CCustom_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCustom_UI::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pAnimator_UICom);
}
