#include "EnginePch.h"
#include "ShadowMap.h"
#include "GameInstance.h"
#include "StaticObject.h"

CShadowMap::CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CShadowMap::Setting_ShadowMap(const SHADOW_MAP_DESC& MapDesc)
{
	m_MapDesc = MapDesc;
	
	if (FAILED(Ready_ShadowMap()))
		return E_FAIL;

	if (FAILED(Ready_SectorUV()))
		return E_FAIL;

	if (FAILED(Ready_Buffers()))
		return E_FAIL;

	if (FAILED(Ready_Matrices()))
		return E_FAIL;

    return S_OK;
}

HRESULT CShadowMap::Bind_ShadowMap_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iSector)
{
	if (iSector >= m_iNumSector)
		return E_FAIL;

	Setting_ShadowMapViewPort(iSector);

	if (FAILED(pShader->Bind_Matrix(pViewName, &m_Matrices[ENUM_CLASS(D3DTS::VIEW)][iSector])))
		CRASH("Failed View Matrix");

	if (FAILED(pShader->Bind_Matrix(pProjName, &m_Matrices[ENUM_CLASS(D3DTS::PROJ)][iSector])))
		CRASH("Failed PROJ Matrix");

	return S_OK;
}

HRESULT CShadowMap::Bind_ShadowMap_Buffer(_uint iBufferIndex)
{
//	m_pContext->PSSetConstantBuffers(iBufferIndex, 1, &m_pConstantBuffer);

	return S_OK;
}

HRESULT CShadowMap::Bind_ShadowMap_Resources(CShader* pShader)
{
	_bool HasShadowMap = true;

	if (m_iNumSector <= 0)
	{
		HasShadowMap = false;
		if (FAILED(pShader->Bind_Value("g_HasShadowMap", &HasShadowMap, sizeof(_bool))))
			return E_FAIL;
		return S_OK;
	}

	if (FAILED(pShader->Bind_Value("g_HasShadowMap", &HasShadowMap, sizeof(_bool))))
		CRASH("Failed Bind HasShadowMap");

	if (FAILED(pShader->Bind_Texture("g_ShadowMap", m_pShadowMapSRV)))
		CRASH("Failed Bind ShadowMap");

	if (FAILED(pShader->Bind_Matrices("g_SectorViewMatrix", m_Matrices[ENUM_CLASS(D3DTS::VIEW)].data(), m_iNumSector)))
		CRASH("Failed View Matrix");

	if (FAILED(pShader->Bind_Matrices("g_SectorProjMatrix", m_Matrices[ENUM_CLASS(D3DTS::PROJ)].data(), m_iNumSector)))
		CRASH("Failed Proj Matrix");

	if (FAILED(pShader->Bind_Value("g_vSectorUV", m_SectorUV.data(), sizeof(_float4) * m_iNumSectorToLayer)))
		CRASH("Failed SectorStartPos");

	//TEST
	_float fMinX = m_MapDesc.vCenterPos.x;
	_float fMinZ = m_MapDesc.vCenterPos.z;
	_float2 vSectorWorldSize = _float2(m_MapDesc.vExtents.x * 2.f, m_MapDesc.vExtents.z * 2.f);
	_float2 vMin = _float2(fMinX, fMinZ);

	if (FAILED(pShader->Bind_Value("iNumSectorX", &m_MapDesc.iNumSectorX, sizeof(_int))))
		CRASH("Failed SectorStartPos");

	if (FAILED(pShader->Bind_Value("iNumSectorToLayer", &m_iNumSectorToLayer, sizeof(_int))))
		CRASH("Failed SectorStartPos");

	if (FAILED(pShader->Bind_Value("vSectorWorldSize", &vSectorWorldSize, sizeof(_float2))))
		CRASH("Failed SectorStartPos");

	if (FAILED(pShader->Bind_Value("vMin", &vMin, sizeof(_float2))))
		CRASH("Failed SectorStartPos");

	if (FAILED(pShader->Bind_Value("vShadowMapSize", &m_vShadowMapSize, sizeof(_float2))))
		CRASH("Failed SectorStartPos");

	return S_OK;
}

HRESULT CShadowMap::Begin_ShadowMap()
{
	m_pContext->OMGetRenderTargets(1, &m_pBackBuffer, &m_pOriginalDSV);

	ID3D11RenderTargetView* pRTV = { nullptr };

	m_pContext->OMSetRenderTargets(1, &pRTV, m_pShadowMapDSV);

	return S_OK;
}

HRESULT CShadowMap::End_ShadowMap()
{
	m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pOriginalDSV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);

	return S_OK;
}

#ifdef _DEBUG
void CShadowMap::Render(CShader* pShader, class CVIBuffer_Rect* pVIBuffer)
{
	if (nullptr == m_pShadowMapSRV)
		return;

	for (_uint i = 0; i < m_iNumLayer; ++i)
	{
		_matrix World = XMMatrixScaling(150.f, 150.f, 1.f) * XMMatrixTranslationFromVector(XMVectorSet(200.f, (-200.f * i + 1) + 680.f * 0.5f, 0.1f, 1.f));
		_float4x4 DebugWorld = {};
		XMStoreFloat4x4(&DebugWorld, World);

		pShader->Bind_Value("g_DebugCSMIndex", &i, sizeof(_uint));
		pShader->Bind_Texture("g_ShadowMap", m_pShadowMapSRV);
		pShader->Bind_Matrix("g_WorldMatrix", &DebugWorld);

		pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::RD_DEBUG_SHAODW_MAP));
		pVIBuffer->Bind_Resources();
		pVIBuffer->Render();
	}

}
#endif

void CShadowMap::Setting_ShadowMapViewPort(_uint iSector)
{
	_uint iSetorIndex = iSector % m_iNumSectorToLayer;

	_float fSectorSizeX = static_cast<_float>(m_MapDesc.iSectorSizeX);
	_float fSectorSizeY = static_cast<_float>(m_MapDesc.iSectorSizeZ);

	_uint iX = iSetorIndex % m_iNumSectorX_ToLayer;
	_uint iY = iSetorIndex / m_iNumSectorX_ToLayer;

	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = static_cast<_float>(iX) * fSectorSizeX;
	Viewport.TopLeftY = static_cast<_float>(iY) * fSectorSizeY;
	Viewport.Width = fSectorSizeX;
	Viewport.Height = fSectorSizeY;
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &Viewport);
}

HRESULT CShadowMap::Ready_ShadowMap()
{
	m_iNumSector = m_MapDesc.iNumSectorX * m_MapDesc.iNumSectorZ;
	
	m_iNumSectorX_ToLayer = (g_iMaxShadowMapSize / m_MapDesc.iSectorSizeX);
	m_iNumSectorZ_ToLayer = (g_iMaxShadowMapSize / m_MapDesc.iSectorSizeZ);

	m_iNumSectorToLayer = m_iNumSectorX_ToLayer * m_iNumSectorZ_ToLayer;

	m_iNumLayer = m_iNumSector / m_iNumSectorToLayer;

	m_iShadowMapSizeX = m_MapDesc.iSectorSizeX * m_iNumSectorX_ToLayer;	// Texture Width
	m_iShadowMapSizeY = m_MapDesc.iSectorSizeZ * m_iNumSectorZ_ToLayer; // Texture Height

	if (m_iShadowMapSizeX > g_iMaxShadowMapSize || m_iShadowMapSizeY > g_iMaxShadowMapSize)
		CRASH("Failed Ready ShadowMap");

	m_vShadowMapSize = _float2(static_cast<_float>(m_iShadowMapSizeX), static_cast<_float>(m_iShadowMapSizeY));

	m_vSectorSize = _float2(static_cast<_float>(m_MapDesc.iSectorSizeX), static_cast<_float>(m_MapDesc.iSectorSizeZ));

	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = m_iShadowMapSizeX;
	TextureDesc.Height = m_iShadowMapSizeY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = m_iNumLayer;

	TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;			// DSV, SRV
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture2D* pTexture2D = { nullptr };
	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture2D)))
		CRASH("Failed Created ShadowMap Texture");

	/////DSV/////
	D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
	DsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	DsvDesc.Texture2DArray.MipSlice = 0;
	DsvDesc.Texture2DArray.FirstArraySlice = 0;
	DsvDesc.Texture2DArray.ArraySize = m_iNumLayer;

	if (FAILED(m_pDevice->CreateDepthStencilView(pTexture2D, &DsvDesc, &m_pShadowMapDSV)))
		CRASH("Failed Created ShadowMap DSV");

	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	SrvDesc.Texture2DArray.MostDetailedMip = 0;
	SrvDesc.Texture2DArray.MipLevels = 1;
	SrvDesc.Texture2DArray.FirstArraySlice = 0;
	SrvDesc.Texture2DArray.ArraySize = m_iNumLayer;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture2D, &SrvDesc, &m_pShadowMapSRV)))
		CRASH("Failed Created ShadowMap SRV");

	m_pContext->ClearDepthStencilView(m_pShadowMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	Safe_Release(pTexture2D);

    return S_OK;
}

HRESULT CShadowMap::Ready_Buffers()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(SHADOW_MAP_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pConstantBuffer)))
		CRASH("ShadowMap Constant Buffer");

	SHADOW_MAP_DATA Data = {};
	ZeroMemory(&Data, sizeof(SHADOW_MAP_DATA));

	_float fMinX = m_MapDesc.vCenterPos.x;
	_float fMinZ = m_MapDesc.vCenterPos.z;

	Data.iNumSectorX = m_MapDesc.iNumSectorX;
	Data.iNumSectorToLayer = m_iNumSectorToLayer;
	Data.vSectorWorldSize = _float2(m_MapDesc.vExtents.x * 2.f, m_MapDesc.vExtents.z * 2.f);
	Data.vMin = _float2(fMinX, fMinZ);

	D3D11_MAPPED_SUBRESOURCE SubResource;
	m_pContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, reinterpret_cast<void*>(&Data), sizeof(SHADOW_MAP_DATA));
	m_pContext->Unmap(m_pConstantBuffer, 0);

	return S_OK;
}

HRESULT CShadowMap::Ready_SectorUV()
{
	for (_uint i = 0; i < m_iNumSectorZ_ToLayer; ++i)
	{
		for (_uint j = 0; j < m_iNumSectorX_ToLayer; ++j)
		{
			_float fSectorStartU = (m_vSectorSize.x * j) / m_iShadowMapSizeX;
			_float fSectorStartV = (m_vSectorSize.y * i) / m_iShadowMapSizeY;

			_float fSectorEndU = (m_vSectorSize.x * (j + 1)) / m_iShadowMapSizeX;
			_float fSectorEndV = (m_vSectorSize.y * (i + 1)) / m_iShadowMapSizeY;

			m_SectorUV.push_back(_float4(fSectorStartU, fSectorStartV, fSectorEndU, fSectorEndV));
		}
	}

	return S_OK;
}

HRESULT CShadowMap::Ready_Matrices()
{
	_int iRadiusX = (m_MapDesc.iNumSectorX);
	_int iRadiusZ = (m_MapDesc.iNumSectorZ);

	for (_uint z = 0; z < iRadiusZ; ++z)
	{
		for (_uint x = 0; x < iRadiusX; ++x)
		{
			_uint iWeightX = (x * 2) + 1;
			_uint iWeightZ = (z * 2) + 1;
			_float3 vCenterPos = Compute_CenterPos(iWeightX, iWeightZ, m_MapDesc.vCenterPos, m_MapDesc.vExtents);

			_float vMarginX = m_MapDesc.vExtents.x + (m_MapDesc.vExtents.x * 0.1f);
			_float vMarginZ = m_MapDesc.vExtents.z + (m_MapDesc.vExtents.z * 0.1f);

			_float3 vMarginExtents = _float3(vMarginX, m_MapDesc.vExtents.y, vMarginZ);
			BoundingBox* Bounding = new BoundingBox(vCenterPos, vMarginExtents);

			_float4x4 ViewMatrix = Make_ViewMatrix(vCenterPos, Bounding, m_MapDesc.vLightDir);
			_float4x4 ProjMatrix = Make_ProjMatrix(Bounding, ViewMatrix);

			m_Matrices[ENUM_CLASS(D3DTS::VIEW)].push_back(ViewMatrix);
			m_Matrices[ENUM_CLASS(D3DTS::PROJ)].push_back(ProjMatrix);
			m_Boundings.push_back(Bounding);
		}
	}

	return S_OK;
}

_float3 CShadowMap::Compute_CenterPos(_int iWeightX, _int iWeightZ, _float3 vOriginPos, _float3 vExtents)
{
	_float fExtentsX = vExtents.x * iWeightX;
	_float fExtentsZ = vExtents.z * iWeightZ;

	_float3 vCenterPos = _float3(vOriginPos.x + fExtentsX, vOriginPos.y, vOriginPos.z + fExtentsZ);

	return vCenterPos;

}

_float CShadowMap::Compute_MaxRadius(const BoundingBox* Bounding, _float3 vCenterPos)
{
	_float fRadius = 0.f;
	_float fDistance = 0.f;

	_float3 vPoints[8];
	Bounding->GetCorners(vPoints);

	for (_uint i = 0; i < 8; i++)
	{
		fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&vCenterPos), XMLoadFloat3(&vPoints[i]))));
		fRadius = max(fRadius, fDistance);
	}

	return fRadius;
}

_float4x4 CShadowMap::Make_ViewMatrix(_float3 vCenterPos, const BoundingBox* Bounding, _float3 vDir)
{
	_vector vAt = XMLoadFloat3(&vCenterPos);

	_float fMaxRadius = Compute_MaxRadius(Bounding, vCenterPos);

	_vector vLookDir = XMVector3Normalize(XMLoadFloat3(&vDir));

	_vector vEye = XMVectorSubtract(vAt, XMVectorScale(vLookDir, fMaxRadius));

	_float4x4 ViewMatrix = {};

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(vEye, vAt, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	return ViewMatrix;
}

_float4x4 CShadowMap::Make_ProjMatrix(const BoundingBox* Bounding, _float4x4 ViewMatrix)
{
	BoundingBox ViewBounding;

	Bounding->Transform(ViewBounding, XMLoadFloat4x4(&ViewMatrix));
		
	_float fMinX = FLT_MAX, fMaxX = FLT_MAX * -1.f;
	_float fMinY = FLT_MAX, fMaxY = FLT_MAX * -1.f;
	_float fMinZ = FLT_MAX, fMaxZ = FLT_MAX * -1.f;

	_float3 vViewPoint[8];
	ViewBounding.GetCorners(vViewPoint);

	for (_uint i = 0; i < 8; i++)
	{
		fMinX = min(fMinX, vViewPoint[i].x);
		fMaxX = max(fMaxX, vViewPoint[i].x);

		fMinY = min(fMinY, vViewPoint[i].y);
		fMaxY = max(fMaxY, vViewPoint[i].y);

		fMinZ = min(fMinZ, vViewPoint[i].z);
		fMaxZ = max(fMaxZ, vViewPoint[i].z);
	}

	_float fTexelSizeX = (ViewBounding.Extents.x * 2.f) / static_cast<_float>(m_vSectorSize.x);		// 현재 Cascade 프러스텀의 NDC상 Texel 사이즈 X

	_float fTexelSizeY = (ViewBounding.Extents.y * 2.f) / static_cast<_float>(m_vSectorSize.y);		// 현재 Cascade 프러스텀의 NDC상 Texel 사이즈 Y

	_float3 vViewCenterPos = ViewBounding.Center;

	vViewCenterPos.x = floor(vViewCenterPos.x / (fTexelSizeX * 0.5f)) * fTexelSizeX;
	vViewCenterPos.y = floor(vViewCenterPos.y / (fTexelSizeY * 0.5f)) * fTexelSizeY;

	_float4 vRect = _float4(vViewCenterPos.x - ViewBounding.Extents.x,		//fCascadeExtent,
		vViewCenterPos.x + ViewBounding.Extents.x,		//fCascadeExtent,
		vViewCenterPos.y - ViewBounding.Extents.y,		//fCascadeExtent,
		vViewCenterPos.y + ViewBounding.Extents.y);	// fCascadeExtent);

	_float fCascadeExtentZ = (fMaxZ - fMinZ) * 0.5f;

	_float fNear = vViewCenterPos.z - fCascadeExtentZ;
	_float fFar = vViewCenterPos.z + fCascadeExtentZ;

	_float4x4 ProjMatrix = {};
	
	XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicOffCenterLH(vRect.x, vRect.y, vRect.z, vRect.w, fNear, fFar));

	//XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicOffCenterLH(fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ));

	return ProjMatrix;
}

CShadowMap* CShadowMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CShadowMap(pDevice, pContext);
}

void CShadowMap::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);


	for (auto& pBounding : m_Boundings)
		Safe_Delete(pBounding);
	m_Boundings.clear();

	Safe_Release(m_pShadowMapDSV);
	Safe_Release(m_pShadowMapSRV);
	Safe_Release(m_pConstantBuffer);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);
}
