#include "ClientPch.h"
#include "Logo_SkyBox.h"

CLogo_SkyBox::CLogo_SkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLogo_SkyBox::CLogo_SkyBox(const CLogo_SkyBox& Prototype)
	: CGameObject{ Prototype }
{
	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
	{
		m_SkyMatrices[i] = Prototype.m_SkyMatrices[i];
		m_iShaderIndex[i] = Prototype.m_iShaderIndex[i];

#ifdef _DEBUG
		m_DebugSkyMatrices[i] = Prototype.m_SkyMatrices[i];
#endif
	}
}

HRESULT CLogo_SkyBox::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;


	m_SkyMatrices[ENUM_CLASS(SKYBOX::BACK)] =
		XMMatrixScaling(5.f, 5.f, 1.f) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(-45.f), 0.f))
		* XMMatrixTranslationFromVector(XMVectorSet(-1.f, 0.f, 1.f, 1.f));

	m_SkyMatrices[ENUM_CLASS(SKYBOX::FIRST_CLOUD)] =
		XMMatrixScaling(4.f, 2.5f, 1.f) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(-45.f), 0.f))
		* XMMatrixTranslationFromVector(XMVectorSet(-1.25f, 0.8f, 1.f, 1.f));

	m_SkyMatrices[ENUM_CLASS(SKYBOX::SEC_CLOUD)] =
		XMMatrixScaling(4.f, 2.5f, 1.f) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(-45.f), 0.f))
		* XMMatrixTranslationFromVector(XMVectorSet(-0.65f, 1.05f, 1.f, 1.f));

	m_SkyMatrices[ENUM_CLASS(SKYBOX::EFFECT)] = 
		XMMatrixScaling(2.f, 1.2f, 1.f) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(-45.f), 0.f))
		* XMMatrixTranslationFromVector(XMVectorSet(-1.f, 0.6f, 1.f, 1.f));

	m_iShaderIndex[ENUM_CLASS(SKYBOX::BACK)] = 0;
	m_iShaderIndex[ENUM_CLASS(SKYBOX::FIRST_CLOUD)] = 1;
	m_iShaderIndex[ENUM_CLASS(SKYBOX::SEC_CLOUD)] = 2;
	m_iShaderIndex[ENUM_CLASS(SKYBOX::EFFECT)] = 3;

	return S_OK;
}

HRESULT CLogo_SkyBox::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	//Env Map Bake
	m_pGameInstance->Add_EnvMap_SkyBox(this);

#ifdef _DEBUG
	m_iIndex = 0;

	m_vScale[ENUM_CLASS(SKYBOX::BACK)] = _float3(5.f, 5.f, 1.f);
	m_vScale[ENUM_CLASS(SKYBOX::FIRST_CLOUD)] = _float3(3.f, 2.f, 1.f);
	m_vScale[ENUM_CLASS(SKYBOX::SEC_CLOUD)] = _float3(3.f, 2.f, 1.f);
	m_vScale[ENUM_CLASS(SKYBOX::EFFECT)] = _float3(2.f, 1.5f, 1.f);

	m_vYawPitchRoll[ENUM_CLASS(SKYBOX::BACK)] = _float3(0.f, -45.f, 0.f);
	m_vYawPitchRoll[ENUM_CLASS(SKYBOX::FIRST_CLOUD)] = _float3(0.f, -45.f, 0.f);
	m_vYawPitchRoll[ENUM_CLASS(SKYBOX::SEC_CLOUD)] = _float3(0.f, -45.f, 0.f);
	m_vYawPitchRoll[ENUM_CLASS(SKYBOX::EFFECT)] = _float3(0.f, -45.f, 0.f);

	m_vPosition[ENUM_CLASS(SKYBOX::BACK)] = _float3(-1.f, 0.f, 1.f);
	m_vPosition[ENUM_CLASS(SKYBOX::FIRST_CLOUD)] = _float3(-1.2f, 0.6f, 1.f);
	m_vPosition[ENUM_CLASS(SKYBOX::SEC_CLOUD)] = _float3(-0.75f, 0.95f, 1.f);
	m_vPosition[ENUM_CLASS(SKYBOX::EFFECT)] = _float3(-1.f, 0.6f, 1.f);

#endif

	return S_OK;
}

void CLogo_SkyBox::Priority_Update(_float fTimeDelta)
{
}

void CLogo_SkyBox::Update(_float fTimeDelta)
{
	//m_pGameInstance->Use_Gizmo_Offset(&m_vScale[m_iIndex], &m_vYawPitchRoll[m_iIndex], &m_vPosition[m_iIndex]);
}

void CLogo_SkyBox::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::PRIORITY, this)))
		return;
}

void CLogo_SkyBox::Render()
{
#ifdef  _DEBUG

	ImGui::Begin("LOGO_SKY");

	if (ImGui::Button("BACK"))
	{
		m_iIndex = 0;

	}

	ImGui::SameLine();

	if (ImGui::Button("FIRST"))
	{
		m_iIndex = 1;
	}

	ImGui::SameLine();
	if (ImGui::Button("SEC"))
	{
		m_iIndex = 2;
	}

	ImGui::SameLine();
	if (ImGui::Button("LAST"))
	{
		m_iIndex = 3;
	}

	//_matrix	WorldMatrix = XMMatrixScaling(m_vScale[m_iIndex].x, m_vScale[m_iIndex].y, m_vScale[m_iIndex].z) *
	//	XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_vYawPitchRoll[m_iIndex].x), XMConvertToRadians(m_vYawPitchRoll[m_iIndex].y), XMConvertToRadians(m_vYawPitchRoll[m_iIndex].z))) *
	//	XMMatrixTranslation(m_vPosition[m_iIndex].x, m_vPosition[m_iIndex].y, m_vPosition[m_iIndex].z);

	//m_SkyMatrices[m_iIndex] = WorldMatrix;
	

	ImGui::End();
#endif //  _DEBUG

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed to Bind ViewMatrix");
	if(FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed to Bind ProjMatrix");
	
	_matrix TranslationMatrix = XMMatrixTranslationFromVector(XMVectorSetW(XMLoadFloat4(m_pGameInstance->Get_CamPos()), 1.f));

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
	{
		XMStoreFloat4x4(&m_CombineMatrix, m_SkyMatrices[i] * TranslationMatrix);

		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_CombineMatrix)))
			CRASH("Failed to Bind WorldMatrix");

		if (FAILED(m_pTexture->Bind_Shader_Resource(m_pShader, "g_Texture", i)))
			CRASH("Failed to Bind Texture");
		
		m_pShader->Begin(m_iShaderIndex[i]);
		
		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}
}

void CLogo_SkyBox::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	_matrix TranslationMatrix = XMMatrixTranslationFromVector(XMVectorSetW(XMLoadFloat4(&vCenter), 1.f));

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
	{
		XMStoreFloat4x4(&m_CombineMatrix, m_SkyMatrices[i] * TranslationMatrix);

		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_CombineMatrix)))
			CRASH("Failed to Bind WorldMatrix");

		if (FAILED(m_pTexture->Bind_Shader_Resource(m_pShader, "g_Texture", i)))
			CRASH("Failed to Bind Texture");

		m_pShader->Begin(m_iShaderIndex[i]);

		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}
}

HRESULT CLogo_SkyBox::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Shader_VtxSkyBox_Logo"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Texture_Logo_Skybox"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTexture), nullptr)))
		ASSERT_CRASH(m_pTexture);

    return S_OK;
}

CLogo_SkyBox* CLogo_SkyBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLogo_SkyBox* pInstance = new CLogo_SkyBox(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLogo_SkyBox");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CLogo_SkyBox::Clone(void* pArg)
{
	CLogo_SkyBox* pInstance = new CLogo_SkyBox(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLogo_SkyBox");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLogo_SkyBox::Free()
{
	__super::Free();

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pTexture);
}
