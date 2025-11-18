#include "EditorPch.h"
#include "SFX_Interface.h"
#include "Augusta_SFX.h"
#include "AugustaBlur.h"

CSFX_Interface::CSFX_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext}
{
}

HRESULT CSFX_Interface::Initialize()
{
	CEdit_ScreenEffect* pAuguSlash = CAugusta_SFX::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(pAuguSlash);

	m_pCurrentSFXs.push_back(pAuguSlash);

	CEdit_ScreenEffect* pAuguBlur = CAugustaBlur::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(pAuguBlur);

	m_pCurrentSFXs.push_back(pAuguBlur);

    return S_OK;
}

void CSFX_Interface::Priority_Update(_float fTimeDelta)
{
	for(auto& SFX : m_pCurrentSFXs)
		SFX->Priority_Update(fTimeDelta);
}

void CSFX_Interface::Update(_float fTimeDelta)
{
	for (auto& SFX : m_pCurrentSFXs)
		SFX->Update(fTimeDelta);
}

void CSFX_Interface::Late_Update(_float fTimeDelta)
{
	for (auto& SFX : m_pCurrentSFXs)
		SFX->Late_Update(fTimeDelta);
}

void CSFX_Interface::Render()
{
	ImGui::Begin("SFX");

	if (ImGui::Button("PLAY"))
	{
		for (auto& SFX : m_pCurrentSFXs)
			SFX->Play();
	}

	if (ImGui::Button("STOP"))
	{
		for (auto& SFX : m_pCurrentSFXs)
			SFX->Stop();
	}
	if (ImGui::Button("Reset"))
	{
		for (auto& SFX : m_pCurrentSFXs)
			SFX->Reset();
	}
	ImGui::End();
}

CSFX_Interface* CSFX_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSFX_Interface* pInstance = new CSFX_Interface(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CSFX_Interface");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CSFX_Interface::Free()
{
	__super::Free();

	for (auto& SFX : m_pCurrentSFXs)
		Safe_Release(SFX);
	m_pCurrentSFXs.clear();
}
