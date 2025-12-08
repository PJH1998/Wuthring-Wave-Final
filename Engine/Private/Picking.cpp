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

	m_pPoints = new _float4[m_iWinSizeX * m_iWinSizeY];
	//m_WorldPoints = new _float4[m_iWinSizeX * m_iWinSizeY];
    return S_OK;
}

void CPicking::Update()
{
	GetCursorPos(&m_ptMouse);
	ScreenToClient(m_hWnd, &m_ptMouse);

	ID3D11Resource* pResource = m_pGameInstance->Get_RT_Resource(TEXT("RT_Depth"));
	if (nullptr == pResource)
		return;

	m_pContext->CopyResource(m_pTexture2D, pResource);
}

_bool CPicking::isPicked(_float3* pOut)
{
	_uint MousePos = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;
	if (MousePos > m_iWinSizeX * m_iWinSizeY)
		return false;

	// Mouse ��ǥ�� DepthDesc ����
	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	if (FAILED(m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &SubResource)))
		return false;

	memcpy(m_pPoints, SubResource.pData, sizeof(_float4) * m_iWinSizeX * m_iWinSizeY);

	_uint iIndex = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;

	_float4 DepthDesc = m_pPoints[iIndex];

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

	WorldPos = XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
	WorldPos = XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

	XMStoreFloat3(pOut, WorldPos);
	return true;
}

_bool CPicking::GetCenterPos(_float3* pOut)
{
	_uint MousePos = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;
	if (MousePos > m_iWinSizeX * m_iWinSizeY)
		return false;

	// Mouse ��ǥ�� DepthDesc ����
	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	if (FAILED(m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &SubResource)))
		return false;

	memcpy(m_pPoints, SubResource.pData, sizeof(_float4) * m_iWinSizeX * m_iWinSizeY);

	_uint iIndex = ((m_iWinSizeY / 2 - 10) * m_iWinSizeX) + m_iWinSizeX / 2 - 100;

	_float4 DepthDesc = m_pPoints[iIndex];

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

	WorldPos = XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
	WorldPos = XMVector3TransformCoord(WorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

	XMStoreFloat3(pOut, WorldPos);
	return true;
}

_bool CPicking::Get_Points(_float fRange, vector<_float4>& pOut,_uint* NumPixels,_float4* pOutMousePos)
{

	_uint MousePos = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;
	if (MousePos > m_iWinSizeX * m_iWinSizeY)
		return false;

	// Mouse ��ǥ�� DepthDesc ����
	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	if (FAILED(m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &SubResource)))
		return false;

	memcpy(m_pPoints, SubResource.pData, sizeof(_float4) * m_iWinSizeX * m_iWinSizeY);

	_uint iIndex = m_ptMouse.y * m_iWinSizeX + m_ptMouse.x;

	_float4 DepthDesc = m_pPoints[iIndex];

	m_pContext->Unmap(m_pTexture2D, 0);

	if (0.f == DepthDesc.w)
		return false;

	//스크린 기준 마우스 찍은 점 기준으로 옆으로 1칸 2칸씩 움직이면서 내가 지정한 월드 길이보다 길어지면 거기서 스탑. 스크린 상에서 마우스 점이랑 그 점이랑 
	//길이 비교해서 범위가 되는 지점 생성. 벡터로 저장 후 벡터에 있는 점들을 월드까지 내리기? -> 카메라가 가까워지면 연산 많이함.
	
	//현재 인스턴스 한 놈이 자꾸 카메라 위치로 나오는 오류 있음.

	_vector MouseWorldPos = {};

	MouseWorldPos = XMVectorSetX(MouseWorldPos, m_ptMouse.x / (m_iWinSizeX * 0.5f) - 1.f);
	MouseWorldPos = XMVectorSetY(MouseWorldPos, m_ptMouse.y / (m_iWinSizeY * -0.5f) + 1.f);
	MouseWorldPos = XMVectorSetZ(MouseWorldPos, DepthDesc.x);
	MouseWorldPos = XMVectorSetW(MouseWorldPos, 1.f);

	MouseWorldPos = XMVector3TransformCoord(MouseWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
	MouseWorldPos = XMVector3TransformCoord(MouseWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));
	XMStoreFloat4(pOutMousePos, MouseWorldPos);
	_float4 TempPoint = {};
	_int PixelRange = {};
	_uint ViewPorts = { 0 };
	D3D11_VIEWPORT ViewPort;
	m_pContext->RSGetViewports(&ViewPorts, &ViewPort);
	ViewPort.MinDepth;
	ViewPort.MaxDepth;
	for (_uint i=0; i< m_iWinSizeX; ++i)
	{
		TempPoint = m_pPoints[iIndex - i];
		_vector TempWorldPos = {};

		TempWorldPos = XMVectorSetX(TempWorldPos, (m_ptMouse.x - i) / (m_iWinSizeX * 0.5f) - 1.f);
		TempWorldPos = XMVectorSetY(TempWorldPos, m_ptMouse.y / (m_iWinSizeY * -0.5f) + 1.f);
		TempWorldPos = XMVectorSetZ(TempWorldPos, TempPoint.x);
		TempWorldPos = XMVectorSetW(TempWorldPos, 1.f);

		TempWorldPos = XMVector3TransformCoord(TempWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
		TempWorldPos = XMVector3TransformCoord(TempWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

		_vector vLength = TempWorldPos - MouseWorldPos;
		_float Dist = XMVectorGetX(XMVector3Length(vLength));
		//_float Dist = vLength.m128_f32[0] * vLength.m128_f32[0] + vLength.m128_f32[2] * vLength.m128_f32[2];

		if (Dist > fRange)
		{
			PixelRange = i;
			break;
		}
	}

	m_WorldPoints.clear();
	_uint Index = {};
	for (_int yOffset = -PixelRange; yOffset <= PixelRange; ++yOffset)
	{
		for (_int xOffset = -PixelRange; xOffset <= PixelRange; ++xOffset)
		{

			_uint SampleX = m_ptMouse.x + xOffset;
			_uint SampleY = m_ptMouse.y + yOffset;

			if (SampleX < 0 || SampleX >= m_iWinSizeX || SampleY < 0 || SampleY >= m_iWinSizeY)
				continue;

			TempPoint = m_pPoints[SampleY * m_iWinSizeX + SampleX];

			if (TempPoint.w == 0.f)
				continue;

			_vector TempWorldPos = {};

			TempWorldPos = XMVectorSetX(TempWorldPos, SampleX / (m_iWinSizeX * 0.5f) - 1.f);
			TempWorldPos = XMVectorSetY(TempWorldPos, SampleY / (m_iWinSizeY * -0.5f) + 1.f);
			TempWorldPos = XMVectorSetZ(TempWorldPos, TempPoint.x);
			TempWorldPos = XMVectorSetW(TempWorldPos, 1.f);

			TempWorldPos = XMVector3TransformCoord(TempWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
			TempWorldPos = XMVector3TransformCoord(TempWorldPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

			if (TempWorldPos.m128_f32[3] == 0.f)
				continue;

			_float4 Temp;
			XMStoreFloat4(&Temp, TempWorldPos);
			m_WorldPoints.push_back(Temp);
		}
	}
	if (m_WorldPoints.empty())
		return false;


	/*for (_uint y = 0; y < m_iWinSizeY; ++y)
	{
		for (_uint x = 0; x < m_iWinSizeX; ++x)
		{
			_uint PixelIndex = y * m_iWinSizeX + x;
			_float4 Depth = m_pPoints[y * m_iWinSizeX + x];

			if (0.f == Depth.w)
			{
				m_WorldPoints[PixelIndex] = _float4(0.f, 0.f, 0.f, 0.f);
				continue;
			}

			_vector PixelPos = {};
			PixelPos = XMVectorSetX(PixelPos, x / (m_iWinSizeX * 0.5f) - 1.f);
			PixelPos = XMVectorSetY(PixelPos, y / (m_iWinSizeY * -0.5f) + 1.f);
			PixelPos = XMVectorSetZ(PixelPos, Depth.x);
			PixelPos = XMVectorSetW(PixelPos, 1.f);

			PixelPos = XMVector3TransformCoord(PixelPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ));
			PixelPos = XMVector3TransformCoord(PixelPos, m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW));

			_vector Delta = MouseWorldPos - PixelPos;

			_float Dist= Delta.m128_f32[0] * Delta.m128_f32[0] + Delta.m128_f32[2] * Delta.m128_f32[2];
			if (Dist < fRange)
				XMStoreFloat4(&m_WorldPoints[PixelIndex], PixelPos);
			else
				XMStoreFloat4(&m_WorldPoints[PixelIndex], XMVectorSet(0.f, 0.f, 0.f, 0.f));
		}
	}*/
	pOut = m_WorldPoints;
	*NumPixels = PixelRange;
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

	Safe_Delete_Array(m_pPoints);
	//Safe_Delete_Array(m_WorldPoints);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pTexture2D);
}
