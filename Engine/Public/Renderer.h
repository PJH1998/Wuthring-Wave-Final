#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CRenderer final : public CBase
{
private:
	explicit CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT		Initialize();
	HRESULT		Add_Render_Object(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	void		Render();

#ifdef _DEBUG
	HRESULT		Add_Render_Debug(class CComponent* pDebugComponent);
#endif

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };

	list<class CGameObject*>			m_RenderObjects[ENUM_CLASS(RENDERGROUP::END)];

	class CShader*						m_pShader = { nullptr };
	class CVIBuffer_Rect*				m_pVIBuffer = { nullptr };

	_float4x4							m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	_uint								m_iWinSizeX{}, m_iWinSizeY{};

#ifdef _DEBUG
	list<class CComponent*>				m_DebugComponents;
	_bool								m_isRenderDebug = { true };
#endif

private:
	// Viewport Size º¯°æ
	void		Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY);

private:
	void		Render_Priority();
	void		Render_Shadow();
	void		Render_NonBlend();
	void		Render_Light();
	void		Render_Combined();
	void		Render_NonLight();
	void        Render_Emissive();
	void        Render_DistortionObject();
	void        Render_Blur();
	void		Render_Blend();
	void        Render_Distortion();
	void		Render_UI();
	void		Render_Fade();

#ifdef _DEBUG
	void		Render_Debug();
#endif

private:
	HRESULT		Ready_RT();
	HRESULT		Ready_MRT();
	HRESULT		Ready_Shadow_DSV();


public:
	static		CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;

};

NS_END