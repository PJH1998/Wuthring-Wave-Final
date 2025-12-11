#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Dialog final : public CCustom_UI
{
public:
	typedef struct tTabUtilityUIIDesc {
		_uint iCharSelectedUtilityIndex = ENUM_CLASS(UI_TAB_UTILITY::NOTHING);
	} UI_TABUTIL_DESC;

public:
	explicit CUI_Dialog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Dialog(const CUI_Dialog& Prototype);
	virtual ~CUI_Dialog() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

	void			Create_ChildText();

private:
	// 매 프레임 돌릴만한 건 캐싱..
	CCustom_UI* m_pRUI_All = { nullptr };


	CCustom_UI* m_pTextUI_Speaker = { nullptr };
	CCustom_UI* m_pTextUI_Dialog = { nullptr };



public:
	static CUI_Dialog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END