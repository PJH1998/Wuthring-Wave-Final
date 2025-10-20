#pragma once
#include "Player_Define.h"

NS_BEGIN(Client)
class CPlayerAugusta final : public CPlayer
{
#pragma region 기본 함수
protected:
	explicit CPlayerAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPlayerAugusta(const CPlayerAugusta& Prototype);
	virtual ~CPlayerAugusta() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;
#pragma endregion

public:

private:
	HRESULT Ready_Components(const PLAYER_DESC* pDesc);

public:
	static		CPlayerAugusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

