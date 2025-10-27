
#include "EditorPch.h"
#include "Shader_Interface.h"

CShader_Interface::CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext }
{
}

HRESULT CShader_Interface::Initialize()
{
	return S_OK;
}

void CShader_Interface::Update_Shadow()
{
	Set_ShadowBias();
	Setting_LUT();
	Set_SSAO();
}

void CShader_Interface::Set_ShadowBias()
{
	ImGui::Begin("SHADOW BIAS");

	
	if(ImGui::CollapsingHeader("Base Bias"))
	{
		Setting_Bias("BIAS_CASCADE[0]", &m_fBias[0]);
		Setting_Bias("BIAS_CASCADE[1]", &m_fBias[1]);
		Setting_Bias("BIAS_CASCADE[2]", &m_fBias[2]);
		Setting_Bias("BIAS_CASCADE[3]", &m_fBias[3]);
	}

	if (ImGui::CollapsingHeader("Min Bias"))
	{
		Setting_Bias("MIN_BIAS_CASCADE[0]", &m_fMinBias[0]);
		Setting_Bias("MIN_BIAS_CASCADE[1]", &m_fMinBias[1]);
		Setting_Bias("MIN_BIAS_CASCADE[2]", &m_fMinBias[2]);
		Setting_Bias("MIN_BIAS_CASCADE[3]", &m_fMinBias[3]);
	}

	if (ImGui::CollapsingHeader("SLOPE_SCALE"))
	{
		//Setting_Bias("SLOPE_SCALE", &m_fSlopeScale);
		ImGui::InputFloat("SCALE", &m_fSlopeScale);
	}

#ifdef _DEBUG
	m_pGameInstance->Bind_RawValue_Renderer("g_fShadowBais", &m_fBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_fMinShadowBias", &m_fMinBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
#endif // _DEBUG


	
	ImGui::End();
}

void CShader_Interface::Set_SSAO()
{
	ImGui::Begin("SSAO");

	if(ImGui::Button("SSAO_ON"))
		m_pGameInstance->IsSSAO(true);
	
	ImGui::SameLine();

	if (ImGui::Button("SSAO_OFF"))
		m_pGameInstance->IsSSAO(false);

	if (ImGui::Button("BLUR_ON"))
		m_pGameInstance->IsSSAO_Blur(true);

	ImGui::SameLine();

	if (ImGui::Button("BLUR_OFF"))
		m_pGameInstance->IsSSAO_Blur(false);


	ImGui::InputFloat("RADIUS", &m_fRadius);
	
	ImGui::DragFloat("MAX_DISTANCE", &m_fMaxDistance, 1.f, 1.f, 50.f, "%.1f");


	ImGui::DragFloat("DEPTH_SIGMA", &m_fSigmaWeight, 0.0001f, 0.0001f, 50.f, "%.5f");

	ImGui::InputFloat("MIN_DEPTH", &m_fMinDepthWeight);

	m_pGameInstance->Bind_RawValue_Renderer("g_fSSAO_Radius", &m_fRadius, sizeof(_float));
	m_pGameInstance->Bind_RawValue_Renderer("g_fSSAO_MaxDistance", &m_fMaxDistance, sizeof(_float));
	m_pGameInstance->Bind_RawValue_Renderer("g_fDepthSigma", &m_fSigmaWeight, sizeof(_float));
	m_pGameInstance->Bind_RawValue_Renderer("g_fMinDepthDistance", &m_fMinDepthWeight, sizeof(_float));
	ImGui::End();
}

void CShader_Interface::Setting_Bias(const _char* pName, _float* pFloat)
{
	ImGui::PushItemWidth(250.f);
	ImGui::DragFloat(pName, pFloat, 0.001f, 0.001f, 0.5f);

//	ImGui::SliderFloat(pName, pFloat, 0.001f, 0.5f);
	ImGui::PopItemWidth();

	//ImGui::SameLine();

	//ImGui::PushItemWidth(120.f);

	//ImGui::InputFloat("", pFloat, 0.001f, 0.01f);
	//ImGui::PopItemWidth();
}

void CShader_Interface::Setting_LUT()
{
	ImGui::Begin("LUT");

	if (ImGui::BeginCombo("LUT_INDEX", "LUT"))
	{
		for (_uint i = 0; i < 5; ++i)
		{

#ifdef _DEBUG
			if (ImGui::Selectable(to_string(i).c_str()))
			{
				m_iLUT_Index = i;
				m_pGameInstance->Set_LUT_Index(m_iLUT_Index);
			}
#endif // _DEBUG

			
		}

		ImGui::EndCombo();
	}

	if (ImGui::CollapsingHeader("LUT_LERP_INTENSITY"))
	{
		ImGui::PushItemWidth(250.f);
		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.01f, 1.f);

		ImGui::PopItemWidth();

	}

#ifdef _DEBUG
	m_pGameInstance->Bind_RawValue_Renderer("g_fLutLerpIntensity", &m_fLUT_Intensity, sizeof(_float));
#endif // _DEBUG


	
	ImGui::End();

}

CShader_Interface* CShader_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShader_Interface* pInstance = new CShader_Interface(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CShader_Interface");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CShader_Interface::Free()
{
	__super::Free();

}
