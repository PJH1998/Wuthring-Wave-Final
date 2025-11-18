#pragma once
#include "UI_Image.h"

NS_BEGIN(Client)
class CUI_Parry final : public CUI_Image
{
public:
	typedef struct tUILockOnDesc {	// 나중에 Transform 은 아닌, 뼈 위치 등 기준으로 변경. matrix, float3 등
		CTransform* pTargetTransform = nullptr;
	} UI_Parry_DESC;

public:
	explicit CUI_Parry(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Parry(const CUI_Parry& Prototype);
	virtual ~CUI_Parry() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;	// 보스 / 엘리트만 쓴다고 가정해야할 듯. 

public:
	void			Enable_Parried();
	
private:
	void			Ready_Presets();
	void			Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos);
	void			Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance);
	 
private:
	//class CGameSystem*		m_pGameSystem = { nullptr };
	//CTransform* m_pTargetTransform = { nullptr };

	_float			m_fElapsedTime = 0.f;
	_bool			m_isParried = false;

	_float3			m_vTargetPos = _float3(0.f, -10.f, 0.f);

	//_float3			m_vOriginSca = {};

public:
	static CUI_Parry*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END