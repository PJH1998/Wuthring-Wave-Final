#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect_Instance;
class CShader;
NS_END

NS_BEGIN(Client)

class CGalbrenaUlti_SFX_Slash final : public CScreenEffect
{
private:
	typedef struct SlashData
	{
		_float2 vSize;
		_float fRotateRadian;
	}SLASH_DATA;

private:
	CGalbrenaUlti_SFX_Slash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrenaUlti_SFX_Slash(const CGalbrenaUlti_SFX_Slash& Prototype);
	virtual ~CGalbrenaUlti_SFX_Slash() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	CVIBuffer_Rect_Instance*	m_pVIBuffer = { nullptr };
	CShader*					m_pShader = { nullptr };

	SLASH_DATA					m_SlashData[5] = {};

	_float2						m_vPivot = {};

	_float2						m_vUpdateTime = {};

	_float2						m_vDrawRadians = {};
	_float						m_fCurrentRadian = {};

	_float2						m_vScale = {};
	_float						m_fCurrentScale = {};

	_float3						m_vColor = {};

private:
	HRESULT						Ready_Components();
	void						Ready_SlashData();
	void						Update_Instance();
	VTXINSTANCE_RECT			Make_Instance(_float2 vPivot, _float fSizeX, _float fSizeY, _float fRotateZ);

public:
	static CGalbrenaUlti_SFX_Slash*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END