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

	SKYBOX_DESC* pDesc = static_cast<SKYBOX_DESC*>(pArg);
	m_iNumModels = pDesc->iNumModel;
	m_fCloudSpeed = pDesc->fCloudSpeed;
	m_fFXScaleRate = pDesc->fFXScaleRate;
	m_vUVRate = pDesc->vUVRate;

	Ready_Component(pDesc->strModelTags);
	//Env Map Bake
	//m_pGameInstance->Add_EnvMap_SkyBox(this);

    return S_OK;
}

void CSkyBox::Priority_Update(_float fTimeDelta)
{
}

void CSkyBox::Update(_float fTimeDelta)
{
	m_fTimeAcc += fTimeDelta * m_fCloudSpeed;

#ifdef _DEBUG
	ImGui::Begin("SkyBox");
	ImGui::Text("UV"); ImGui::SameLine();
	ImGui::PushID(1);
	ImGui::InputFloat2("##", reinterpret_cast<_float*>(&m_vUVRate));
	ImGui::PopID();

	ImGui::Text("Color"); ImGui::SameLine();
	ImGui::PushID(2);
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&m_vBackGroundColor));
	ImGui::PopID();
	ImGui::End();
#endif
}

void CSkyBox::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::PRIORITY, this);
}

void CSkyBox::Render()
{
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(m_pGameInstance->Get_CamPos()));// +XMVectorSet(0.f, -10.f, 0.f, 0.f));

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pShaderCom->Bind_Value("g_fCloudSpeed", &m_fTimeAcc, sizeof(_float));
	m_pShaderCom->Bind_Value("g_fFXScaleRate", &m_fFXScaleRate, sizeof(_float));
	m_pShaderCom->Bind_Value("g_vUVRate", &m_vUVRate, sizeof(_float2));
	m_pShaderCom->Bind_Value("g_vBackGroundColor", &m_vBackGroundColor, sizeof(_float3));

	for (_uint i = 0; i < m_iNumModels; ++i)
	{
		if (nullptr == m_pModelCom[i])
			continue;

		m_pModelCom[i]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE);
		m_pShaderCom->Begin(i);
		m_pModelCom[i]->Render(0);
	}
}

void CSkyBox::Render_Shadow()
{
}

void CSkyBox::Render_OutLine()
{
}

void CSkyBox::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vCenter));// +XMVectorSet(0.f, -10.f, 0.f, 0.f));

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix);

	m_pShaderCom->Bind_Value("g_fCloudSpeed", &m_fTimeAcc, sizeof(_float));

	for (_uint i = 0; i < m_iNumModels; ++i)
	{
		m_pModelCom[i]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE);
		m_pShaderCom->Begin(i);
		m_pModelCom[i]->Render(0);
	}
}

void CSkyBox::Ready_Component(const vector<_wstring>& strModelTags)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxSkyBox"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	// Com_Model_Dome
	if (FAILED(Add_Component(m_iCurrentLevel, strModelTags[ENUM_CLASS(SKYTYPE::DOME)],
		TEXT("Com_Model_Dome"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(SKYTYPE::DOME)]), nullptr)))
		CRASH("Com_Model_Dome");

	// Com_Model_Background
	if (FAILED(Add_Component(m_iCurrentLevel, strModelTags[ENUM_CLASS(SKYTYPE::BACKGROUND)],
		TEXT("Com_Model_Background"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(SKYTYPE::BACKGROUND)]), nullptr)))
		CRASH("Com_Model_Background");

	// Com_Model_FX1
	if (FAILED(Add_Component(m_iCurrentLevel, strModelTags[ENUM_CLASS(SKYTYPE::FX1)],
		TEXT("Com_Model_FX1"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(SKYTYPE::FX1)]), nullptr)))
		CRASH("Com_Model_FX1");
	
	// Com_Model_Cloud
	if(m_iNumModels > ENUM_CLASS(SKYTYPE::CLOUD))
		if (FAILED(Add_Component(m_iCurrentLevel, strModelTags[ENUM_CLASS(SKYTYPE::CLOUD)],
			TEXT("Com_Model_Cloud"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(SKYTYPE::CLOUD)]), nullptr)))
			CRASH("Com_Model_Cloud");
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
