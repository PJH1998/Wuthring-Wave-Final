#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;

class CShadow final : public CBase
{
private:
	explicit CShadow();
	virtual ~CShadow() = default;
	
public:
	const _float4x4*		Get_Matrix( D3DTS eType ) { return nullptr; }// &m_Matrices[ENUM_CLASS( eType )];}
	HRESULT					Bind_Shadow_Resource(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName);

	HRESULT					Bind_Shadow_Resource_Object(class CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iShadowMapIndex);
	HRESULT					Bind_Shadow_Resrouce_Renderer(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pDistanceName);
	_bool					IsIn_SplitFrustrum(const BoundingBox* ObjectVolume, _uint iShadowMapIndex);

	//Test
	const _matrix			Get_Matrix( D3DTS eType, _uint iIndex ) { return XMLoadFloat4x4( &m_Matrices[ENUM_CLASS( eType )][iIndex] ); }

public:
	HRESULT					Initialize(_float fWidth, _float fHeight);
	HRESULT					Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc, const _fvector& vAt);

	void					Update_Transform(const _fvector& vAt);
	void					Update_Shadow_ViewProj();

	void					Update_Test();
private:
	_float					m_fWidth{}, m_fHeight{};

	_uint					m_iNumSplits = {};
	_uint					m_iNumSplitDistances = {};

	SHADOW_LIGHT_DESC		m_MainShadowDesc = {};
	

	_float3					m_vLookDir = {};
	_float					m_fDistance = {};
	_float					m_fFar = {};
	vector<_float4x4>		m_Matrices[ENUM_CLASS(D3DTS::END)] = {};
//	_float4x4				m_Matrices[ENUM_CLASS(D3DTS::END)] = {};

	_matrix					m_CameraViewMatrix = {};					//
	
	_float4x4				m_ViewMatrix = {};							// Shadow Light View
	vector<_float4x4>		m_ProjMatrices;								// Shadow Light Proj * Num Splits

	vector<_float>			m_SplitDistances;							// 프러스텀들 길이
	vector<_float4>*		m_pSplitPoints = { nullptr };				// 나눈 프러스텀들 Points
	vector<_float4>*		m_pSplitPlanes = { nullptr };				// 프러스텀 평면들 컬릴용


private:
	CGameInstance*			m_pGameInstance = { nullptr };

private:
	_vector					Compute_Center(const vector<_float4>& FrustrumPoints);
	_float					Compute_Radius(const vector<_float4>& FrustrumPoints, _vector vCenterPos);

	void					Make_Matrices(const _float4* pFrustrumPoints);

	void					Make_ProjMatrices(const _float4* pFrustrumPoints);

	_matrix					Make_SplitViewMatrix( const vector<_float4>& SplitFrustrumPoints);
	_matrix					Make_SplitProjMatrix( const vector<_float4>& SplitFrustrumPoints, _uint iIndex);

	/*_matrix					Make_SplitViewMatrix(const _float3& vMaxExtents, const _float3& vMinExtents, _vector vCenterPos);
	_matrix					Make_SplitProjMatrix( const _float3& vMaxExtents, const _float3& vMinExtents );*/


	void					Make_SplitPlanes();

	_float					Compute_SplitDistances(_float fCameraNear, _float fCameraFar, _uint iIndex, _uint iNumSplit, _float fLambda);
	
public:
	static CShadow*			Create(_float fWidth, _float fHeight);
	virtual	void			Free() override;
};

NS_END