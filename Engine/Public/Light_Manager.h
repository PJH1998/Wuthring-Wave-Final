#pragma once
#include "Base.h"

#include "Light.h"

NS_BEGIN(Engine)

class CLight_Manager final : public CBase
{
private:
	explicit CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_LightDesc(const _wstring& strLightTag);
#ifdef _DEBUG
	LIGHT_DESC*		Get_LightDesc_For_Map(const _wstring& strLightTag);
#endif

public:
	HRESULT					Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	HRESULT					Clear_Light();
	HRESULT					Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

private:

	map<_wstring, CLight*>		m_Lights;
	LIGHT_DESC*					m_pShadowLight;

public:
	static		CLight_Manager*		Create();
	virtual		void				Free() override;
};

NS_END