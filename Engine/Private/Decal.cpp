#include "EnginePch.h"
#include "Decal.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CDecal::Initialize(CTexture* pTexture, _uint iMaxDecal)
{
	ASSERT_CRASH(pTexture);

	m_pDecalTexture = pTexture;
	m_iMaxDecal = iMaxDecal;

    return S_OK;
}

void CDecal::Update(_float fTimeDelta)
{
	if (m_DecalDatas.empty())
		return;

	for (auto iter = m_DecalDatas.begin(); iter != m_DecalDatas.end();)
	{
		(iter)->vLifeTime.x += fTimeDelta;
		if ((iter)->vLifeTime.x >= (iter)->vLifeTime.y)
			m_DecalDatas.erase(iter);
		else
			++iter;
	}

	m_iNumDecals = m_DecalDatas.size();
	Update_Buffer();
}

HRESULT CDecal::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (FAILED(pShader->Bind_Value("g_iNumDecals", &m_iNumDecals, sizeof(_uint))))
		CRASH("Failed Bind Num Decals");

//	pShader->Bind_Texture("아마도", m_pSRV);
//	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DECAL));
	
	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

    return S_OK;
}

void CDecal::Add_Decal(const DECAL_DESC& DecalDesc)
{
	DECAL_DATA Data = {};

	_matrix WorldMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&DecalDesc.vScale)) *
		XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&DecalDesc.vRotation)) *
		XMMatrixTranslationFromVector(XMLoadFloat3(&DecalDesc.vPosition));

	XMStoreFloat4x4(&Data.WorldMatrixInv, XMMatrixInverse(nullptr, WorldMatrix));
	Data.vLifeTime = _float2(0.f, DecalDesc.fLifeTime);

	m_DecalDatas.push_back(Data);
}

HRESULT CDecal::Ready_Buffers()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(DECAL_DATA) * m_iMaxDecal;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(DECAL_DATA);
	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Created Buffer");
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pBuffer, nullptr, &m_pSRV)))
		CRASH("Failed to Created SRV");

    return S_OK;
}

void CDecal::Update_Buffer()
{
	D3D11_MAPPED_SUBRESOURCE SubResource;
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_DecalDatas, sizeof(DECAL_DATA) * m_iNumDecals);
	m_pContext->Unmap(m_pBuffer, 0);
}

CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTexture* pTexture, _uint iMaxDecal)
{
	CDecal* pInstance = new CDecal(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pTexture, iMaxDecal)))
	{
		MSG_BOX("Failed to Created : CDecal");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CDecal::Free()
{
	__super::Free();

	Safe_Release(m_pDecalTexture);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pBuffer);
	Safe_Release(m_pSRV);
}
