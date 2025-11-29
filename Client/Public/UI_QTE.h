#pragma once
#include "UI_Image.h"

NS_BEGIN(Client)

class CUI_QTE final : public CUI_Image
{
public:
	explicit CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_QTE(const CUI_QTE& Prototype);
	virtual ~CUI_QTE() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

private:
	void			PreAssign_ChildUIs();

private:
	//CCustom_UI* m_pRUI? = ;
	CCustom_UI*		m_pRUI_All					= nullptr;
	CCustom_UI*		m_pUI_SectorA_KeyGuide		= nullptr;
	CCustom_UI*		m_pUI_SectorA_BG			= nullptr;
	CCustom_UI*		m_pUI_SectorA_FG			= nullptr;


	_bool			m_IsGoinDisabled = false;
	_float			m_fDisableTimer = 0.f;

	_uint			m_iAnimOrder = 0;

	class CGameSystem* m_pGameSystem = { nullptr };

public:
	static CUI_QTE*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END