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


	if (FAILED(Ready_FroxelVolume()))
		return E_FAIL;

	if (FAILED(Ready_ComputeShader()))
		return E_FAIL;

    return S_OK;
}

HRESULT CVolumetricFog::Ready_FroxelVolume()
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

	if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pVF_UAV)))
		CRASH("Failed to Created FV_UAV");

	//SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	SRVDesc.Texture3D.MostDetailedMip = 0;
	SRVDesc.Texture3D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pVF_SRV)))
		CRASH("Failed to Created FV_SRV");

	Safe_Release(pTexture);

	//BUFFER
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(VF_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pVF_Buffer)))
		CRASH("ShadowMap Constant Buffer");

	//BUFFER SETTING
	VF_DATA Data = {};
	ZeroMemory(&Data, sizeof(VF_DATA));


	D3D11_MAPPED_SUBRESOURCE SubResource;
	m_pContext->Map(m_pVF_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, reinterpret_cast<void*>(&Data), sizeof(VF_DATA));
	m_pContext->Unmap(m_pVF_Buffer, 0);

	return S_OK;

    return S_OK;
}

HRESULT CVolumetricFog::Ready_ComputeShader()
{
	SHADER_MACRO Macro = { { "THREAD_X", "8" }, { "THREAD_Y", "8" }, { "THREAD_Z", "4" }, { NULL, NULL } };

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
	Safe_Release(m_pVF_SRV);
	Safe_Release(m_pVF_UAV);
	Safe_Release(m_pVF_Buffer);
}
