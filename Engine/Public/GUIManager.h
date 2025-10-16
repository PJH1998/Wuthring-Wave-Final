#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGUIManager final : public CBase
{
private:
	explicit CGUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CGUIManager() = default;

public:
	ImGuiContext*		Get_ImGuiContext() { return ImGui::GetCurrentContext(); }
	void					Add_GUI_Func(function<void()> func);
	void					Clear_Func();

public:
	HRESULT				Initialize(HWND hWnd);
	void					Update();
	void					Render();

private:
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	ID3D11RenderTargetView*	m_pMainRTV = { nullptr };
	ID3D11DepthStencilView*	m_pMainDSV = { nullptr };

	vector<function<void()>>	m_Functions;

public:
	static		CGUIManager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	virtual		void				Free() override;
};

NS_END