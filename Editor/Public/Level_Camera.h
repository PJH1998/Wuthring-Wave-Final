#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_Camera final : public CLevel
{
private:
	explicit CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Camera() = default;

public:
	virtual		HRESULT		Initialize() override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Render() override;

public:
	static		CLevel_Camera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END