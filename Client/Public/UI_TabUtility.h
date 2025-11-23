#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_TabUtility final : public CCustom_UI
{
public:
	explicit CUI_TabUtility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_TabUtility(const CUI_TabUtility& Prototype);
	virtual ~CUI_TabUtility() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;
	
public:
	// 종료 애니메이션 재생 + 현재 선택한 종류의 인덱스 반환
	_uint			Req_OffTabUI()	{ m_IsGoinDisabled = true; return m_iSelectedIndex; }

private:
	void			Update_MouseSelection();

	void			Update_GoinDisable(_float fTimeDelta);

private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			Create_ChildText();

private:
	// 매 프레임 돌릴만한 건 캐싱..
	CCustom_UI*			m_pRUI_All			= { nullptr };
	CCustom_UI*			m_pUI_Back			= { nullptr };
	CCustom_UI*			m_pUI_Hover			= { nullptr };
	CCustom_UI*			m_pUI_Arrow			= { nullptr };

	CCustom_UI*			m_pUI_InstHover		= { nullptr };

	CTransform*			m_pTransformCom_UIArrow			= { nullptr };


	class CGameSystem*	m_pGameSystem		= { nullptr };

private:
	_uint				m_iSelectedIndex	= ENUM_CLASS(UI_TAB_UTILITY::NOTHING);

	_bool				m_IsGoinDisabled	= false;
	_float				m_fDisableTimer		= 0.f;
	_uint				m_iAnimOrder		= 0;

	_bool				m_isFirstCheckedIndex	= false;

public:
	static CUI_TabUtility*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END