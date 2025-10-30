#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Test_UI :
	public CLevel
{
private:
	explicit CLevel_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Test_UI() = default;

public:
	virtual		HRESULT		Initialize() override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Render() override;

public:
	static		CLevel_Test_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END