#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)
class CNapal final : public CActor
{
public:
	typedef struct tagNapalDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPos;
		_float3 vInitRot;
	}NAPALDESC;

private:
	explicit CNapal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNapal(const CNapal& Prototype);
	virtual ~CNapal() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_Shadow() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override; // 임시

private:
	_string					m_strAnimationTag[2];

	_uint					m_iCount{};
	_uint					m_iIndex{};
	_bool					m_isRender{};
	_float4					m_vBaseColor{};

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(NAPALDESC* pDesc);

public:
	static CNapal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
