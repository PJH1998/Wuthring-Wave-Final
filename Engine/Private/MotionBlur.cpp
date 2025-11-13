#include "EnginePch.h"
#include "MotionBlur.h"
#include "GameInstance.h"

CMotionBlur::CMotionBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlur { pDevice, pContext }
{
}

HRESULT CMotionBlur::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_fWinSizeX = static_cast<_float>(iWinSizeX);
	m_fWinSizeY = static_cast<_float>(iWinSizeY);

	m_fLimitVelocity = 1.f;
	m_fLimitDepth = 150.f;
	m_fLengthScale = 45.f;

	D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
	DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	DefaultSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	DefaultSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	DefaultSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	DefaultSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	DefaultSamplerDesc.MinLOD = 0;
	DefaultSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&DefaultSamplerDesc, &m_pClampSampler);
	ASSERT_CRASH(m_pClampSampler);

	return S_OK;
}

HRESULT CMotionBlur::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	//READY VELOCITY_MAP
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_VELOCITY_MAP"))))
		CRASH("Failed Begin MRT_VELOCITY_MAP");

	if (FAILED(pShader->Bind_Value("g_fWidth", (&m_fWinSizeX), sizeof(_float))))
		CRASH("Failed Bind g_fWidth");
	if (FAILED(pShader->Bind_Value("g_fHeight", (&m_fWinSizeY), sizeof(_float))))
		CRASH("Failed Bind g_fHeight");

	if (FAILED(pShader->Bind_Matrix("g_PrevCamViewMatrix", m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_PrevCamProjMatrix", m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");


	if (FAILED(pShader->Bind_Value("g_fLimitDepth", &m_fLimitDepth, sizeof(_float))))
		CRASH("Failed Bind g_fLimitVelocity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::VELOCITY_MAP));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	// DOWNSAMPLE
	_uint iDownSizeX = m_iWinSizeX >> 1;
	_uint iDownSizeY = m_iWinSizeY >> 1;

	//BackBuffer DownScale
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOWNSAMPLE");

	// DEPTH DownScale
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE_DEPTH"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOWNSAMPLE");

	// Motion Blur + UpScale
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "DepthTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "VelocityMap", m_pGameInstance->Get_RT_SRV(TEXT("RT_VelocityMap")))))
		CRASH("Failed Add_SRVData");

	MOTION_BLUR_DATA Data = {};
	Data.fLimitVelocity = m_fLimitVelocity;
	Data.fLimitDepth = m_fLimitDepth;
	Data.fLengthScale = m_fLengthScale;

	if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_MotionBlur"), "MOTION_DATA", reinterpret_cast<void*>(&Data), sizeof(MOTION_BLUR_DATA))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_SamplerState(TEXT("RCS_MotionBlur"), 0, m_pClampSampler)))
		return E_FAIL;

	//if (FAILED(m_pSubResource->Set_DefalutSampler(TEXT("RCS_MotionBlur"), 0)))
	//	CRASH("Failed Set_DefalutSampler");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_MotionBlur"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_MotionBlur");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Combine"), pShader, "g_BackBufferTexture")))
		CRASH("Failed Bind BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_MotionBlur")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(pShader->Bind_Texture("g_VelocityMap", m_pGameInstance->Get_RT_SRV(TEXT("RT_VelocityMap")))))
		CRASH("Failed Bind VelocityMap");

	if (FAILED(pShader->Bind_Value("g_fLimitVelocity", &m_fLimitVelocity, sizeof(_float))))
		CRASH("Failed Bind g_fLimitVelocity");

	if (FAILED(pShader->Bind_Value("g_fEffectIntensity", &m_fIntensity, sizeof(_float))))
		CRASH("Failed Bind g_fEffectIntensity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::MOTION_BLUR));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

CMotionBlur* CMotionBlur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CMotionBlur* pInstance = new CMotionBlur(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CMotionBlur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMotionBlur::Free()
{
	__super::Free();

	Safe_Release(m_pClampSampler);
}
