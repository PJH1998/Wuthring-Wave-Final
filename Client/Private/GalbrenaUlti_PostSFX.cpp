#include "ClientPch.h"
#include "GalbrenaUlti_PostSFX.h"

CGalbrenaUlti_PostSFX::CGalbrenaUlti_PostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect{ pDevice, pContext }

{
}

CGalbrenaUlti_PostSFX::CGalbrenaUlti_PostSFX(const CGalbrenaUlti_PostSFX& Prototype)
	: CScreenEffect{ Prototype }
	, m_RadialData{ Prototype.m_RadialData }
	, m_vLengthScale { Prototype.m_vLengthScale }
	, m_vMaxDistance { Prototype.m_vMaxDistance }
	, m_vRadialTime { Prototype.m_vRadialTime }
	, m_iLUT_Index { Prototype.m_iLUT_Index }
	, m_vLutIntensity { Prototype.m_vLutIntensity }
{
	for (_uint i = 0; i < 2; ++i)
		m_vReverseTime[i] = Prototype.m_vReverseTime[i];
}

HRESULT CGalbrenaUlti_PostSFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vMaxDistance = _float2(1.f, 0.2f);
	m_vLengthScale = _float2(0.f, -0.2f);

	m_vEffectTime = _float2(0.f, 0.7f);

	m_vReverseTime[0] = _float2(0.1f, 0.2f);
	m_vReverseTime[1] = _float2(0.4f, 0.5f);

	m_vRadialTime = _float2(0.3f, m_vEffectTime.y);

	m_RadialData.fMinDistance = 0.f;
	m_RadialData.fMaxDistance = m_vMaxDistance.x;
	m_RadialData.fLengthScale = m_vLengthScale.x;
	m_RadialData.vPivot = _float2(0.7f, 0.75f);

	m_iLUT_Index = 3;
	m_vLutIntensity = _float2(0.f, 1.f);

	return S_OK;
}

HRESULT CGalbrenaUlti_PostSFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(SFX_RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	Setting_Scale(m_vWinSize.x, m_vWinSize.y);
	Setting_Pos(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.5f);

	return S_OK;
}

void CGalbrenaUlti_PostSFX::Priority_Update(_float fTimeDelta)
{
}

void CGalbrenaUlti_PostSFX::Update(_float fTimeDelta)
{
	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		m_pGameInstance->Setting_LUT(m_iPrevLutIndex, m_fPrevLutIntensity, m_PrevIsDynamicLUT);
		return;
	}

	m_fCurrentTime += fTimeDelta;

	//LUT Setting
	_float fLUTRatio = SmoothStep(m_vEffectTime.x, m_vRadialTime.y, m_fCurrentTime);
	if (fLUTRatio > 0.f)
	{
		m_fCurrentLutIntensity = lerp(m_vLutIntensity.x, m_vLutIntensity.y, fLUTRatio);
		m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fCurrentLutIntensity, true);
	}

	//Radial Setting
	_float fRadialRatio = SmoothStep(m_vRadialTime.x, m_vRadialTime.y, m_fCurrentTime);
	m_RadialData.fMaxDistance = lerp(m_vMaxDistance.x, m_vMaxDistance.y, fRadialRatio);
	m_RadialData.fLengthScale = lerp(m_vLengthScale.x, m_vLengthScale.y, fRadialRatio);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(SFX_RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	//Reverse
	m_IsReverse = false;
	for (_uint i = 0; i < 2; ++i)
	{
		if (m_fCurrentTime >= m_vReverseTime[i].x && m_fCurrentTime <= m_vReverseTime[i].y)
		{
			m_IsReverse = true;
			break;
		}
	}
}

void CGalbrenaUlti_PostSFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::POST_SFX, this)))
		return;
}

void CGalbrenaUlti_PostSFX::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_IsReverse", &m_IsReverse, sizeof(_bool))))
		CRASH("Failed to Bind vScreenSize");

	if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(m_pShader, "g_NoiseTexture")))
		CRASH("Failed to Bind NoseTexture");

	if (FAILED(m_pShader->Bind_Texture("g_SceneTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed to Bind SceneTexture");

	m_pShader->Begin(ENUM_CLASS(SHADER_SFX_BURST::GALBRENA_BLUR));

	m_pContext->PSSetConstantBuffers(0, 1, &m_pBuffer);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CGalbrenaUlti_PostSFX::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;

	m_pGameInstance->Get_Current_LutSetting(&m_iPrevLutIndex, &m_fPrevLutIntensity, &m_PrevIsDynamicLUT);

	m_vLutIntensity.x = m_fPrevLutIntensity;

	m_RadialData.fMaxDistance = m_vMaxDistance.x;
	m_RadialData.fLengthScale = m_vLengthScale.x;

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(SFX_RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);
}

HRESULT CGalbrenaUlti_PostSFX::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Noise"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pNoiseTexture), nullptr)))
		ASSERT_CRASH(m_pNoiseTexture);

	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(SFX_RADIAL_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Create : Constant Buffer");

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(SFX_RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	return S_OK;
}

CGalbrenaUlti_PostSFX* CGalbrenaUlti_PostSFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrenaUlti_PostSFX* pInstance = new CGalbrenaUlti_PostSFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrenaUlti_PostSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrenaUlti_PostSFX::Clone(void* pArg)
{
	CGalbrenaUlti_PostSFX* pInstance = new CGalbrenaUlti_PostSFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrenaUlti_PostSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrenaUlti_PostSFX::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
	Safe_Release(m_pBuffer);
	Safe_Release(m_pNoiseTexture);
}
