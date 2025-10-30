#include "EnginePch.h"
#include "Renderer.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "RendererSubResource.h"
#include "RendererCS.h"

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

	m_fWinSizeX = ( ViewPort.Width );
	m_fWinSizeY = ( ViewPort.Height );

	if (FAILED(Ready_RT()))
		return E_FAIL;
	if (FAILED(Ready_MRT()))
		return E_FAIL;
	if (FAILED(Ready_SubResource()))
		return E_FAIL;
	if (FAILED(Ready_RCS()))
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
	Render_Emissive();
	Render_Bloom();
	Render_BloomCombined();
	Render_DistortionObject();
	Render_Blend();
	Render_Distortion();
	Render_LUT();
	Render_Fog();
	Render_Blur();
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
void CRenderer::Setting_SSAO(_float fRadius, _float fMaxDistance)
{
	m_pSubResource->Setting_SSAO(fRadius, fMaxDistance);
}
void CRenderer::SetBloomIntensity(_float fIntensity)
{
	m_pSubResource->SetBloomIntensity(fIntensity);
}
void CRenderer::Setting_Fog(_float2 vDepthDistance, _float2 vHeightDistance, _float4 vColor)
{
	m_pSubResource->Setting_Fog(vDepthDistance, vHeightDistance, vColor);
}
void CRenderer::SetDof(_float fDepth, _float fRange, _float fScale)
{
	m_pSubResource->SetDof(fDepth, fRange, fScale);
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

	Setting_Viewport(m_iWinSizeX, m_iWinSizeY);

	m_pGameInstance->End_CSM();

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
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_PBR"), m_pShader, "g_PBRTexture")))
		CRASH("Failed Bind RT_PBR");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Normal"), m_pShader, "g_NormalTexture")))
		CRASH("Failed Bind RT_Normal");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_Depth");

	//Toon Ramp Texture
	if (FAILED(m_pSubResource->Bind_Ramp_Texture(m_pShader, "g_RampTexture", 0)))
		return;

	m_pGameInstance->Render_Light(m_pShader, m_pVIBuffer);

	m_pGameInstance->End_MRT();
}


void CRenderer::Render_SSAO()
{
#ifdef _DEBUG
	if (false == m_IsSSAO)
	{
		m_pGameInstance->Clear_RT(TEXT("RT_SSAO"));
		m_pGameInstance->Clear_RCS(TEXT("RCS_SSAO_BLUR_X"));
		m_pGameInstance->Clear_RCS(TEXT("RCS_SSAO_BLUR_Y"));
		return;
	}
#endif
	
#pragma region SSAO
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SSAO"))))
		CRASH("Render Fail");

	if (FAILED(m_pSubResource->Bind_SSAO_Resources(m_pShader)))
		CRASH("Failed Bind SSAO Resources");

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSAO))))
		CRASH("Render Fail");

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
#pragma endregion

#pragma region BLUR_X
	if (FAILED(m_pSubResource->Add_SSAO_Blur_BufferData(TEXT("RCS_SSAO_BLUR_X"), m_fWinSizeX, m_fWinSizeY)))
		CRASH("Failed Add_SSAO_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_SSAO")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSAO_BLUR_X"))))
		CRASH("Failed RCS_SSAO_BLUR_X");
#pragma endregion

#pragma region BLUR_Y
	if (FAILED(m_pSubResource->Add_SSAO_Blur_BufferData(TEXT("RCS_SSAO_BLUR_Y"), m_fWinSizeX, m_fWinSizeY)))
		CRASH("Failed Add_SSAO_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_SSAO_BLUR_X")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSAO_BLUR_Y"))))
		CRASH("Failed RCS_SSAO_BLUR_Y");
#pragma endregion
}

void CRenderer::Render_Combined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Diffuse"), m_pShader, "g_DiffuseTexture")))
		CRASH("Failed Bind RT_Diffuse");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_ToonRim"), m_pShader, "g_ToonRimTexture")))
		CRASH("Render Fail")

	if(FAILED(m_pGameInstance->Bind_RendererCS(TEXT("RCS_SSAO_BLUR_Y"), m_pShader, "g_SsaoTexture")))
		CRASH("Failed Bind_SsaoTexture");

	if(FAILED(m_pGameInstance->Bind_CSM_SRV(m_pShader, "g_ShadowMap")))
		CRASH("Failed Bind_CSM_SRV");
	
	if (FAILED(m_pGameInstance->Bind_CSM_Resources(m_pShader, "g_ShadowViewMatrix", "g_ShadowProjMatrix", "g_vLightDirection")))
		CRASH("Failed Bind CSM Resource");

	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		CRASH("Render Fail");
	
	if (FAILED(m_pSubResource->Bind_Ramp_Texture(m_pShader, "g_ColorRampTexture", 1)))
		return;

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

void CRenderer::Render_Bloom()
{
	// DOWNSAMPLE FIRST
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Emissive")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), 0)))
		CRASH("Failed RCS_DOWNSAMPLE");

	for (_uint i = 0; i < 3; i++)
	{
		// DOWNSAMPLE SECOND
		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), i))))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), i + 1)))
			CRASH("Failed RCS_DOWNSAMPLE");
	}

	for (_int j = 3; j >= 0; --j)
	{
		// BLUR_X
		_uint iDownWinSizeX = static_cast<_uint>( m_fWinSizeX ) >> (j + 1);
		_uint iDownWinSizeY = static_cast<_uint>( m_fWinSizeY ) >> (j + 1);

		
		if (FAILED(m_pSubResource->Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_X"), iDownWinSizeX, iDownWinSizeY, m_iBloomWeight)))
			CRASH("Failed Add_SizeData_BufferData");

		ID3D11ShaderResourceView* pBlur = j == 3 ? m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), j) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE_BLOOM"), j + 1);

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_X"), "InputTexture", pBlur)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_X"), j)))
			CRASH("Failed RCS_GAUSSIAN_BLUR_X");

		// BLUR_Y
		if (FAILED(m_pSubResource->Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_Y"), iDownWinSizeX, iDownWinSizeY, m_iBloomWeight)))
			CRASH("Failed Add_SizeData_BufferData");

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_X"), j))))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_Y"), j)))
			CRASH("Failed RCS_GAUSSIAN_BLUR_Y");

		// UPSAMPLE
		_uint iUpWinSizeX = ( m_iWinSizeX ) >> j;
		_uint iUpWinSizeY = ( m_iWinSizeY ) >> j;

		if (FAILED(m_pSubResource->Add_Bloom_BufferData(TEXT("RCS_UPSAMPLE_BLOOM"), iUpWinSizeX, iUpWinSizeY, j)))
			CRASH("Failed Add_UpSample_BufferData");

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE_BLOOM"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_Y"), j))))
			CRASH("Failed Add_SRVData");

		ID3D11ShaderResourceView* pBase = j == 0 ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Emissive")) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), j - 1);
		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE_BLOOM"), "BaseTexture", pBase)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_UPSAMPLE_BLOOM"), j)))
			CRASH("Failed RCS_UPSAMPLE");
	}
}

void CRenderer::Render_BloomCombined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Emissive"), m_pShader, "g_BlurTexture")))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RendererCS(TEXT("RCS_UPSAMPLE_BLOOM"), m_pShader, "g_BloomTexture", 0)))
		CRASH("Failed Bind_RendererCS");

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLOOM));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

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

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Distortion()
{
//	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
//		CRASH("Render Fail")

	//if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Distortion"), m_pShader, "g_DistortionTexture")))
	//	CRASH("Render Fail")
	////if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BlurEnd"), m_pShader, "g_BlurEndTexture")))
	////	CRASH("Render Fail")

	//m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DISTORTION));

	//m_pVIBuffer->Bind_Resources();
	//m_pVIBuffer->Render();

	//m_pGameInstance->End_MRT();
}

void CRenderer::Render_LUT()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Lut"))))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_BackBuffer"), m_pShader, "g_BackBufferTexture")))
		CRASH("Failed Bind RT_Backbuffer");

	if (FAILED(m_pSubResource->Bind_LUT_Texture(m_pShader, m_iLUT_Index)))
		return;

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::LUT));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Fog()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false);

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Lut"), m_pShader, "g_LutResultTexture")))
		CRASH("Failed Bind RT_Lut");

	if (FAILED(m_pSubResource->Bind_Fog_Resources(m_pShader)))
		return;

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::FOG));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Blur()
{
	if (m_IsBlur)
	{
		switch(m_eBlurType)
		{
			case BLUR_TYPE::DOF:
				Render_DOF();
				break;
		}
	}
	else
	{
		if (FAILED(m_pShader->Bind_Texture("g_Texture", m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer")))))
			CRASH("Failed RT_BackBuffer");

		m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DRAW));

		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}
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

void CRenderer::Render_DOF()
{
	//COC 계산
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DOF"))))
		CRASH("Failed Begin MRT_DOF");

	if (FAILED(m_pSubResource->Bind_Dof_Resource(m_pShader)))
		CRASH("Failed Bind_Dof_Resource");

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DOF_DEPTH));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	//DOWNSAMPLE
	if(FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"))))
		CRASH("Failed RCS_DOWNSAMPLE");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE_DEPTH"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Dof")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"))))
		CRASH("Failed RCS_DOWNSAMPLE");

	//BLUR_X
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_X"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE")))))
		CRASH("Failed Add_SRVData");
	
	if(FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_X"), "DepthTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pSubResource->Add_DOF_BufferData(TEXT("RCS_DOF_X"), static_cast<_float>(m_iWinSizeX>> 1 ), static_cast<_float>( m_iWinSizeY >> 1 ))))
		CRASH("Failed Add_BufferData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOF_X"))))
		CRASH("Failed RCS_DOF_X");

	//BLUR_Y
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOF_X")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_Y"), "DepthTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pSubResource->Add_DOF_BufferData(TEXT("RCS_DOF_Y"), static_cast<_float>( m_iWinSizeX >> 1 ), static_cast<_float>( m_iWinSizeY >> 1 ))))
		CRASH("Failed Add_BufferData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOF_Y"))))
		CRASH("Failed RCS_DOF_Y");

	//UPSAMPLE
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE_DOF"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOF_Y")))))
		CRASH("Failed Add_SRVData");

	if(FAILED(m_pSubResource->Add_UPSample_BufferData(TEXT("RCS_UPSAMPLE_DOF"), m_fWinSizeX, m_fWinSizeY)))
		CRASH("Failed Add_BufferData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_UPSAMPLE_DOF"))))
		CRASH("Failed RCS_UPSAMPLE_DOF");

	//Combined
	if (FAILED(m_pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(m_pShader->Bind_Texture("g_DofTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Dof")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(m_pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE_DOF")))))
		CRASH("Failed Bind Blur Texture");
	

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DOF));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
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
	
	if (FAILED(m_pGameInstance->Debug_Render_RCS()))
		CRASH("Render RCS");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("ProjMatrix");

	m_pGameInstance->Render_CSM(m_pShader, m_pVIBuffer);
		
}
#endif

ID3D11ShaderResourceView* CRenderer::Get_BlurSRV()
{
	if (m_eBlurType == BLUR_TYPE::END)
		return nullptr;

	_wstring strRCSTag = {};

	switch (m_eBlurType)
	{
		case BLUR_TYPE::DOF:
			strRCSTag = TEXT("RCS_UPSAMPLE_DOF");
			break;

		case BLUR_TYPE::RADIAL:
			break;

		case BLUR_TYPE::MOTION:
			break;
	}

	return m_pGameInstance->Get_RCS_SRV(strRCSTag);
}

HRESULT CRenderer::Ready_RT()
{
	/* RenderTarget Diffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Diffuse"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 0.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Normal */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Normal"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Depth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Depth"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Metaliic */
	if(FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_PBR"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Shade */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_ToonRim"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Specular"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* RenderTarget Back_Buffer */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_BackBuffer"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Emissive*/
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Emissive"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Lut */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Lut"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SSAO */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_SSAO"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Distortion */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Distortion"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Dof"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
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
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);

#pragma endregion

#pragma region MRT_LIGHT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_ToonRim"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_Specular"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_SSAO
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_SSAO"), TEXT("RT_SSAO"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_LUT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lut"), TEXT("RT_Lut"))))
		ASSERT_CRASH(false);
#pragma endregion

	// RENDERGROUP::SHADOW_MAP // 미구현
#pragma region MRT_SHADOW_MAP
	//if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Shadow_Map"), TEXT("RT_LightDepth_Map"))))
	//	ASSERT_CRASH(false);
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
	// RENDERGROUP::DISTORTION
#pragma region MRT_DISTORTION
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Distortion"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_DOF
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DOF"), TEXT("RT_Dof"))))
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

HRESULT CRenderer::Ready_RCS()
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
	BlurRCS.iMipLevels = 4;
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
	DownSampleRCS.iMipLevels = 4;
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
	UpSampleRCS.strEntryPoint = "UpSample_Bloom";
	UpSampleRCS.iWidth = m_iWinSizeX;
	UpSampleRCS.iHeight = m_iWinSizeY;
	UpSampleRCS.fDefinitionX = 16.f;
	UpSampleRCS.fDefinitionY = 16.f;
	UpSampleRCS.eFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	UpSampleRCS.iMipLevels = 4;
	UpSampleRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);
	
	//UpSample Bloom
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE_BLOOM"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE");

	UpSampleRCS.iMipLevels = 2;
	UpSampleRCS.strEntryPoint = "UpSampleDOF";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE_DOF"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE_DEPTH");
#pragma endregion



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

HRESULT CRenderer::Ready_SubResource()
{
	m_pSubResource = CRendererSubResource::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSubResource);

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
	Safe_Release(m_pSubResource);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
