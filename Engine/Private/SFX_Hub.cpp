#include "EnginePch.h"
#include "SFX_Hub.h"
#include "SFX.h"
#include "GameInstance.h"
#include "RendererCS.h"
#include "SSAO.h"
#include "Bloom.h"
#include "DOF.h"
#include "MotionBlur.h"
#include "ScreenBlur.h"

CSFX_Hub::CSFX_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CSFX_Hub::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	if (FAILED(Ready_SFX()))
		return E_FAIL;

	if (FAILED(Ready_SFX_CS()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSFX_Hub::Begin_SFX(SFX_TOGGLE eType)
{
	CSFX* pSFX = Find_SFX(static_cast<SFX_TYPE>(eType));
	if (nullptr == pSFX)
		return E_FAIL;

	m_pCurrentSFX = pSFX;

	return S_OK;
}

HRESULT CSFX_Hub::End_SFX()
{
	if (nullptr == m_pCurrentSFX)
		return S_OK;

	m_pCurrentSFX = nullptr;

	return S_OK;
}

HRESULT CSFX_Hub::Render_SFX_Toggle(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	if (nullptr == m_pCurrentSFX)
		return E_FAIL;

	m_pCurrentSFX->Render(pVIBuffer, pShader);

	return S_OK;
}

HRESULT CSFX_Hub::Render_SFX(SFX_TYPE eType, CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	CSFX* pSFX = Find_SFX(eType);
	if (nullptr == pSFX)
		return E_FAIL;

	return pSFX->Render(pVIBuffer, pShader);
}

CSFX* CSFX_Hub::Find_SFX(SFX_TYPE eType)
{
	auto iter = m_SFXs.find(eType);
	if (iter == m_SFXs.end())
		return nullptr;
	return iter->second;
}

HRESULT CSFX_Hub::Ready_SFX()
{
	CSSAO* pSSao = CSSAO::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pSSao);
	m_SFXs.emplace(SFX_TYPE::SSAO, pSSao);

	CBloom* pBloom = CBloom::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pSSao);
	m_SFXs.emplace(SFX_TYPE::BLOOM, pBloom);

	CMotionBlur* pMotionBlur = CMotionBlur::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pMotionBlur);
	m_SFXs.emplace(SFX_TYPE::MOTION, pMotionBlur);

	CDOF* pDOF = CDOF::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pDOF);
	m_SFXs.emplace(SFX_TYPE::DOF, pDOF);

	CScreenBlur* pScreenBlur = CScreenBlur::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pScreenBlur);
	m_SFXs.emplace(SFX_TYPE::BLUR, pScreenBlur);

	return S_OK;
}

HRESULT CSFX_Hub::Ready_SFX_CS()
{
#pragma region SSAO_BLUR_RCS
	CRendererCS::RCS_DESC BlurDesc = {};
	BlurDesc.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_SSAO.hlsl");
	BlurDesc.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	BlurDesc.strEntryPoint = "SSAO_BLUR_X";
	BlurDesc.iWidth = m_iWinSizeX;
	BlurDesc.iHeight = m_iWinSizeY;
	BlurDesc.fDefinitionX = 16.f;
	BlurDesc.fDefinitionY = 16.f;
	BlurDesc.eFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	BlurDesc.iMipLevels = 1;
	BlurDesc.vClearColor = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSAO_BLUR_X"), &BlurDesc)))
		CRASH("Failed Add RCS_SSAO");

	BlurDesc.strEntryPoint = "SSAO_BLUR_Y";

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSAO_BLUR_Y"), &BlurDesc)))
		CRASH("Failed Add RCS_SSAO");

#pragma endregion

#pragma region BLUR
	CRendererCS::RCS_DESC BlurRCS = {};
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Blur.hlsl");
	BlurRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	BlurRCS.strEntryPoint = "GaussianBlur_X";
	BlurRCS.iWidth = m_iWinSizeX >> 1;
	BlurRCS.iHeight = m_iWinSizeY >> 1;
	BlurRCS.fDefinitionX = 16.f;
	BlurRCS.fDefinitionY = 16.f;
	BlurRCS.eFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	BlurRCS.iMipLevels = 3;
	BlurRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_GAUSSIAN_BLUR_X"), &BlurRCS)))
		CRASH("Failed Add G_BlurX");

	BlurRCS.strEntryPoint = "GaussianBlur_Y";

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_GAUSSIAN_BLUR_Y"), &BlurRCS)))
		CRASH("Failed Add G_BlurY");

	BlurRCS.strEntryPoint = "DOF_X";
	BlurRCS.iMipLevels = 2;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOF_X"), &BlurRCS)))
		CRASH("Failed Add DOF_X");

	BlurRCS.strEntryPoint = "DOF_Y";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOF_Y"), &BlurRCS)))
		CRASH("Failed Add DOF_Y");

	BlurRCS.strEntryPoint = "Motion_Blur";
	BlurRCS.iWidth = m_iWinSizeX;
	BlurRCS.iHeight = m_iWinSizeY;
	BlurRCS.iMipLevels = 1;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_MotionBlur"), &BlurRCS)))
		CRASH("Failed Add RCS_MotionBlur");
#pragma endregion

#pragma region DOWNSAMPLE
	CRendererCS::RCS_DESC DownSampleRCS = {};
	DownSampleRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Sampling.hlsl");
	DownSampleRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	DownSampleRCS.strEntryPoint = "DownSample";
	DownSampleRCS.iWidth = m_iWinSizeX >> 1;
	DownSampleRCS.iHeight = m_iWinSizeY >> 1;
	DownSampleRCS.fDefinitionX = 16.f;
	DownSampleRCS.fDefinitionY = 16.f;
	DownSampleRCS.eFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	DownSampleRCS.iMipLevels = 3;
	DownSampleRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOWNSAMPLE"), &DownSampleRCS)))
		CRASH("Failed Add RCS_DOWNSAMPLE");

	//DOF Depth
	DownSampleRCS.iMipLevels = 2;
	DownSampleRCS.eFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"), &DownSampleRCS)))
		CRASH("Failed Add RCS_DOWNSAMPLE_DEPTH");
#pragma endregion

#pragma region UPSAMPLE
	CRendererCS::RCS_DESC UpSampleRCS = {};
	UpSampleRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Sampling.hlsl");
	UpSampleRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	UpSampleRCS.strEntryPoint = "UpSample";
	UpSampleRCS.iWidth = m_iWinSizeX;
	UpSampleRCS.iHeight = m_iWinSizeY;
	UpSampleRCS.fDefinitionX = 16.f;
	UpSampleRCS.fDefinitionY = 16.f;
	UpSampleRCS.eFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	UpSampleRCS.iMipLevels = 3;
	UpSampleRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	//Default
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE_DEPTH");

	//UpSample Bloom
	UpSampleRCS.strEntryPoint = "UpSample_Bloom";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE_BLOOM"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE");
#pragma endregion

	return S_OK;
}

CSFX_Hub* CSFX_Hub::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CSFX_Hub* pInstance = new CSFX_Hub(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CSFX_Hub");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSFX_Hub::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	
	for (auto& Pair : m_SFXs)
		Safe_Release(Pair.second);
	m_SFXs.clear();

	Safe_Release(m_pCurrentSFX);
}
