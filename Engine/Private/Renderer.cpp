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
		CRASH("Buffer Fail");

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		CRASH("Shader Fail");

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

void CRenderer::Render()
{
	Render_Priority();
	Render_Shadow();
	Render_NonBlend();
	Render_Light();
	Render_Combined();
	Render_NonLight();
	Render_Blend();
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

void CRenderer::Render_Priority()
{
    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::PRIORITY)].clear();
}

void CRenderer::Render_Shadow()
{
	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)].clear();
}

void CRenderer::Render_NonBlend()
{
    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONBLEND)].clear();
}

void CRenderer::Render_Light()
{
}

void CRenderer::Render_Combined()
{
}

void CRenderer::Render_NonLight()
{
	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::NONLIGHT)].clear();
}

void CRenderer::Render_Blend()
{
    for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ENUM_CLASS(RENDERGROUP::BLEND)].clear();
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

#ifdef _DEBUG
void CRenderer::Render_Debug()
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
		return;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("ProjMatrix");

	if (FAILED(m_pGameInstance->Render_RT(m_pShader, m_pVIBuffer)))
		CRASH("Render RT");
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
		CRASH("Shadow DSV Texture");

	if (FAILED(m_pDevice->CreateDepthStencilView(pTexture2D, nullptr, &m_pShadowDSV)))
		CRASH("Shadow DSV");

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

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pShadowDSV);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}
