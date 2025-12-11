#pragma once
#include "Player_Define.h"
#include "RoverState_Enum.h"

NS_BEGIN(Client)
class CLogoMaleRover final : public CCharacter
{
public:
	enum LOGO_STATE
	{
		STATE_PICK = 0,
		STATE_PICK_END,
		STATE_HOLD,
		STATE_END
	};

#pragma region 0. LOGO
protected:
	explicit CLogoMaleRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLogoMaleRover(const CLogoMaleRover& Prototype);
	virtual ~CLogoMaleRover() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;
	virtual void	Render_OutLine() override;
#pragma endregion


#pragma region STATE
public:
	void Logo_Input();

public:
	virtual void Object_Func(const _wstring& wStrObjectTag) override;
#pragma endregion

private:
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsAnimationEnd = { false };
	_bool m_States[STATE_END] = {};


private:
	void Bind_Resources();

	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);

public:
	static		CLogoMaleRover* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

