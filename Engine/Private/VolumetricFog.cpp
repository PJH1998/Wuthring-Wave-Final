#include "EnginePch.h"
#include "VolumetricFog.h"
#include "Gameinstance.h"
#include "ComputeShader.h"

CVolumetricFog::CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVolumetricFog::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_vFroxelSize.x = iWinSizeX >> 3; // 3 = DownSacle Factor
	m_vFroxelSize.y = iWinSizeY >> 3;
	m_vFroxelSize.z = 128;				// 64~128

	m_vDefinition.x = 8;
	m_vDefinition.y = 8;
	m_vDefinition.z = 4;

	m_iMaxLight = 32;


	m_VF_Data.fWinSizeX = static_cast<_float>(iWinSizeX);
	m_VF_Data.fWinSizeY = static_cast<_float>(iWinSizeY);

	m_VF_Data.vFroxelSize = m_vFroxelSize;
	m_VF_Data.iSliceCount = 128;

	if (FAILED(Ready_FroxelVolume()))
		return E_FAIL;

	if (FAILED(Ready_ComputeShader()))
		return E_FAIL;

    return S_OK;
}

HRESULT CVolumetricFog::SetUp_FogNF()
{
	_float fFar = m_pGameInstance->Get_CurrentCamera_Far();

	m_vFogRange.x = fFar * 0.3f;
	m_vFogRange.y = fFar;

	m_VF_Data.fNear = m_vFogRange.x;
	m_VF_Data.fFar = m_vFogRange.y;

	m_VF_Data.fCamNear = m_pGameInstance->Get_CurrentCamera_Near();
	m_VF_Data.fCamFar = m_pGameInstance->Get_CurrentCamera_Far();

	return S_OK;
}

void CVolumetricFog::Add_LightData(const VF_LIGHT& LightData)
{
	if (m_LightDatas.size() >= m_iMaxLight)
		return;

	m_LightDatas.push_back(LightData);
}

void CVolumetricFog::Render()
{
	Update_Buffer();

	m_pCS->Set_ConstantBuffer("VF_Data", m_pBuffers[ENUM_CLASS(BUFFER::DATA)]);
	m_pCS->Set_SRV("g_LightDatas", m_pSRVs[ENUM_CLASS(SRV::LIGHT)]);
	m_pCS->Set_SRV("g_DepthTexture", m_pGameInstance->Get_HZB_Resource());
	m_pCS->Set_UAV("OutputTexture", m_pUAV);

	_uint iThreadGroupCountX = static_cast<_uint>((m_vFroxelSize.x + m_vDefinition.x) / m_vFroxelSize.x);
	_uint iThreadGroupCountY = static_cast<_uint>((m_vFroxelSize.y + m_vDefinition.y) / m_vFroxelSize.y);
	_uint iThreadGroupCountZ = static_cast<_uint>((m_vFroxelSize.z + m_vDefinition.z) / m_vFroxelSize.z);

	m_pCS->Dispatch(iThreadGroupCountX, iThreadGroupCountY, iThreadGroupCountZ);


	m_LightDatas.clear();
}

void CVolumetricFog::Update_Buffer()
{
	_uint iLightCount = static_cast<_uint>(m_LightDatas.size());

	m_VF_Data.ViewMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);
	m_VF_Data.ProjMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);
	m_VF_Data.InvViewMatrix = *m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW);
	m_VF_Data.InvProjMatrix = *m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW);
	m_VF_Data.iLightCount = iLightCount;

	//VF_DATA UPDATE
	D3D11_MAPPED_SUBRESOURCE VF_SubResource = {};
	m_pContext->Map(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0, D3D11_MAP_WRITE_DISCARD, 0, &VF_SubResource);
	memcpy(VF_SubResource.pData, &m_VF_Data, sizeof(VF_DATA));
	m_pContext->Unmap(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0);

	//LIGHT_DATA UPDATE
	if(iLightCount > 0)
	{
		D3D11_BOX Box = { 0, 0, 0, max(sizeof(VF_LIGHT) * iLightCount, 1), 1, 1 };
		m_pContext->UpdateSubresource(m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)], 0, &Box, m_LightDatas.data(), 0, 0);
	}
}

HRESULT CVolumetricFog::Ready_FroxelVolume()
{
	if (FAILED(Ready_Texture()))
		return E_FAIL;

	if (FAILED(Ready_Buffer()))
		return E_FAIL;
	
    return S_OK;
}

HRESULT CVolumetricFog::Ready_Texture()
{
	//Texture
	D3D11_TEXTURE3D_DESC TextureDesc = {};
	TextureDesc.Width = m_vFroxelSize.x;
	TextureDesc.Height = m_vFroxelSize.y;
	TextureDesc.Depth = m_vFroxelSize.z;
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture3D* pTexture = { nullptr };
	if (FAILED(m_pDevice->CreateTexture3D(&TextureDesc, nullptr, &pTexture)))
		CRASH("Failed to Created Texture3D");

	//UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.Format = TextureDesc.Format;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	UAVDesc.Texture3D.FirstWSlice = 0;
	UAVDesc.Texture3D.MipSlice = 0;
	UAVDesc.Texture3D.WSize = TextureDesc.Depth;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pUAV)))
		CRASH("Failed to Created FV_UAV");

	//SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	SRVDesc.Texture3D.MostDetailedMip = 0;
	SRVDesc.Texture3D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pSRVs[ENUM_CLASS(SRV::VF)])))
		CRASH("Failed to Created FV_SRV");

	Safe_Release(pTexture);

	return S_OK;
}

HRESULT CVolumetricFog::Ready_Buffer()
{
	//CONSTANT BUFFER
	D3D11_BUFFER_DESC CBufferDesc = {};
	CBufferDesc.ByteWidth = sizeof(VF_DATA);
	CBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&CBufferDesc, nullptr, &m_pBuffers[ENUM_CLASS(BUFFER::DATA)])))
		CRASH("Failed to Created VF_CBuffer");

	//CONSTANT BUFFER SETTING
	VF_DATA Data = {};
	ZeroMemory(&Data, sizeof(VF_DATA));

	D3D11_MAPPED_SUBRESOURCE SubResource;
	m_pContext->Map(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, reinterpret_cast<void*>(&Data), sizeof(VF_DATA));
	m_pContext->Unmap(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0);

	//STRUCTURED BUFFER
	D3D11_BUFFER_DESC StructureBufferDesc = {};
	StructureBufferDesc.ByteWidth = sizeof(VF_LIGHT) * m_iMaxLight;
	StructureBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	StructureBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	StructureBufferDesc.StructureByteStride = sizeof(VF_LIGHT);
	StructureBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	StructureBufferDesc.CPUAccessFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&StructureBufferDesc, nullptr, &m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)])))
		CRASH("Failed to Created VF_LightBuffer");

	//SBUFFER SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SBuffer_SRVDesc = {};
	SBuffer_SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SBuffer_SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SBuffer_SRVDesc.Buffer.FirstElement = 0;
	SBuffer_SRVDesc.Buffer.NumElements = m_iMaxLight;
	
	if(FAILED(m_pDevice->CreateShaderResourceView(m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)], &SBuffer_SRVDesc, &m_pSRVs[ENUM_CLASS(SRV::LIGHT)])))
		CRASH("Failed to Created Buffer_SRV");

	return S_OK;
}

HRESULT CVolumetricFog::Ready_ComputeShader()
{
	SHADER_MACRO Macro = { { "THREAD_X", "8" }, { "THREAD_Y", "8" }, { "THREAD_Z", "8" }, { NULL, NULL } };

	m_pCS = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_VF.hlsl"), Macro, "VolumetricFog");
	ASSERT_CRASH(m_pCS);

	return S_OK;
}

CVolumetricFog* CVolumetricFog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY)
{
	CVolumetricFog* pInstance = new CVolumetricFog(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CVolumetricFog");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CVolumetricFog::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pCS);

	Safe_Release(m_pUAV);
	
	for(_uint i=0; i< ENUM_CLASS(SRV::END); ++i)
		Safe_Release(m_pSRVs[i]);

	for (_uint j = 0; j < ENUM_CLASS(BUFFER::END); ++j)
		Safe_Release(m_pBuffers[j]);
}
