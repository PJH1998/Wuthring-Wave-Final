#include "EnginePch.h"
#include "Picking.h"

#include "GameInstance.h"

CPicking::CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPicking::Initialize(HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{
	m_hWnd = hWnd;
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = iWinSizeX;
	TextureDesc.Height = iWinSizeY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

    return S_OK;
}

void CPicking::Update()
{
	GetCursorPos(&m_ptMouse);
	ScreenToClient(m_hWnd, &m_ptMouse);

	ID3D11Resource* pResource = m_pGameInstance->Get_RT_Resource(TEXT("Target_Depth"));
	if (nullptr == pResource)
		return;

	m_pContext->CopyResource(m_pTexture2D, pResource);
}

_bool CPicking::isPicked(_float3* pOut)
{
	// Mouse 醫뚰몴??DepthDesc 異붿텧
	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	if (FAILED(m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &SubResource)))
		return false;

	_float4* pPoints = new _float4[m_iWinSizeX * m_iWinSizeY];
	pPoints = static_cast<_float4*>(SubResource.pData);

	_uint iIndex = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;

	_float4 DepthDesc = pPoints[iIndex];

	m_pContext->Unmap(m_pTexture2D, 0);

	// Picking??Object ?꾨떂
	if (0.f == DepthDesc.w)
		return false;

	// World濡?移섑솚
	_vector WorldPos = {};
	WorldPos = XMVectorSetX(WorldPos, m_ptMouse.x / (m_iWinSizeX * 0.5f) - 1.f);
	WorldPos = XMVectorSetY(WorldPos, m_ptMouse.y / (m_iWinSizeY * -0.5f) + 1.f);
	WorldPos = XMVectorSetZ(WorldPos, DepthDesc.x);
	WorldPos = XMVectorSetW(WorldPos, 1.f);

	XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
	XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

	XMStoreFloat3(pOut, WorldPos);

	return true;
}

CPicking* CPicking::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{
	CPicking* pInstance = new CPicking(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd, iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Create : Picking");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CPicking::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pTexture2D);
}
