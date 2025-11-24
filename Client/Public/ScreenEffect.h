#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

class CScreenEffect abstract : public CGameObject
{
protected:
	CScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CScreenEffect(const CScreenEffect& Prototype);
	virtual ~CScreenEffect() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override; 
	virtual		void		Render() override;

protected:
	_float4x4				m_ViewMatrix = {};
	_float4x4				m_ProjMatrix = {};

	_float2					m_vWinSize = {};


	_float2					m_vEffectTime = {};
	_float					m_fCurrentTime = {};

protected:
	void					Setting_Scale(_float fSizeX, _float fSizeY);
	void					Setting_Pos(_float fPosX, _float fPosY);

public:
	virtual CGameObject*	Clone(void* pArg) PURE;
	virtual void			Free() override;
};

NS_END