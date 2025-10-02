#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CRenderer final : public CBase
{
private:
	explicit CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	HRESULT Add_Render_Object(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Render();
	HRESULT Add_LUT(const _wstring& strLUTTag, const _tchar* pFilePath);
	HRESULT Change_LUT(const _wstring& strLUTTag);

#ifdef _DEBUG
	HRESULT Add_Render_Debug(class CComponent* pDebugComponent);
#endif

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*					m_pContext = { nullptr };
	class CGameInstance*					m_pGameInstance = { nullptr };
	ID3D11DepthStencilView*				m_pShadowDSV = { nullptr };

	map<const _wstring, ID3D11ShaderResourceView*> m_LUTSRVs;
	ID3D11ShaderResourceView*			m_pMainLUTSRV = { nullptr };

	list<class CGameObject*>				m_RenderObjects[ENUM_CLASS(RENDERGROUP::END)];

	class CShader*								m_pShader = { nullptr };
	class CVIBuffer_Rect*						m_pVIBuffer = { nullptr };
	_float4x4		m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};

	_uint											m_iWinSizeX{}, m_iWinSizeY{};

#ifdef _DEBUG
	list<class CComponent*>				m_DebugComponents;
	_bool											m_isRenderDebug = { true };
#endif

private:
	void		Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY);

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();
	HRESULT Render_Light();
	HRESULT Render_Combined();
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_Emissive();
	HRESULT Render_Blur();
	HRESULT Render_UI();
	HRESULT Render_Fade();

#ifdef _DEBUG
	HRESULT Render_Debug();
#endif

private:
	HRESULT		Ready_RT();
	HRESULT		Ready_MRT();
	HRESULT		Ready_Shadow_DSV();

public:
	static		CRenderer*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;

};

NS_END