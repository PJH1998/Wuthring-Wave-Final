#include "ClientPch.h"
#include "Excute_PostSFX.h"

CExcute_PostSFX::CExcute_PostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect { pDevice, pContext }
{
}

CExcute_PostSFX::CExcute_PostSFX(const CExcute_PostSFX& Prototype)
	: CScreenEffect{ Prototype }
	, m_iLUT_Index{ Prototype.m_iLUT_Index }
	, m_fIntensityTIme{ Prototype.m_fIntensityTIme }
{
}

HRESULT CExcute_PostSFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_iLUT_Index = 3;

	m_fIntensityTIme = 0.4f;
	m_vEffectTime = _float2(0.f, 0.7f);

	return S_OK;
}

HRESULT CExcute_PostSFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_SlashData.vScreenSize = _float2(static_cast<_float>(g_iWinSizeX), static_cast<_float>(g_iWinSizeY));
	m_SlashData.vSlashPoint0 = _float2(static_cast<_float>(g_iWinSizeX), 85.f);
	m_SlashData.vSlashPoint1 = _float2(0.f, 780.f);
	m_SlashData.fOffset = 350.f;
	m_SlashData.fIntensity = 1.f;

	if (FAILED(Ready_Buffer()))
		return E_FAIL;

	Setting_Scale(m_vWinSize.x, m_vWinSize.y);
	Setting_Pos(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.5f);

	return S_OK;
}

void CExcute_PostSFX::Priority_Update(_float fTimeDelta)
{

}

void CExcute_PostSFX::Update(_float fTimeDelta)
{
	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		m_pGameInstance->Setting_LUT(m_iPrevLutIndex, m_fPrevLutIntensity, m_PrevIsDynamicLUT);
		return;
	}

	m_fCurrentTime += fTimeDelta;

	if (m_fCurrentTime >= m_fIntensityTIme)
	{
		m_fCurrentLUTIntensity = max(m_fCurrentLUTIntensity - (fTimeDelta * 2.5f), m_fPrevLutIntensity);

		m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fCurrentLUTIntensity, true);
	}

	m_SlashData.fIntensity = (1.f - (m_fCurrentTime / m_vEffectTime.y));

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_SlashData, sizeof(SFX_SLASH_DATA));
	m_pContext->Unmap(m_pBuffer, 0);
}

void CExcute_PostSFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::POST_SFX, this)))
		return;
}

void CExcute_PostSFX::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Texture("g_SceneTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed to Bind SceneTexture");

	m_pShader->Begin(ENUM_CLASS(SHADER_SFX_BURST::EXCUTE));

	m_pContext->PSSetConstantBuffers(1, 1, &m_pBuffer);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CExcute_PostSFX::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;

	m_pGameInstance->Get_Current_LutSetting(&m_iPrevLutIndex, &m_fPrevLutIntensity, &m_PrevIsDynamicLUT);

	m_fCurrentLUTIntensity = 1.f;

	m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fCurrentLUTIntensity, true);

	m_pGameInstance->Setting_Radial(_float2(0.5f, 0.5f), _float2(0.f, 0.5f), -1.f);
	m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL, 0.4f);

}

HRESULT CExcute_PostSFX::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

HRESULT CExcute_PostSFX::Ready_Buffer()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(SFX_RADIAL_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Create : Constant Buffer");

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_SlashData, sizeof(SFX_SLASH_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	return S_OK;
}

CExcute_PostSFX* CExcute_PostSFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CExcute_PostSFX* pInstance = new CExcute_PostSFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CExcute_PostSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CExcute_PostSFX::Clone(void* pArg)
{
	CExcute_PostSFX* pInstance = new CExcute_PostSFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CExcute_PostSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CExcute_PostSFX::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
	Safe_Release(m_pBuffer);
}
