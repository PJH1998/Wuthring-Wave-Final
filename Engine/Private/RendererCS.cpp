#include "EnginePch.h"
#include "RendererCS.h"
#include "ComputeShader.h"

CRendererCS::CRendererCS(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CRendererCS::Add_BufferData(const _char* pConstantName, void* pData, _uint iLength)
{
	BUFFER_DATA* pBufferData = Find_Buffer(pConstantName);
	if (nullptr == pBufferData)
	{
		ID3D11Buffer* pBuffer = { nullptr };
		if (FAILED(Ready_Buffer(&pBuffer, iLength)))
			return;

		D3D11_MAPPED_SUBRESOURCE SubResource;
		m_pContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
		memcpy(SubResource.pData, pData, iLength);
		m_pContext->Unmap(pBuffer, 0);
		
		BUFFER_DATA Data = make_pair(iLength, pBuffer);

		m_Buffers.emplace(pConstantName, Data);
	}
	else
	{
		if (pBufferData->first != iLength)
			CRASH("Wrong Input Length");

		D3D11_MAPPED_SUBRESOURCE SubResource;
		m_pContext->Map(pBufferData->second, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
		memcpy(SubResource.pData, pData, iLength);
		m_pContext->Unmap(pBufferData->second, 0);
	}
}

void CRendererCS::Add_SRVData(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	if (nullptr == pSRV)
		CRASH("Failed Set SRV_DATA");

	SRV_DATA Data = make_pair(pConstantName, pSRV);

	Safe_AddRef(pSRV);

	m_SRVs.push_back(Data);
}

HRESULT CRendererCS::Initialize(void* pDesc)
{
	ASSERT_CRASH(pDesc);

	RCS_DESC* pRCS_Desc = static_cast<RCS_DESC*>( pDesc );

	m_pComputeShader = CComputeShader::Create(m_pDevice, m_pContext, pRCS_Desc->pFilePath, pRCS_Desc->eShaderMacro, pRCS_Desc->strEntryPoint);
	ASSERT_CRASH(m_pComputeShader);

	m_fWidht = static_cast<_float>( pRCS_Desc->iWidth);
	m_fHeight = static_cast<_float>( pRCS_Desc->iHeight );
	m_fDefinitionX = pRCS_Desc->fDefinitionX;
	m_fDefinitionY = pRCS_Desc->fDefinitionY;
	m_vClearColor = pRCS_Desc->vClearColor;

	if (FAILED(Ready_BindTexture(pRCS_Desc->iWidth, pRCS_Desc->iHeight, pRCS_Desc->eFormat)))
		CRASH("Failed Ready Textures");

	return S_OK;
}

void CRendererCS::Bind_Resources()
{
	for (auto& Pair : m_Buffers)
		m_pComputeShader->Set_ConstantBuffer(Pair.first, Pair.second.second);
	
	for (auto& SRV : m_SRVs)
		m_pComputeShader->Set_SRV(SRV.first, SRV.second);

	m_pComputeShader->Set_UAV(m_UAV.first, m_UAV.second);
}

void CRendererCS::Dispatch()
{
	m_pComputeShader->Dispatch(static_cast<_uint>((m_fWidht + m_fDefinitionX -1.f) / m_fDefinitionX) , static_cast<_uint>(( m_fHeight + m_fDefinitionY - 1.f ) / m_fDefinitionY), 1);

	Clear_Resource();
}

void CRendererCS::Clear()
{
	m_pContext->ClearUnorderedAccessViewFloat(m_UAV.second, reinterpret_cast<_float*>( &m_vClearColor ));
}

void CRendererCS::Clear_Resource()
{
	m_Buffers.clear();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV.second);
	m_SRVs.clear();
}

#ifdef _DEBUG
HRESULT CRendererCS::Debug_Render(const _wstring& strRCS_Name)
{
	ImGui::Begin(WStringToString(strRCS_Name).c_str());

	ImGui::Image(reinterpret_cast<ImTextureID>( m_pComputeSRV ), ImVec2(500.f, 500.f));

	ImGui::End();

	return S_OK;
}
#endif

HRESULT CRendererCS::Ready_Buffer(ID3D11Buffer** ppOut, _uint iLength)
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = iLength;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, ppOut)))
		CRASH("Failed to Create : Constant Buffer");

	return S_OK;
}

HRESULT CRendererCS::Ready_BindTexture(_uint iWidth, _uint iHeight, DXGI_FORMAT eFormat)
{
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = eFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc{};

	UAVDesc.Format = TextureDesc.Format;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	UAVDesc.Texture2D.MipSlice = 0;
	
	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pTexture2D, &UAVDesc, &m_UAV.second)))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture2D, &SRVDesc, &m_pComputeSRV)))
		return E_FAIL;

	return S_OK;
}

CRendererCS::BUFFER_DATA* CRendererCS::Find_Buffer(const _char* pConstantName)
{
	auto iter = m_Buffers.find(pConstantName);
	if (iter != m_Buffers.end())
		return &(iter->second);
	return nullptr;
}

CRendererCS* CRendererCS::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pDesc)
{
	CRendererCS* pInstance = new CRendererCS(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pDesc)))
	{
		MSG_BOX("Failed to Created : CRendererCS");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRendererCS::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pComputeShader);

	for (auto& Pair : m_Buffers)
		Safe_Release(Pair.second.second);
	m_Buffers.clear();

	for (auto& SRV : m_SRVs)
		Safe_Release(SRV.second);
	m_SRVs.clear();

	Safe_Release(m_pComputeShader);
	Safe_Release(m_pTexture2D);
	Safe_Release(m_pComputeSRV);
	Safe_Release(m_UAV.second);
}
