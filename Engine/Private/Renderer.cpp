#include "EnginePch.h"
#include "Renderer.h"

#include "GameInstance.h"
#include "GameObject.h"

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

	m_iWinSizeX = ViewPort.Width;
	m_iWinSizeY = ViewPort.Height;

	if (FAILED(Ready_RT()))
		return E_FAIL;
	if (FAILED(Ready_MRT()))
		return E_FAIL;
	if (FAILED(Ready_Shadow_DSV()))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(m_iWinSizeX, m_iWinSizeY, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_iWinSizeX, m_iWinSizeY, 0.f, 1.f));

#ifdef _DEBUG
	//if (FAILED(m_pGameInstance->Ready_Debug_RT(TEXT("Target_Emissive"), 150.0f, 150.0f, 300.f, 300.f)))
	//	return E_FAIL;
#endif

    return S_OK;
}

HRESULT CRenderer::Add_Render_Object(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
    if (nullptr == pRenderObject)
        return E_FAIL;

    m_RenderObjects[ENUM_CLASS(eRenderGroup)].push_back(pRenderObject);

    Safe_AddRef(pRenderObject);
    
    return S_OK;
}

HRESULT CRenderer::Render()
{
    if (FAILED(Render_Priority()))
        return E_FAIL;
	if (FAILED(Render_Shadow()))
		return E_FAIL;
    if (FAILED(Render_NonBlend()))
        return E_FAIL;
	if (FAILED(Render_Light()))
		return E_FAIL;
	if (FAILED(Render_Combined()))
		return E_FAIL;
	if (FAILED(Render_NonLight()))
		return E_FAIL;
    if (FAILED(Render_Blend()))
        return E_FAIL;
	if (FAILED(Render_Emissive()))
		return E_FAIL;
	if (FAILED(Render_Blur()))
		return E_FAIL;
    if (FAILED(Render_UI()))
        return E_FAIL;
	if (FAILED(Render_Fade()))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(Render_Debug()))
		return E_FAIL;
#endif


    return S_OK;
}

HRESULT CRenderer::Add_LUT(const _wstring& strLUTTag, const _tchar* pFilePath)
{
	_tchar      szExt[MAX_PATH] = {};

	// Path Split => 확장자만 추출
	_wsplitpath_s(pFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

	ID3D11ShaderResourceView* pSRV = { nullptr };

	HRESULT hr = {};

	if (0 == lstrcmp(szExt, TEXT(".dds")))
	{
		hr = CreateDDSTextureFromFile(m_pDevice, pFilePath, nullptr, &pSRV);
	}
	else if (0 == lstrcmp(szExt, TEXT(".tga")))
	{
		MSG_BOX("TGA");
		return E_FAIL;
	}
	else // dds외 Window가 지원하는 파일
	{
		hr = CreateWICTextureFromFile(m_pDevice, pFilePath, nullptr, &pSRV);
	}

	if (FAILED(hr))
	{
		MessageBoxW(NULL, pFilePath, L"Texture", MB_OK);
		return E_FAIL;
	}
	m_LUTSRVs.emplace(strLUTTag, pSRV);

	return S_OK;
}

HRESULT CRenderer::Change_LUT(const _wstring& strLUTTag)
{
	auto iter = m_LUTSRVs.find(strLUTTag);
	if (iter == m_LUTSRVs.end())
		return E_FAIL;

	Safe_Release(m_pMainLUTSRV);
	m_pMainLUTSRV = iter->second;
	Safe_AddRef(m_pMainLUTSRV);

	return S_OK;
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
#endif

void CRenderer::Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY)
{
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.Width = iWinSizeX;
	Viewport.Height = iWinSizeY;
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &Viewport);
}

HRESULT CRenderer::Render_Priority()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"))))
		return E_FAIL;

    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)].clear();

	m_pGameInstance->End_MRT();

    return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Shadow"), m_pShadowDSV)))
		return E_FAIL;

	Setting_Viewport(g_iMaxWidth, g_iMaxHeight);

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)].clear();

	m_pGameInstance->End_MRT();

	Setting_Viewport(m_iWinSizeX, m_iWinSizeY);

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)].clear();

	m_pGameInstance->End_MRT();

    return S_OK;
}

HRESULT CRenderer::Render_Light()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Render_Light(m_pShader, m_pVIBuffer)))
		return E_FAIL;

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Combined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_Shadow_Resource(m_pShader, "g_LightViewMatrix", "g_LightProjMatrix", "g_fLightFar")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_LightDepth"), m_pShader, "g_LightDepthTexture")))
		return E_FAIL;

	if (nullptr != m_pMainLUTSRV)
		m_pShader->Bind_Texture("g_LUTTexture", m_pMainLUTSRV);

	m_pShader->Begin(1);
	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		return E_FAIL;

    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)].clear();

	m_pGameInstance->End_MRT();

    return S_OK;
}

HRESULT CRenderer::Render_Emissive()
{
	// Alpha Sorting
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::EMISSIVE)].sort([this](CGameObject* pSrc, CGameObject* pDst) {
			return m_pGameInstance->Compute_Distance_ToCam(pSrc) > m_pGameInstance->Compute_Distance_ToCam(pDst);
		});

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Emissive"), nullptr , false)))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::EMISSIVE)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::EMISSIVE)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Blur()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	//if (FAILED(m_pShader->Bind_Value("g_fWidth", &m_iWinSizeX, sizeof(_uint))))
	//	return E_FAIL;
	//if (FAILED(m_pShader->Bind_Value("g_fHeight", &m_iWinSizeY, sizeof(_uint))))
	//	return E_FAIL;

	// Blur_X
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")))
		return E_FAIL;

	m_pShader->Begin(4);
	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	// Blur_Y
	//if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")))
	//	return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Blur_X"), m_pShader, "g_BlurTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_Distortion"), m_pShader, "g_DistortionTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("Target_BackBuffer"), m_pShader, "g_BackBufferTexture")))
		return E_FAIL;

	m_pShader->Begin(5);
	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::UI)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::UI)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_Fade()
{
	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::FADE)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::FADE)].clear();

	return S_OK;
}

#ifdef _DEBUG
HRESULT CRenderer::Render_Debug()
{
	if (m_pGameInstance->Get_DIKeyState(DIK_PGDN) == KEYSTATE::DOWN)
		m_isRenderDebug = !m_isRenderDebug;

	for (auto& pComponent : m_DebugComponents)
	{
		if (nullptr != pComponent)
			pComponent->Render();
		Safe_Release(pComponent);
	}
	m_DebugComponents.clear();

	if (false == m_isRenderDebug)
		return S_OK;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Render_RT(m_pShader, m_pVIBuffer)))
		return E_FAIL;

	return S_OK;
}
#endif

HRESULT CRenderer::Ready_RT()
{
	return S_OK;
}

HRESULT CRenderer::Ready_MRT()
{
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
		return E_FAIL;

	if (FAILED(m_pDevice->CreateDepthStencilView(pTexture2D, nullptr, &m_pShadowDSV)))
		return E_FAIL;

	Safe_Release(pTexture2D);

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

	for (auto& Pair : m_LUTSRVs)
		Safe_Release(Pair.second);
	m_LUTSRVs.clear();

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pShadowDSV);
	Safe_Release(m_pMainLUTSRV);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}
