#pragma once
#include "Editor_Define.h"
#include "GameObject.h"


NS_BEGIN(Editor)

class CDecal final : public CGameObject
{
public:
	typedef struct tagDecalDesc : Engine::EFFECT_DESC
	{
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float LifeTime = {};
	}DECAL_DESC;

private:
	CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecal(const CDecal& Prototype);
	virtual ~CDecal() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Root_Transform(_fmatrix WorldMatrix);
	

private:
	DECAL_DESC					m_tDesc = {};

	_wstring					m_wstrMyTag = {};
	_float4						m_vColor = {};
	_float						m_LifeTime = {};

public:
	static CDecal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END