#include "EnginePch.h"
#include "Renderer.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "ShaderFilter.h"

CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice },
	m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CRenderer::Initialize()
{
	_uint iNumViewPort = { 1 };
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewPort, &ViewPort);

	m_iWinSizeX = static_cast<_uint>(ViewPort.Width);
	m_iWinSizeY = static_cast<_uint>(ViewPort.Height);

	if (FAILED(Ready_RT()))
		return E_FAIL;
	if (FAILED(Ready_MRT()))
		return E_FAIL;
	if (FAILED(Ready_Shader_Filter()))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		CRASH("Buffer Fail");

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		CRASH("Shader Fail");

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<_float>(m_iWinSizeX), static_cast<_float>(m_iWinSizeY), 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(static_cast<_float>( m_iWinSizeX ), static_cast<_float>( m_iWinSizeY ), 0.f, 1.f));

	m_pGameInstance->Begin_MRT(TEXT("MRT_SSAO"));
	m_pGameInstance->End_MRT();
#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Ready_Debug_RT(TEXT("RT_Diffuse"), 150.0f, 150.0f, 300.f, 300.f)))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Add_Render_Object(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (nullptr == pRenderObject)
		return E_FAIL;
	
	{
		lock_guard<recursive_mutex> lock(m_RecursiveMutex);
		m_RenderObjects[ENUM_CLASS(eRenderGroup)].push_back(pRenderObject);
		Safe_AddRef(pRenderObject);
	}

	return S_OK;
}

void CRenderer::Render()
{
	Render_Priority();
	Render_Shadow();
	Render_Outline();
	Render_NonBlend();
	Render_Light();
	Render_SSAO();
	Render_Combined();
	Render_NonLight();
	//Render_Emissive();
	//Render_Blur();
	//Render_Blend();
	//Render_Distortion();
	Render_LUT();
	Render_UI();
	Render_Fade();

#ifdef _DEBUG
	Render_Debug();
#endif
}

#ifdef _DEBUG
HRESULT CRenderer::Add_Render_Debug(CComponent* pDebugComponent)
{
	if (nullptr == pDebugComponent)
		return E_FAIL;

	m_DebugComponents.push_back(pDebugComponent);
	Safe_AddRef(pDebugComponent);

	return S_OK;
}
HRESULT CRenderer::Bind_RawValue(const _char* pConstantName, void* pValue, _uint iLength)
{
	return m_pShader->Bind_Value(pConstantName, pValue, iLength);
}
#endif

void CRenderer::Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY)
{
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.Width = static_cast<_float>(iWinSizeX);
	Viewport.Height = static_cast<_float>(iWinSizeY);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &Viewport);
}

void CRenderer::Render_Priority()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"))))
		CRASH("Render Fail")

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Shadow()
{
	Setting_Viewport(g_iMaxWidth, g_iMaxHeight);

	m_pGameInstance->Begin_CSM();

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow(); // if(m_pGameInstance->IsIn_SplitFrustrum())

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)].clear();

	m_pGameInstance->End_CSM();

	Setting_Viewport(m_iWinSizeX, m_iWinSizeY);
}

void CRenderer::Render_Outline()
{
	if(FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
	   CRASH("Failed Begin MRT");

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_OutLine();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_NonBlend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Object"))))
		CRASH("Render Fail")

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Light()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Light"))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		CRASH("Failed Bind WorldMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed Bind ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed Bind ProjMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");
	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		CRASH("Failed Bind CamPosition");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Normal"), m_pShader, "g_NormalTexture")))
		CRASH("Failed Bind RT_Normal");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_Depth");

	//Toon Ramp Texture
	if (FAILED(m_pFilter->Bind_Ramp_Texture(m_pShader, "g_RampTexture")))
		return;

	m_pGameInstance->Render_Light(m_pShader, m_pVIBuffer);

	m_pGameInstance->End_MRT();
}


void CRenderer::Render_SSAO()
{

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SSAO"))))
		CRASH("Render Fail");
#ifdef _DEBUG
	if (false == m_IsSSAO)
	{
		m_pGameInstance->End_MRT();
		return;
	}
#endif
	if (FAILED(m_pShader->Bind_Matrix("g_CamViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Matrix("g_CamProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Render Fail");

	if (FAILED(m_pFilter->Bind_Noise_Texture(m_pShader, "g_NoiseTexture")))
		CRASH("Failed Bind_Noise_Texture");

	if (FAILED(m_pFilter->Bind_Sample_Vector(m_pShader, "g_vSampleVector")))
		CRASH("Failed Bind Sample Vector");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSAO))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	//GaussianBlur_RenderTager(TEXT("RT_SSAO"), TEXT("MRT_SSAO"), BLUR_TYPE::GAUSSIAN);
#ifdef _DEBUG
	if (false == m_IsSSAO_Blur)
	{
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur"))))
			CRASH("Render Fail");

		m_pGameInstance->End_MRT();
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BlurEnd"))))
			CRASH("Render Fail");

		m_pGameInstance->End_MRT();
		return;
	}
#endif
	SSAO_Blur();
}

void CRenderer::Render_Combined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Diffuse"), m_pShader, "g_DiffuseTexture")))
		CRASH("Render Fail")
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Shade"), m_pShader, "g_ShadeTexture")))
		CRASH("Render Fail")
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Specular"), m_pShader, "g_SpecularTexture")))
		CRASH("Render Fail")
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_SSAO"), m_pShader, "g_SsaoTexture")))
		CRASH("Failed Bind_SsaoTexture");

	if(FAILED(m_pGameInstance->Bind_CSM_SRV(m_pShader, "g_ShadowMap")))
		CRASH("Failed Bind_CSM_SRV");
	
	if (FAILED(m_pGameInstance->Bind_CSM_Resources(m_pShader, "g_ShadowViewMatrix", "g_ShadowProjMatrix", "g_vLightDirection")))
		CRASH("Failed Bind CSM Resource");

	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		CRASH("Render Fail")

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::COMBINED))))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_ShadowDistance_Resource(1)))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_NonLight()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail")

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Emissive()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Emissive"), nullptr, false)))
		CRASH("Render Fail")

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::EMISSIVE)])
	{
		if(nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::EMISSIVE)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_DistortionObject()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Distortion"), nullptr, false)))
		CRASH("Render Fail")

		for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::DISTORTION)])
		{
			if (nullptr != pRenderObject)
				pRenderObject->Render();

			Safe_Release(pRenderObject);
		}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::DISTORTION)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Blur()
{
//{
//	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur"))))
//		CRASH("Render Fail")
//
//	//if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
//	//	CRASH("Render Fail")
//	//if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
//	//	CRASH("Render Fail")
//	//if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
//	//	CRASH("Render Fail")
//
//	if (FAILED(m_pShader->Bind_Value("g_fWidth", &m_iWinSizeX, sizeof(_float))))
//		CRASH("Render Fail")
//
//	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Emissive"), m_pShader, "g_BlurBeginTexture")))
//		CRASH("Render Fail")
//
//	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR_X))))
//		CRASH("Render Fail")
//
//	m_pVIBuffer->Bind_Resources();
//	m_pVIBuffer->Render();
//
//	m_pGameInstance->End_MRT();
//
//	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BlurEnd"))))
//		CRASH("Render Fail")
//
//	//if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
//	//	CRASH("Render Fail")
//	//if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
//	//	CRASH("Render Fail")
//	//if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
//	//	CRASH("Render Fail")
//
//	if (FAILED(m_pShader->Bind_Value("g_fHeight", &m_iWinSizeY, sizeof(_float))))
//		CRASH("Render Fail")
//
//	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Blur"), m_pShader, "g_BlurTexture")))
//		CRASH("Render Fail")
//
//	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BackBuffer"), m_pShader, "g_BackBufferTexture")))
//		CRASH("Render Fail")
//
//	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR_Y))))
//		CRASH("Render Fail")
//
//	m_pVIBuffer->Bind_Resources();
//	m_pVIBuffer->Render();
//
//	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Blend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail")

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)].clear();

//	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Distortion()
{
//	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
//		CRASH("Render Fail")

	//if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
	//	CRASH("Render Fail")
	//if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
	//	CRASH("Render Fail")
	//if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
	//	CRASH("Render Fail")

	//if (FAILED(m_pShader->Bind_Value("g_fWidth", &m_iWinSizeX, sizeof(_float))))
	//	CRASH("Render Fail")
	//if (FAILED(m_pShader->Bind_Value("g_fHeight", &m_iWinSizeY, sizeof(_float))))
	//	CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Distortion"), m_pShader, "g_DistortionTexture")))
		CRASH("Render Fail")
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BlurEnd"), m_pShader, "g_BlurEndTexture")))
		CRASH("Render Fail")

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DISTORTION));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_LUT()
{
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BackBuffer"), m_pShader, "g_BackBufferTexture")))
		CRASH("Failed Bind RT_Backbuffer");

	if (FAILED(m_pFilter->Bind_LUT_Texture(m_pShader, "g_LUT_Texture", m_iLUT_Index, "g_iLutIndex")))
		return;

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::LUT));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CRenderer::Render_UI()
{
	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::UI)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::UI)].clear();
}

void CRenderer::Render_Fade()
{
	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::FADE)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::FADE)].clear();
}

void CRenderer::GaussianBlur_RenderTager(const _tchar* pBlurRenderTarget, const _tchar* pCombinedBlurMRT, BLUR_TYPE eType)
{
	_uint iShaderPass = eType == BLUR_TYPE::GAUSSIAN ? ENUM_CLASS(SHADER_DEFFERED::GAUSSIAN_BLUR_X) : 0;// ENUM_CLASS(SHADER_DEFFERED::BILATERAL_BLUR_X);

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur"))))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(pBlurRenderTarget, m_pShader, "g_BlurBeginTexture")))
		CRASH("Render Fail")

	if (FAILED(m_pShader->Bind_Value("g_fWidth", &m_iWinSizeX, sizeof(_float))))
		CRASH("Render Fail")

	if (FAILED(m_pShader->Begin(iShaderPass++)))
		CRASH("Render Fail")

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BlurEnd"))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Value("g_fHeight", &m_iWinSizeY, sizeof(_float))))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Blur"), m_pShader, "g_BlurTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Begin(iShaderPass)))
		CRASH("Render Fail")

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	if (FAILED(m_pGameInstance->Begin_MRT(pCombinedBlurMRT, nullptr, false)))
		CRASH("Render Fail");

	if(FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BlurEnd"), m_pShader, "g_BlurEndTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR_COMBINED))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::SSAO_Blur()
{
	_float fFar = m_pGameInstance->Get_CurrentCamera_Far();

	if (FAILED(m_pShader->Bind_Value("g_fFar", &fFar, sizeof(_float))))
		CRASH("Failed Bind Far");

#pragma region BLUR_X
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur"))))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_SSAO"), m_pShader, "g_BlurBeginTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Value("g_fWidth", &m_iWinSizeX, sizeof(_float))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSAO_BLUR_X))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
#pragma endregion

#pragma region BLUR_Y

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BlurEnd"))))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Blur"), m_pShader, "g_BlurTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Value("g_fHeight", &m_iWinSizeY, sizeof(_float))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSAO_BLUR_Y))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
#pragma endregion

#pragma region BLUR_RETURN
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SSAO"), nullptr, false)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BlurEnd"), m_pShader, "g_BlurEndTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR_COMBINED))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
#pragma region BLUR_END
}

#ifdef _DEBUG
void CRenderer::Render_Debug()
{
	if (m_pGameInstance->Get_DIKeyState(DIK_PGDN) == KEYSTATE::DOWN)
		m_isRenderDebug = !m_isRenderDebug;

	if (m_pGameInstance->Get_DIKeyState(DIK_HOME) == KEYSTATE::DOWN)
		m_IsSSAO = !m_IsSSAO;

	for (auto& pComponent : m_DebugComponents)
	{
		if (nullptr != pComponent)
			pComponent->Render();
		Safe_Release(pComponent);
	}
	m_DebugComponents.clear();

	{   

		if(FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Debug"))))
			CRASH("MRT_Debug");

		for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RD_DEBUG)])
		{
			if (nullptr != pRenderObject)
				pRenderObject->Render();

			Safe_Release(pRenderObject);
		}

		m_RenderObjects[ENUM_CLASS(RENDERGROUP::RD_DEBUG)].clear();

		m_pGameInstance->End_MRT();
	}

	if (false == m_isRenderDebug)
		return;

	if (FAILED(m_pGameInstance->Render_RT()))
		CRASH("Render RT");


	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("ProjMatrix");
	m_pGameInstance->Render_CSM(m_pShader, m_pVIBuffer);
		
}
#endif

HRESULT CRenderer::Ready_RT()
{
	/* RenderTarget Diffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Diffuse"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 0.f, 1.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Normal */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Normal"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Depth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Depth"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Shade */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Shade"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Speuclar */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Specular"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SSAO */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_SSAO"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget LightDepth_Map */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_LightDepth_Map"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Back_Buffer */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_BackBuffer"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Emissive*/
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Emissive"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Blur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Blur"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Blur_End*/
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_BlurEnd"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Distortion */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Distortion"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	
#ifdef _DEBUG
	/* RenderTarget Debug */
	if(FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Debug"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 0.f))))
		ASSERT_CRASH(false);
#endif

	return S_OK;
}

HRESULT CRenderer::Ready_MRT()
{
	// RENDERGROUP::NONBLEND
#pragma region MRT_OBJECT    
	if(FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Diffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Normal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_LIGHT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_Shade"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_Specular"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_SSAO
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_SSAO"), TEXT("RT_SSAO"))))
		ASSERT_CRASH(false);
#pragma endregion

	// RENDERGROUP::SHADOW_MAP // 미구현
#pragma region MRT_SHADOW_MAP
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Shadow_Map"), TEXT("RT_LightDepth_Map"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_BACKBUFFER
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BackBuffer"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
#pragma endregion

	// RENDERGROUP::EMISSIVE
#pragma region MRT_EMISSIVE
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Emissive"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Emissive"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_BLUR
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur"), TEXT("RT_Blur"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_BLUR_END
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurEnd"), TEXT("RT_BlurEnd"))))
		ASSERT_CRASH(false);
#pragma endregion

	// RENDERGROUP::DISTORTION
#pragma region MRT_DISTORTION
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Distortion"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
#pragma endregion

#ifdef _DEBUG
#pragma region MRT_DEBUG
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Debug"), TEXT("RT_Debug"))))
		ASSERT_CRASH(false);
#pragma endregion
#endif

	return S_OK;
}

HRESULT CRenderer::Ready_Shadow_DSV()
{
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = g_iMaxWidth;
	TextureDesc.Height = g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture2D* pTexture2D = { nullptr };
	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture2D)))
		CRASH("Shadow DSV Texture");

	Safe_Release(pTexture2D);

	return S_OK;
}

HRESULT CRenderer::Ready_Shader_Filter()
{
	m_pFilter = CShaderFilter::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pFilter);

	return S_OK;
}

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer* pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : Renderer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRenderer::Free()
{
	__super::Free();

#ifdef _DEBUG
	for (auto& pComponent : m_DebugComponents)
		Safe_Release(pComponent);
	m_DebugComponents.clear();
#endif

	for (size_t i = 0; i < ENUM_CLASS(RENDERGROUP::END); i++)
	{
		for (auto& pRenderObject : m_RenderObjects[i])
			Safe_Release(pRenderObject);

		m_RenderObjects[i].clear();
	}


	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pFilter);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
