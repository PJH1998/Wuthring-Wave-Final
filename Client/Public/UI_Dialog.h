#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Dialog final : public CCustom_UI
{
public:
	typedef struct tUIDialogDesc {
		_string strFilePath = {};
		_bool	isInteractable = true;
	} UI_DIALOG_DESC;

	typedef struct tLocalDialogDesc {
		_wstring strSpeaker = {};
		_wstring strDialog = {};
	} DIALOG_DESC;

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

public:
	void			Req_Close_Dialog()		{ m_isGoinDisable = true; }
	void			Req_Next_Dialog();
	void			Req_Finish_CurDialog();

	void			Req_InteractExternally()	{ m_isInteracted_Externally = true; }

	_bool			Get_isFinished_CurDialog()	{ return m_isCurDialogFinished; }
	_bool			Get_isLast_CurDialog()		{ return m_iDialogOrder == m_vecDialogs.size() - 1; }


private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

	HRESULT			Create_ChildText_Speaker();
	HRESULT			Create_ChildText_Dialog();

private:
	void			Update_DialogInstance(_float fTimeDelta);
	void			Update_DialogOrder(_float fTimeDelta);
	void			Update_GoinDisable(_float fTimeDelta);

	void			Load_Dialog(const _char* pFilePath);
	void			Change_Dialog(_uint iDialogIndex);
	

private:
	// 매 프레임 돌릴만한 건 캐싱..
	CCustom_UI* m_pRUI_All				= { nullptr };
	CCustom_UI* m_pUI_SectorA_Images	= { nullptr };
 
	CCustom_UI* m_pTextUI_Speaker		= { nullptr };
	CCustom_UI* m_pTextUI_Dialog		= { nullptr };

private:
	_bool				m_isGoinDisable = false;
	_float				m_fDisableTimer = 0.f;
	const _float		m_fDisableTime = (15.f) * 1.f / 60.f;
	_uint				m_iAnimOrder = 0;

	_float				m_fTickElapsedTime = 0.f;
	const _float		m_fInstIntervalTime = 0.1f;		// 글자간 출력 시간간격 (낮은 값일수록 글자들이 빠르게 이어 나옴)
	const _float		m_fInstFadeInTime = 0.5f;		// 각 글자 당 알파 변화 시간 (낮을수록 각 글자 하나하나가 빠르게 나타남)
	
	vector<DIALOG_DESC>	m_vecDialogs = {};
	_uint				m_iDialogOrder = 0;

	_bool				m_isCurDialogFinished = false;


	_bool				m_isInteracted = false;
	_bool				m_isInteracted_Externally = false;

	_bool				m_isInteractable = true;

private:
	class CGameSystem* m_pGameSystem = { nullptr };

public:
	static CUI_Dialog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END