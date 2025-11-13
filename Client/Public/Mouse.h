#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

class CMouse final : public CGameObject
{
private:
	explicit CMouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMouse(const CMouse& Prototype);
	virtual ~CMouse() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	_int						m_iCursorPosX{}, m_iCursorPosY{};

public:
	static		CMouse*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END