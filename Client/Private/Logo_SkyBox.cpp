#include "ClientPch.h"
#include "Logo_SkyBox.h"

CLogo_SkyBox::CLogo_SkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLogo_SkyBox::CLogo_SkyBox(const CLogo_SkyBox& Prototype)
	: CGameObject{ Prototype }
	, m_vPosition { Prototype.m_vPosition }
	, m_vRotateQuaternion { Prototype.m_vRotateQuaternion }
	, m_vScale { Prototype.m_vScale }
{
}

HRESULT CLogo_SkyBox::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLogo_SkyBox::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	_matrix WorldMatrix = XMMatrixScaling(m_vScale.x, m_vScale.y, 1.f) * XMMatrixRotationQuaternion(m_vRotateQuaternion) * XMMatrixTranslationFromVector(m_vPosition);

	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CLogo_SkyBox::Priority_Update(_float fTimeDelta)
{
}

void CLogo_SkyBox::Update(_float fTimeDelta)
{
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

	if (ImGui::CollapsingHeader("ROTATE"))
	{
		ImGui::InputFloat("YAW", &m_vYawPitchRoll.x);
		ImGui::InputFloat("PITCH", &m_vYawPitchRoll.y);
		ImGui::InputFloat("ROLL", &m_vYawPitchRoll.z);

		m_vRotateQuaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_vYawPitchRoll.x), XMConvertToRadians(m_vYawPitchRoll.y), XMConvertToRadians(m_vYawPitchRoll.z));
	}

	if (ImGui::CollapsingHeader("POSITION"))
	{
		ImGui::InputFloat("X", &m_vDebugPosition.x);
		ImGui::InputFloat("Y", &m_vDebugPosition.y);
		ImGui::InputFloat("Z", &m_vDebugPosition.z);

		m_vPosition = XMVectorSet(m_vDebugPosition.x, m_vDebugPosition.y, m_vDebugPosition.z, 1.f);
	}

	if (ImGui::CollapsingHeader("SCALE"))
	{
		ImGui::InputFloat("SCALE_X", &m_vScale.x);
		ImGui::InputFloat("SCALE_Y", &m_vScale.y);
	}

	_matrix WorldMatrix = XMMatrixScaling(m_vScale.x, m_vScale.y, 1.f) * XMMatrixRotationQuaternion(m_vRotateQuaternion) * XMMatrixTranslationFromVector(m_vPosition);

	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	ImGui::End();
#endif //  _DEBUG


	m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix");
	m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
	{
		if (FAILED(m_pSkyTextures[i]->Bind_Shader_Resource(m_pShader, "g_DiffuseTexture")))
			CRASH("Failed to Bind DiffuseTexture");

		m_pShader->Begin(0);

		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}
}

void CLogo_SkyBox::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

HRESULT CLogo_SkyBox::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPosTex_SkyBox"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Texture_LogoSky_Back"), 
		TEXT("Com_Texture_Back"), reinterpret_cast<CComponent**>(&m_pSkyTextures[ENUM_CLASS(SKYBOX::BACK)]), nullptr)))
		ASSERT_CRASH(m_pSkyTextures[ENUM_CLASS(SKYBOX::BACK)]);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Texture_LogoSky_Mid"),
		TEXT("Com_Texture_Mid"), reinterpret_cast<CComponent**>(&m_pSkyTextures[ENUM_CLASS(SKYBOX::MID)]), nullptr)))
		ASSERT_CRASH(m_pSkyTextures[ENUM_CLASS(SKYBOX::MID)]);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Texture_LogoSky_Front"),
		TEXT("Com_Texture_Front"), reinterpret_cast<CComponent**>(&m_pSkyTextures[ENUM_CLASS(SKYBOX::FRONT)]), nullptr)))
		ASSERT_CRASH(m_pSkyTextures[ENUM_CLASS(SKYBOX::FRONT)]);


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

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);

	for (_uint i = 0; i < ENUM_CLASS(SKYBOX::END); ++i)
		Safe_Release(m_pSkyTextures[i]);
}
