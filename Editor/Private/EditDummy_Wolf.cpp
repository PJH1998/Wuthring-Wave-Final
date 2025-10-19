#include "EditorPch.h"
#include "EditDummy_Wolf.h"

CEditDummy_Wolf::CEditDummy_Wolf(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEditDummy { pDevice, pContext }
{
}

CEditDummy_Wolf::CEditDummy_Wolf(const CEditDummy_Wolf& Prototype)
    : CEditDummy { Prototype }
{
}

HRESULT CEditDummy_Wolf::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        CRASH("Failed Init_Prototype Dummy_Wolf");

    return S_OK;
}

HRESULT CEditDummy_Wolf::Initialize_Clone(void* pArg)
{
    if (nullptr == pArg)
        CRASH("Failed to Cloned : Dummy_Wolf");

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    DUMMY_WOLF_DESC* pDesc = static_cast<DUMMY_WOLF_DESC*>( pArg );

    if (FAILED(Ready_Components(pDesc->PreTransformMatrix)))
        return E_FAIL;

    return S_OK;
}

void CEditDummy_Wolf::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Wolf::Update(_float fTimeDelta)
{
}

void CEditDummy_Wolf::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
    m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
}

void CEditDummy_Wolf::Render()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    _uint iNumMesh = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMesh; ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

        _bool HasNormal = { false };
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
            HasNormal = true;

        m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

        m_pShaderCom->Begin(0);
        m_pModelCom->Render(i);
    }
}

void CEditDummy_Wolf::Render_Shadow()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");

    m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

    _uint iNumMesh = m_pModelCom->Get_NumMesh();

    for (_uint i = 0; i < iNumMesh; ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
        m_pShaderCom->Begin(5);

        m_pModelCom->Render(i);
    }
}

HRESULT CEditDummy_Wolf::Ready_Components(_fmatrix PreTransformMatrix)
{
    m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat");
    ASSERT_CRASH(m_pModelCom);

    m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
    ASSERT_CRASH(m_pShaderCom);

    return S_OK;
}

CEditDummy_Wolf* CEditDummy_Wolf::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEditDummy_Wolf* pInstance = new CEditDummy_Wolf(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEditDummy_Wolf");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CEditDummy_Wolf::Clone(void* pArg)
{
    CEditDummy_Wolf* pInstance = new CEditDummy_Wolf(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEditDummy_Wolf");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CEditDummy_Wolf::Free()
{
    __super::Free();

    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
}
