#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_UI final : public CLevel
{
private:
	explicit CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_UI() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:

	vector<class CCustom_UI*> m_vecCustomUIs = {};

public:
	static		CLevel_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END