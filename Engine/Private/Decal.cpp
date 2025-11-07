#include "EnginePch.h"
#include "Decal.h"
#include "Texture.h"
#include "VIBuffer_Decal.h"
#include "Shader.h"
#include "GameInstance.h"

CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CDecal::Initialize()
{
	m_pVIBuffer_Decal = CVIBuffer_Decal::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pVIBuffer_Decal);

	return S_OK;
}

void CDecal::Update(_float fTimeDelta)
{
	for (auto iter = m_DecalDatas.begin(); iter != m_DecalDatas.end();)
	{
		iter->vLifeTime.x += fTimeDelta;

		if (iter->vLifeTime.x >= iter->vLifeTime.y)
		{
			iter = m_DecalDatas.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	m_iNumDecals = static_cast<_uint>(m_DecalDatas.size());
	
	if (m_iNumDecals > 0)
		m_pVIBuffer_Decal->Update_Buffer(m_DecalDatas);
	else
		m_pVIBuffer_Decal->Clear();
}

void CDecal::Render(CShader* pShader)
{
	if (m_iNumDecals < 0 || m_iNumDecals >= g_iMaxDecal)
		return;

	Bind_Resources(pShader);

	pShader->Begin(0);

	m_pVIBuffer_Decal->Bind_Resources();
	m_pVIBuffer_Decal->Render();
}

HRESULT CDecal::Add_DecalTexture(const _tchar* pFilePath, TEXTURETYPE eTextureType)
{
	if (nullptr != m_pDecalTexture[ENUM_CLASS(eTextureType)])
		return E_FAIL;

	CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, pFilePath, 1);
	ASSERT_CRASH(pTexture);

	m_pDecalTexture[ENUM_CLASS(eTextureType)] = pTexture;

    return S_OK;
}

HRESULT CDecal::Add_DecalData(const DECAL_DESC& Decal)
{
	if (m_iNumDecals >= g_iMaxDecal)
		return E_FAIL;

	_matrix WorldInv = XMMatrixInverse(nullptr, Decal.WorldMatrix);

	VTXINSTANCE_DECAL Data = {};

	memcpy(&Data.vRight, &Decal.WorldMatrix, sizeof(_matrix));
	memcpy(&Data.vRightInv, &WorldInv, sizeof(_matrix));

	Data.vLifeTime = _float2(0.f, Decal.fLifeTime);
	Data.vColor = Decal.vColor;

	m_DecalDatas.push_back(Data);

	return S_OK;
}

ID3D11ShaderResourceView* CDecal::Get_DecalSRV(TEXTURETYPE eTextureType)
{
	if(nullptr == m_pDecalTexture[ENUM_CLASS(eTextureType)])
		return nullptr;

	return m_pDecalTexture[ENUM_CLASS(eTextureType)]->Get_SRV(0);
}

HRESULT CDecal::Bind_Resources(CShader* pShader)
{
	if (nullptr == pShader)
		return E_FAIL;
	
	if (m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::DIFFUSE)])
	{
		_bool HasDiffuse = false;
		if (SUCCEEDED(m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::DIFFUSE)]->Bind_Shader_Resource(pShader, "g_DiffuseTexture")))
			HasDiffuse = true;

		if (FAILED(pShader->Bind_Value("g_HasDiffuse", &HasDiffuse, sizeof(_bool))))
			CRASH("Failed Bind Value");
	}

	if (m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::NORMAL)])
	{
		_bool HasNormal = false;
		if (SUCCEEDED(m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::NORMAL)]->Bind_Shader_Resource(pShader, "g_NormalTexture")))
			HasNormal = true;

		if (FAILED(pShader->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Failed Bind Value");
	}

	if (m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::MASK)])
	{
		_bool HasMask = false;
		if (SUCCEEDED(m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::MASK)]->Bind_Shader_Resource(pShader, "g_MaskTexture")))
			HasMask = true;

		if (FAILED(pShader->Bind_Value("g_HasMask", &HasMask, sizeof(_bool))))
			CRASH("Failed Bind Value");
	}
	
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_Depth");

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");

	return S_OK;
}

CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDecal* pInstance = new CDecal(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDecal::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pVIBuffer_Decal);

	for (_uint i = 0; i < ENUM_CLASS(TEXTURETYPE::END); ++i)
		Safe_Release(m_pDecalTexture[i]);
}
