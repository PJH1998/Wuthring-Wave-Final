#include "ClientPch.h"
#include "Heaven_SkyBox.h"

CHeaven_SkyBox::CHeaven_SkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext}
{
}

CHeaven_SkyBox::CHeaven_SkyBox(const CHeaven_SkyBox& Prototype)
	: CGameObject { Prototype }
{
	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
		m_iShaderIndex[i] = Prototype.m_iShaderIndex[i];
}

HRESULT CHeaven_SkyBox::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_iShaderIndex[ENUM_CLASS(SKYBOX::DOME)] = 0;
	m_iShaderIndex[ENUM_CLASS(SKYBOX::CLOUD)] = 1;
	m_iShaderIndex[ENUM_CLASS(SKYBOX::FX)] = 2;

    return S_OK;
}

HRESULT CHeaven_SkyBox::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if(FAILED(Ready_Component()))

    return S_OK;
}

void CHeaven_SkyBox::Priority_Update(_float fTimeDelta)
{
}

void CHeaven_SkyBox::Update(_float fTimeDelta)
{
	m_fTime = fmod(m_fTime + (fTimeDelta * 0.1f), 1.f);
}

void CHeaven_SkyBox::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::PRIORITY, this);
}

void CHeaven_SkyBox::Render()
{
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(m_pGameInstance->Get_CamPos()));

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed to Bind ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_fTime", &m_fTime, sizeof(_float))))
		CRASH("Failed to Bind Time");

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
	{
		_uint iNumMeshes = m_pModel[i]->Get_NumMesh();
		for (_uint j = 0; j < iNumMeshes; ++j)
		{
			if (FAILED(m_pModel[i]->Bind_Materials(m_pShader, "g_DiffuseTexture", j, TEXTURETYPE::DIFFUSE)))
				CRASH("Failed to Bind DiffuseTexture");

			m_pShader->Begin(m_iShaderIndex[i]);

			m_pModel[i]->Render(j);
		}
	}
}

void CHeaven_SkyBox::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

HRESULT CHeaven_SkyBox::Ready_Component()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Shader_HeavenSkyBox"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		CRASH("Failed to Add Shader Heaven SkyBox");
	
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Dome"),
		TEXT("Com_Model_Dome"), reinterpret_cast<CComponent**>(&m_pModel[ENUM_CLASS(SKYBOX::DOME)]), nullptr)))
		CRASH("Failed to Add Comp Heaven Dome");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Cloud"),
		TEXT("Com_Model_Cloud"), reinterpret_cast<CComponent**>(&m_pModel[ENUM_CLASS(SKYBOX::CLOUD)]), nullptr)))
		CRASH("Failed to Add Comp Heaven Cloud");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Fx"),
		TEXT("Com_Model_Fx"), reinterpret_cast<CComponent**>(&m_pModel[ENUM_CLASS(SKYBOX::FX)]), nullptr)))
		CRASH("Failed to Add Comp Heaven Cloud");
}

CHeaven_SkyBox* CHeaven_SkyBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHeaven_SkyBox* pInstance = new CHeaven_SkyBox(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Creatd : CHeaven_SkyBox");
		Safe_Release(pInstance);
	}
    return pInstance;
}

CGameObject* CHeaven_SkyBox::Clone(void* pArg)
{
	CHeaven_SkyBox* pInstance = new CHeaven_SkyBox(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CHeaven_SkyBox");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHeaven_SkyBox::Free()
{
	__super::Free();

	Safe_Release(m_pShader);

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
		Safe_Release(m_pModel[i]);
}
