#include "EnginePch.h"
#include "HZB.h"

#include "GameInstance.h"

CHZB::CHZB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CHZB::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	// Texture2D Desc
	D3D11_TEXTURE2D_DESC TextureDesc = {};

	TextureDesc.Width = m_iWinSizeX >> 1;
	TextureDesc.Height = m_iWinSizeY >> 1;
	TextureDesc.MipLevels = MAX_MIPLEVEL;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_FLOAT;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	// Create Texture2D
	ID3D11Texture2D* pTexture = { nullptr };

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture)))
		CRASH("Texture2D");

	// Create UAVs
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.Format = TextureDesc.Format;
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = i;

		if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pUAV[i])))
			CRASH("HZB UAV");

		// Create SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = TextureDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = i;
		SRVDesc.Texture2D.MipLevels = 1;

		if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pSRV[i])))
			CRASH("HZB SRV");
	}

	// Create MMSRV
	D3D11_SHADER_RESOURCE_VIEW_DESC MMSRVDesc = {};
	MMSRVDesc.Format = TextureDesc.Format;
	MMSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	MMSRVDesc.Texture2D.MostDetailedMip = 0;
	MMSRVDesc.Texture2D.MipLevels = MAX_MIPLEVEL;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &MMSRVDesc, &m_pMMSRV)))
		CRASH("HZB MMSRV");

	Safe_Release(pTexture);

	// Create CS
	SHADER_MACRO ShaderMacro = {
		{ "THREAD_X", "8" },
		{ "THREAD_Y", "8" },
		{ "THREAD_Z", "1" },
		{ nullptr, nullptr }
	};
	m_pComputeShader = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_HZB.hlsl"),
		ShaderMacro, "HZB");
	ASSERT_CRASH(m_pComputeShader);

    return S_OK;
}

void CHZB::Update()
{
	_float4 vClearColor = _float4(0.f, 0.f, 0.f, 0.f);
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
	{
		m_pContext->ClearUnorderedAccessViewFloat(m_pUAV[i], reinterpret_cast<_float*>(&vClearColor));

		ID3D11ShaderResourceView* pSRV = 0 == i ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")) : m_pSRV[i - 1];

		0 == i ? m_pComputeShader->Set_SRV("InputTexture", pSRV) : m_pComputeShader->Set_SRV("InputMipTexture", pSRV);
	
		m_pComputeShader->Set_UAV("OutputTexture", m_pUAV[i]);

		_uint iSizeX = max(m_iWinSizeX >> (i + 1), 1);
		_uint iSizeY = max(m_iWinSizeY >> (i + 1) , 1);

		_uint iThreadGroupX = (iSizeX + 7) / 8;
		_uint iThreadGroupY = (iSizeY + 7) / 8;

		m_pComputeShader->Dispatch(iThreadGroupX, iThreadGroupY, 1);
	}
}
#ifdef _DEBUG
void CHZB::Render()
{
	ImGui::Begin("HZB_RENDER");

	if (ImGui::BeginCombo("HZB", "List"))
	{
		for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
		{
			_string strHZB = "HZB";
			strHZB += to_string(i);
			
			if (ImGui::Selectable(strHZB.c_str()))
			{
				AddRemoveHZB(strHZB, reinterpret_cast<ImTextureID>(m_pSRV[i]));
			}
		}

		ImGui::EndCombo();
	}
	ImGui::End();

	for (auto& Pair : m_RenderTextures)
	{
		ImGui::Begin(Pair.first.c_str());
		ImGui::Image(Pair.second, ImVec2(500.f, 500.f));
		ImGui::End();
	}
	
}
void CHZB::AddRemoveHZB(const _string& strHZB, ImTextureID TextureID)
{
	auto iter = m_RenderTextures.find(strHZB);
	if (iter != m_RenderTextures.end())
	{
		m_RenderTextures.erase(iter);
		return;
	}

	m_RenderTextures.emplace(strHZB, TextureID);
	
}
#endif

CHZB* CHZB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CHZB* pInstance = new CHZB(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
		CRASH("HZB");

    return pInstance;
}

void CHZB::Free()
{
	__super::Free();

	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
		Safe_Release(m_pUAV[i]);

	Safe_Release(m_pMMSRV);

	Safe_Release(m_pComputeShader);

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
