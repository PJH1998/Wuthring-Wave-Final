#include "EnginePch.h"
#include "Shadow.h"

#include "Shader.h"
#include "GameInstance.h"

CShadow::CShadow()
	: m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pGameInstance);
}

void CShadow::Update_Transform(const _fvector& vAt)
{
	//_vector vDir = XMVector3Normalize(XMLoadFloat4(&m_MainShadowDesc.vDirection));

	//_vector vLightDir = XMVectorAdd(XMVectorScale(vDir, m_MainShadowDesc.fDistance), vAt);

	//_vector vEye = XMVectorSubtract(vAt, vLightDir);

	//XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)], XMMatrixLookAtLH(vEye, vAt, XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	//XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)], XMMatrixOrthographicLH(m_fWidth, m_fHeight, m_MainShadowDesc.fNear, m_MainShadowDesc.fFar));
}

void CShadow::Update_Shadow_ViewProj()
{
	const _float4* pCurrentWorldPoints = m_pGameInstance->Get_Frustrum_WorldPoints();

	m_CameraViewMatrix = m_pGameInstance->Get_TransformState_Matrix( D3DTS::VIEW );

	Make_Matrices( pCurrentWorldPoints );
}

void CShadow::Update_Test()
{
	const _float4* pCurrentWorldPoints = m_pGameInstance->Get_Frustrum_WorldPoints();

	m_CameraViewMatrix = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);

	Make_Matrices( pCurrentWorldPoints );
}

HRESULT CShadow::Bind_Shadow_Resource(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName)
{
	if (FAILED(pShader->Bind_Matrix(pViewName, Get_Matrix(D3DTS::VIEW))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix(pProjName, Get_Matrix(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Value(pFarName, &m_MainShadowDesc.fFar, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

HRESULT CShadow::Bind_Shadow_Resource_Object(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iShadowMapIndex)
{
	ASSERT_CRASH(pShader);

	if (FAILED(pShader->Bind_Matrix(pViewName, &m_Matrices[ENUM_CLASS(D3DTS::VIEW)][iShadowMapIndex])))
		CRASH("Failed Bind Shadow View Matrix");

	if (FAILED(pShader->Bind_Matrix(pProjName, &m_Matrices[ENUM_CLASS(D3DTS::PROJ)][iShadowMapIndex])))
		CRASH("Failed Bind Shadow Proj Matrix");

	return S_OK;
}

HRESULT CShadow::Bind_Shadow_Resrouce_Renderer(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pDistanceName)
{
	ASSERT_CRASH(pShader);

	if (FAILED(pShader->Bind_Matrices(pViewName, m_Matrices[ENUM_CLASS(D3DTS::VIEW)].data(), m_iNumSplits)))
		CRASH("Failed Bind Shadow View Matrix");

	if (FAILED(pShader->Bind_Matrices(pProjName, m_Matrices[ENUM_CLASS(D3DTS::PROJ)].data(), m_iNumSplits)))
		CRASH("Failed Bind Shadow Proj Matrix");

	if(FAILED(pShader->Bind_Value(pDistanceName, reinterpret_cast<_float*>(&m_SplitDistances[0]), sizeof(_float) * 4)))
		CRASH("Failed Bind Shadow Split Distance");

	return S_OK;
}

_bool CShadow::IsIn_SplitFrustrum(const BoundingBox* ObjectVolume, _uint iShadowMapIndex)
{
	for (_uint i = 0; i < 6; i++)
	{
		PlaneIntersectionType Result = ObjectVolume->Intersects(XMLoadFloat4(&m_pSplitPlanes[iShadowMapIndex][i]));
		
		if (Result == PlaneIntersectionType::FRONT)
			return false;
	}
	return true;
}

HRESULT CShadow::Initialize(_float fWidth, _float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	//TEST
	m_iNumSplits = g_iNumCascade;
	m_iNumSplitDistances = m_iNumSplits + 1;

	m_Matrices[ENUM_CLASS( D3DTS::VIEW )].resize( m_iNumSplits );
	m_Matrices[ENUM_CLASS( D3DTS::PROJ)].resize( m_iNumSplits );

	m_ProjMatrices.resize(m_iNumSplits);
	m_SplitDistances.resize(m_iNumSplitDistances, 0.f);

	m_pSplitPoints = new vector<_float4>[m_iNumSplits];

	for (_uint i = 0; i < m_iNumSplits; i++)
		m_pSplitPoints[i].resize(8, _float4(0.f, 0.f, 0.f, 0.f));

	return S_OK;
}

HRESULT CShadow::Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc, const _fvector& vAt)
{
	memcpy(&m_MainShadowDesc, &Desc, sizeof(SHADOW_LIGHT_DESC));
	Update_Test();

//	Update_Transform(vAt);

	return S_OK;
}

_vector CShadow::Compute_Center(const vector<_float4>& FrustrumPoints)
{
	_vector vCenterPos = XMVectorZero();

	for (_uint i = 0; i < 8; i++)
		vCenterPos = XMVectorAdd(vCenterPos, XMLoadFloat4(&FrustrumPoints[i]));

	vCenterPos = XMVectorSetW(XMVectorScale(vCenterPos, ( 1.f / 8.f )),1.f);
	
	return vCenterPos;
}

_float CShadow::Compute_Radius(const vector<_float4>& FrustrumPoints, _vector vCenterPos )
{
	_float fRadius = 0.f;

	_float fDistance = 0.f;
	for (_uint i = 0; i < 8; i++)
	{
		fDistance = XMVectorGetX( XMVector3Length( XMVectorSubtract( XMLoadFloat4( &FrustrumPoints[i] ), vCenterPos ) ) );

		fRadius = max(fDistance, fRadius);
	}

	fRadius = ceil( fRadius * 16.f ) / 16.f;

	return fRadius;
}

void CShadow::Make_Matrices(const _float4* pFrustrumPoints)
{
	_float fCameraNear = m_pGameInstance->Get_CurrentCamera_Near();		// Camera 교체시 한번만 받아오고 싶
	_float fCameraFar = m_pGameInstance->Get_CurrentCamera_Far();

	for (_uint i = 0; i < m_iNumSplitDistances; i++)
		m_SplitDistances[i] = Compute_SplitDistances(fCameraNear, fCameraFar, i, m_iNumSplits, 0.5f);

	_float fSplitNear = {};
	_float fSplitFar = {};
	_float fNearRatio = {};
	_float fFarRatio = {};

	for (_uint j = 0; j < m_iNumSplits; j++)
	{
		fSplitNear = m_SplitDistances[j];
		fSplitFar = m_SplitDistances[j + 1];

		fNearRatio = (fSplitNear - fCameraNear) / (fCameraFar - fCameraNear);
		fFarRatio = (fSplitFar - fCameraNear) / (fCameraFar - fCameraNear);


		for (_uint k = 0; k < 4; k++)				// 프러스텀 나눠서 새로운 프러스텀 만들기
		{
			XMStoreFloat4(&m_pSplitPoints[j][k], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fNearRatio));
			XMStoreFloat4(&m_pSplitPoints[j][k + 4], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fFarRatio));

			XMStoreFloat4( &m_pSplitPoints[j][k], XMVector3TransformCoord( XMLoadFloat4(&m_pSplitPoints[j][k]),m_CameraViewMatrix));
			XMStoreFloat4( &m_pSplitPoints[j][k+4], XMVector3TransformCoord( XMLoadFloat4( &m_pSplitPoints[j][k+4] ), m_CameraViewMatrix ) );
		}

		_vector vCenterPos = Compute_Center( m_pSplitPoints[j]);
		_float	fMaxRadius = Compute_Radius( m_pSplitPoints[j], vCenterPos );
		_float	fMinRadius = fMaxRadius * -1.f;
		
		_float3 vMaxExtents = _float3( fMaxRadius, fMaxRadius, fMaxRadius );
		_float3 vMinExtents = _float3( fMinRadius, fMinRadius, fMinRadius );

		XMStoreFloat4x4( &m_Matrices[ENUM_CLASS( D3DTS::VIEW )][j], Make_SplitViewMatrix( m_pSplitPoints[j]) );
		XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)][j], Make_SplitProjMatrix( m_pSplitPoints[j], j )); // 나눈 프러
	//	XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)][j], Make_SplitViewMatrix(vMaxExtents, vMinExtents, vCenterPos));
	//	XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)][j], Make_SplitProjMatrix(vMaxExtents, vMinExtents)); // 나눈 프러스텀으로 투영행렬 만들기
	}
}

_matrix CShadow::Make_SplitViewMatrix( const vector<_float4>& SplitFrustrumPoints )
{
	_vector vCenterPos = Compute_Center( SplitFrustrumPoints );

	_float	fMaxRadius = Compute_Radius( SplitFrustrumPoints, vCenterPos );
	_float	fMinRadius = fMaxRadius * -1.f;

	_vector vDir = XMVector3Normalize( XMLoadFloat4( &m_MainShadowDesc.vDirection ) );

	if (XMVectorGetX( XMVector3Length( vDir ) ) == 0.f)
		vDir = XMVectorSet( 0.f, -1.f, 0.1f, 0.f );

	_float fFar = fMaxRadius - fMinRadius;

	_vector vEye = XMVectorSubtract( vCenterPos, XMVectorScale( vDir, fFar) );

	m_fFar = XMVectorGetX( XMVector3Length( XMVectorSubtract( vEye, vCenterPos ) ) );

	_matrix ViewMatrix = XMMatrixLookAtLH( vEye, vCenterPos, XMVectorSet( 0.f, 1.f, 0.f, 0.f ) );

	return ViewMatrix;
}

_matrix CShadow::Make_SplitProjMatrix( const vector<_float4>& SplitFrustrumPoints , _uint iIndex )
{
	_float4 vShadowViewPoints[8] = {};
	
	_float fMinX = FLT_MAX, fMaxX = FLT_MAX * -1.f;
	_float fMinY = FLT_MAX, fMaxY = FLT_MAX * -1.f;
	_float fMinZ = FLT_MAX, fMaxZ = FLT_MAX * -1.f;
	
	for (_uint i = 0; i < 8; i++)
	{
		XMStoreFloat4(&vShadowViewPoints[i], XMVector3TransformCoord(XMLoadFloat4(&SplitFrustrumPoints[i]), XMLoadFloat4x4(&m_Matrices[ENUM_CLASS( D3DTS::VIEW )][iIndex] ))); // 나눈 프러스텀 내 뷰행렬로 올리기
	
		fMinX = min(fMinX, vShadowViewPoints[i].x);
		fMaxX = max(fMaxX, vShadowViewPoints[i].x);
	
		fMinY = min(fMinY, vShadowViewPoints[i].y);
		fMaxY = max(fMaxY, vShadowViewPoints[i].y);
	
		fMinZ = min(fMinZ, vShadowViewPoints[i].z);
		fMaxZ = max(fMaxZ, vShadowViewPoints[i].z);
	}
	
	_float fNear = fMinZ;
	_float fFar = fMaxZ;
	
	//if (fNear > fFar)
	//	swap(fNear, fFar);
	
	return XMMatrixOrthographicOffCenterLH(fMinX, fMaxX, fMinY, fMaxY, fNear, fFar);		// 뷰행렬 올린 프러스텀에서 Min,Max, Near, Far 구해서 투영행렬 생성	
}

//_matrix CShadow::Make_SplitViewMatrix( const _float3& vMaxExtents, const _float3& vMinExtents, _vector vCenterPos )
//{
//	_vector vDir = XMVector3Normalize( XMLoadFloat4( &m_MainShadowDesc.vDirection ) );
//
//	if (XMVectorGetX( XMVector3Length( vDir ) ) == 0.f)
//		vDir = XMVectorSet( 0.f, -1.f, 0.f, 0.f );
//
//	_float fFar = vMaxExtents.z - vMinExtents.z;
//
//	_vector vEye = XMVectorSubtract( vCenterPos, XMVectorScale( vDir, fFar) );
//
//	m_fFar = XMVectorGetX( XMVector3Length( XMVectorSubtract( vEye, vCenterPos ) ) );
//
//	_matrix ViewMatrix = XMMatrixLookAtLH( vEye, vCenterPos, XMVectorSet( 0.f, 1.f, 0.f, 0.f ) );
//
//	return ViewMatrix;
//}
//_matrix CShadow::Make_SplitProjMatrix( const _float3& vMaxExtents, const _float3& vMinExtents )
//{
//	_matrix ProjMatrix = XMMatrixOrthographicOffCenterLH( vMinExtents.x, vMaxExtents.x, vMinExtents.y, vMaxExtents.y, vMinExtents.z, m_fFar);
//	
//	return ProjMatrix;
//}

/*
void CShadow::Make_ProjMatrices(const _float4* pFrustrumPoints)
{
	_float fCameraNear = m_pGameInstance->Get_CurrentCamera_Near();
	_float fCameraFar = m_pGameInstance->Get_CurrentCamera_Far();

	for (_uint i = 0; i < m_iNumSplitDistances; i++)
		m_SplitDistances[i] = Compute_SplitDistances(fCameraNear, fCameraFar, i, m_iNumSplits, 0.7f);

	_float fSplitNear = {};
	_float fSplitFar = {};
	_float fNearRatio = {};
	_float fFarRatio = {};

	for (_uint j = 0; j < m_iNumSplits; j++)
	{
		fSplitNear = m_SplitDistances[j];
		fSplitFar = m_SplitDistances[j + 1];

		fNearRatio = ( fSplitNear - fCameraNear ) / ( fCameraFar - fCameraNear );
		fFarRatio = ( fSplitFar - fCameraNear ) / ( fCameraFar - fCameraNear );

		
		for (_uint k = 0; k < 4; k++)				// 프러스텀 나눠서 새로운 프러스텀 만들기
		{
			XMStoreFloat4(&m_pSplitPoints[j][k], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fNearRatio));
			XMStoreFloat4(&m_pSplitPoints[j][k+4], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fFarRatio));
		}

		XMStoreFloat4x4(&m_ProjMatrices[j], Make_SplitProjMatrix(m_pSplitPoints[j])); // 나눈 프러스텀으로 투영행렬 만들기
	}
}
*/


//
//_matrix CShadow::Make_SplitProjMatrix(const vector<_float4>& SplitPoints)
//{
//	_float4 vShadowViewPoints[8] = {};
//
//	_float fMinX = FLT_MAX, fMaxX = FLT_MAX * -1.f;
//	_float fMinY = FLT_MAX, fMaxY = FLT_MAX * -1.f;
//	_float fMinZ = FLT_MAX, fMaxZ = FLT_MAX * -1.f;
//
//	for (_uint i = 0; i < 8; i++)
//	{
//		XMStoreFloat4(&vShadowViewPoints[i], XMVector3TransformCoord(XMLoadFloat4(&SplitPoints[i]), XMLoadFloat4x4(&m_ViewMatrix))); // 나눈 프러스텀 내 뷰행렬로 올리기
//
//		fMinX = min(fMinX, vShadowViewPoints[i].x);
//		fMaxX = max(fMaxX, vShadowViewPoints[i].x);
//
//		fMinY = min(fMinY, vShadowViewPoints[i].y);
//		fMaxY = max(fMaxY, vShadowViewPoints[i].y);
//
//		fMinZ = min(fMinZ, vShadowViewPoints[i].z);
//		fMaxZ = max(fMaxZ, vShadowViewPoints[i].z);
//	}
//
//	_float fNear = fMinZ;
//	_float fFar = fMaxZ;
//
//	//if (fNear > fFar)
//	//	swap(fNear, fFar);
//
//	return XMMatrixOrthographicOffCenterLH(fMinX, fMaxX, fMinY, fMaxY, fNear, fFar);		// 뷰행렬 올린 프러스텀에서 Min,Max, Near, Far 구해서 투영행렬 생성
//}

void CShadow::Make_SplitPlanes()
{
	for (_uint i = 0; i < m_iNumSplits; i++)
	{
		XMStoreFloat4(&m_pSplitPlanes[i][0],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][1]), XMLoadFloat4(&m_pSplitPoints[i][5]), XMLoadFloat4(&m_pSplitPoints[i][6])));
		XMStoreFloat4(&m_pSplitPlanes[i][1],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][4]), XMLoadFloat4(&m_pSplitPoints[i][0]), XMLoadFloat4(&m_pSplitPoints[i][3])));
		XMStoreFloat4(&m_pSplitPlanes[i][2],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][4]), XMLoadFloat4(&m_pSplitPoints[i][5]), XMLoadFloat4(&m_pSplitPoints[i][1])));
		XMStoreFloat4(&m_pSplitPlanes[i][3],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][3]), XMLoadFloat4(&m_pSplitPoints[i][2]), XMLoadFloat4(&m_pSplitPoints[i][6])));
		XMStoreFloat4(&m_pSplitPlanes[i][4],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][5]), XMLoadFloat4(&m_pSplitPoints[i][4]), XMLoadFloat4(&m_pSplitPoints[i][7])));
		XMStoreFloat4(&m_pSplitPlanes[i][5],
			XMPlaneFromPoints(XMLoadFloat4(&m_pSplitPoints[i][0]), XMLoadFloat4(&m_pSplitPoints[i][1]), XMLoadFloat4(&m_pSplitPoints[i][2])));
	}
}

_float CShadow::Compute_SplitDistances(_float fCameraNear, _float fCameraFar, _uint iIndex, _uint iNumSplit, _float fLambda)
{
	_float fSplitDistance = {};
	_float fLinearSplit = fCameraNear + ( ( fCameraFar - fCameraNear ) * ( static_cast<_float>( iIndex ) / static_cast<_float>( iNumSplit ) ) );	// Linear 보간
	_float fLogSplit = fCameraNear * powf(fCameraFar / fCameraNear, static_cast<_float>( iIndex ) / static_cast<_float>( iNumSplit ));				// Log	 보간

	fSplitDistance = ( fLambda * fLogSplit ) + ( 1.f - fLambda ) * fLinearSplit;			// Linear와 Log Lerp

	return fSplitDistance;
}

CShadow* CShadow::Create(_float fWidth, _float fHeight)
{
	CShadow* pInstance = new CShadow();

	if (FAILED(pInstance->Initialize(fWidth, fHeight)))
	{
		MSG_BOX("Failed to Create : Shadow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShadow::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Delete_Array(m_pSplitPoints);
	Safe_Delete_Array(m_pSplitPlanes);
}
