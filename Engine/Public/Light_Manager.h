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
	const LIGHT_DESC*		Get_LightDesc(const _wstring& strLightTag);

public:
	HRESULT					Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	HRESULT					SetUp_Light(class CShader* pShader, const _wstring& strLightTag, LIGHT_DESC::TYPE eType);
	HRESULT					Clear_Light();
	HRESULT					Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

private:
	map<_wstring, CLight*>		m_Lights;

public:
	static		CLight_Manager*	Create();
	virtual		void					Free() override;
};

NS_END