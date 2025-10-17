#include "ClientPch.h"
#include "ShadowDummy.h"

CShadowDummy::CShadowDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject { pDevice, pContext }
{
}

CShadowDummy::CShadowDummy(const CShadowDummy& Prototype)
    : CGameObject { Prototype }
{
}

HRESULT CShadowDummy::Initialize_Prototype()
{
    if(FAILED(__super::Initialize_Prototype()))
        CRASH("melong")

    return S_OK;
}

HRESULT CShadowDummy::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        CRASH("clone failed")

	Ready_Component();
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, -150.f, 0.f, 1.f));

    return S_OK;
}

void CShadowDummy::Priority_Update(_float fTimeDelta)
{
}

void CShadowDummy::Update(_float fTimeDelta)
{
}

void CShadowDummy::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CShadowDummy::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

}

void CShadowDummy::Render_Shadow()
{
}

void CShadowDummy::Ready_Component()
{	
    // Com_Shader
    Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>( &m_pShaderCom ), nullptr);

    // Com_Model
    Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Model_Wolf"),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>( &m_pModelCom ), nullptr);
}

CShadowDummy* CShadowDummy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShadowDummy* pInstance = new CShadowDummy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CShadowDummy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CShadowDummy::Clone(void* pArg)
{
	CShadowDummy* pClone = new CShadowDummy(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CShadowDummy (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CShadowDummy::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
