#include "EnginePch.h"
#include "ShadowMap.h"
#include "GameInstance.h"

CShadowMap::CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CShadowMap::Initialize(const SHADOW_MAP_DESC& MapDesc)
{
	m_MapDesc = MapDesc;

	if (FAILED(Ready_ShadowMap()))
		return E_FAIL;

	if (FAILED(Ready_Matrices()))
		return E_FAIL;

    return S_OK;
}

HRESULT CShadowMap::Ready_ShadowMap()
{
	_uint iSizeX = m_MapDesc.iSectorSizeX * m_MapDesc.iNumSectorX;
	_uint iSizeY = m_MapDesc.iSectorSizeY * m_MapDesc.iNumSectorY;

	if (iSizeX > g_iMaxShadowMapSize || iSizeY > g_iMaxShadowMapSize)
		CRASH("Failed Ready ShadowMap");

	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = iSizeX;
	TextureDesc.Height = iSizeY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;

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
	DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Texture2DArray.MipSlice = 0;
	DsvDesc.Texture2DArray.FirstArraySlice = 0;
	DsvDesc.Texture2DArray.ArraySize = 1;

	if (FAILED(m_pDevice->CreateDepthStencilView(pTexture2D, &DsvDesc, &m_pShadowMapDSV)))
		CRASH("Failed Created ShadowMap DSV");

	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Texture2DArray.MostDetailedMip = 0;
	SrvDesc.Texture2DArray.MipLevels = 1;
	SrvDesc.Texture2DArray.FirstArraySlice = 0;
	SrvDesc.Texture2DArray.ArraySize = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture2D, &SrvDesc, &m_pShadowMapSRV)))
		CRASH("Failed Created ShadowMap SRV");

	Safe_Release(pTexture2D);

    return S_OK;
}

HRESULT CShadowMap::Ready_Matrices()
{
	_int iRadiusX = (m_MapDesc.iNumSectorX - 1);
	_int iRadiusY = (m_MapDesc.iNumSectorY - 1);

	for (_int i = -iRadiusX; i <= iRadiusX; i+=2)
	{
		for (_int j = -iRadiusY; j <= iRadiusY; j+=2)
		{
			_float3 vCenterPos = Compute_CenterPos(i, j, m_MapDesc.vCenterPos, m_MapDesc.vExtents);

			BoundingBox Bounding = BoundingBox(vCenterPos, m_MapDesc.vExtents);

			_float4x4 ViewMatrix = Make_ViewMatrix(vCenterPos, Bounding, m_MapDesc.vLightDir);
			_float4x4 ProjMatrix = Make_ProjMatrix(Bounding, ViewMatrix);

			m_Matrices[ENUM_CLASS(D3DTS::VIEW)].push_back(ViewMatrix);
			m_Matrices[ENUM_CLASS(D3DTS::PROJ)].push_back(ProjMatrix);
			m_Boundings.push_back(Bounding);
		}
	}

	return S_OK;
}

_float3 CShadowMap::Compute_CenterPos(_int iWeightX, _int iWieghtY, _float3 vOriginPos, _float3 vExtents)
{
	_float fExtentsX = vExtents.x * iWeightX;
	_float fExtentsZ = vExtents.z * iWieghtY;

	_float3 vCenterPos = _float3(vOriginPos.x + fExtentsX, vOriginPos.y, vOriginPos.z + fExtentsZ);

	return vCenterPos;
}

_float CShadowMap::Compute_MaxRadius(const BoundingBox& Bounding, _float3 vCenterPos)
{
	_float fRadius = 0.f;
	_float fDistance = 0.f;

	_float3 vPoints[8];
	Bounding.GetCorners(vPoints);

	for (_uint i = 0; i < 8; i++)
	{
		fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&vCenterPos), XMLoadFloat3(&vPoints[i]))));
		fRadius = max(fRadius, fDistance);
	}

	return fRadius;
}

_float4x4 CShadowMap::Make_ViewMatrix(_float3 vCenterPos, const BoundingBox& Bounding, _float3 vDir)
{
	_vector vAt = XMLoadFloat3(&vCenterPos);

	_float fMaxRadius = Compute_MaxRadius(Bounding, vCenterPos);

	_vector vLookDir = XMVector3Normalize(XMLoadFloat3(&vDir));

	_vector vEye = XMVectorSubtract(vAt, XMVectorScale(vLookDir, fMaxRadius));

	_float4x4 ViewMatrix = {};

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(vEye, vAt, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	return ViewMatrix;
}

_float4x4 CShadowMap::Make_ProjMatrix(BoundingBox Bounding, _float4x4 ViewMatrix)
{
	Bounding.Transform(Bounding, XMLoadFloat4x4(&ViewMatrix));
		
	_float fMinX = FLT_MAX, fMaxX = FLT_MAX * -1.f;
	_float fMinY = FLT_MAX, fMaxY = FLT_MAX * -1.f;
	_float fMinZ = FLT_MAX, fMaxZ = FLT_MAX * -1.f;

	_float3 vViewPoint[8];
	Bounding.GetCorners(vViewPoint);

	for (_uint i = 0; i < 8; i++)
	{
		fMinX = min(fMinX, vViewPoint[i].x);
		fMaxX = max(fMaxX, vViewPoint[i].x);

		fMinY = min(fMinY, vViewPoint[i].y);
		fMaxY = max(fMaxY, vViewPoint[i].y);

		fMinZ = min(fMinZ, vViewPoint[i].z);
		fMaxZ = max(fMaxZ, vViewPoint[i].z);
	}

	_float4x4 ProjMatrix = {};

	XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicOffCenterLH(fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ));

	return ProjMatrix;
}

CShadowMap* CShadowMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const SHADOW_MAP_DESC& MapDesc)
{
	CShadowMap* pInstance = new CShadowMap(pDevice, pContext);
	if (FAILED(pInstance->Initialize(MapDesc)))
	{
		MSG_BOX("Failed to Created : CShadowMap");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CShadowMap::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pShadowMapDSV);
	Safe_Release(m_pShadowMapSRV);
	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);
}
