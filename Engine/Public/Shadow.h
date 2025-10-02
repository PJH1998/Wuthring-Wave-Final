#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CShadow final : public CBase
{
private:
	explicit CShadow();
	virtual ~CShadow() = default;

public:
	void						Update_Transform(const _fvector& vAt);
	const _float4x4*		Get_Matrix(D3DTS eType) { return &m_Matrices[ENUM_CLASS(eType)]; }
	HRESULT					Bind_Shadow_Resource(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName);

public:
	HRESULT					Initialize(_float fWidth, _float fHeight);
	HRESULT					Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc);

private:
	_float							m_fWidth{}, m_fHeight{};

	SHADOW_LIGHT_DESC	m_MainShadowDesc = {};
	_float3						m_vLookDir = {};
	_float							m_fDistance = {};

	_float4x4						m_Matrices[ENUM_CLASS(D3DTS::END)] = {};

public:
	static		CShadow*	Create(_float fWidth, _float fHeight);
	virtual		void			Free() override;
};

NS_END