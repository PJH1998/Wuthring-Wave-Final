#pragma once
#include "Player_Define.h"

NS_BEGIN(Client)
class CPlayerAugusta final : public CPlayer
{
public:
	enum PARTTYPE : _uint
	{
		PART_WEAPON = 0,
		PART_SHIELD = 1,
		TYPE_END
	};

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

private:
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };

private:
	// Runtime 도중 필요한 값에 대한 준비.
	void Bind_Resources();

	// 초기 값에 대한 준비.
	void Ready_Components(const PLAYER_DESC* pDesc);
	void Ready_Variables(const PLAYER_DESC* pDesc);
	void Ready_Positions(const PLAYER_DESC* pDesc);
	void Ready_PartObjects(const PLAYER_DESC* pDesc);

public:
	static		CPlayerAugusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

