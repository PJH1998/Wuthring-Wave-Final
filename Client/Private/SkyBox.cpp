#include "ClientPch.h"
#include "SkyBox.h"

CSkyBox::CSkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CSkyBox::CSkyBox(const CSkyBox& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CSkyBox::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSkyBox::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Model Tag None");

	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SkyBox");

	m_iCurrentLevel = m_pGameInstance->Get_CurrentLevel();

    return S_OK;
}

void CSkyBox::Priority_Update(_float fTimeDelta)
{
}

void CSkyBox::Update(_float fTimeDelta)
{
}

void CSkyBox::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::PRIORITY, this);
}

void CSkyBox::Render()
{
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(m_pGameInstance->Get_CamPos()));

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	//m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE);
	//
	//m_pModelCom->Render(0);
}

void CSkyBox::Render_Shadow()
{
}

void CSkyBox::Render_OutLine()
{
}

void CSkyBox::Ready_Component(const _wstring& strModelTag)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	// Com_Model
	if (FAILED(Add_Component(m_iCurrentLevel, strModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");
}

CSkyBox* CSkyBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSkyBox* pInstance = new CSkyBox(pDevice, pContext);

	if(FAILED(pInstance->Initialize_Prototype()))
		CRASH("SkyBox")

    return pInstance;
}

CGameObject* CSkyBox::Clone(void* pArg)
{
	CSkyBox* pClone = new CSkyBox(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SkyBox")

	return pClone;
}

void CSkyBox::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for(_uint i = 0; i < ENUM_CLASS(SKYTYPE::END); ++i)
		Safe_Release(m_pModelCom[i]);
}
