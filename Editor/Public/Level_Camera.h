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

private:
	class CMap_Interface*		m_pMapInterface = { nullptr };

	class CSpringCamera_Edit*	m_pSpringCamera = { nullptr };

private:
	_bool						m_isMapInterface = { false };

private:
	void						Ready_Camera();
	void						Ready_Dummy();

public:
	static		CLevel_Camera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END