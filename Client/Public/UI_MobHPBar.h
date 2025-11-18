#pragma once
#include "UI_Image.h"

NS_BEGIN(Client)
class CUI_MobHPBar final : public CUI_Image
{
public:
	typedef struct tUIMobHPBarDesc {	// 나중에 Transform 은 아닌, 뼈 위치 등 기준으로 변경. matrix, float3 등
		// hp와 같은 정보들을 벡터로 받아와야하나? 어떤 정보를 어떻게 받아오지

	} UI_MOBHP_DESC;

public:
	explicit CUI_MobHPBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_MobHPBar(const CUI_MobHPBar& Prototype);
	virtual ~CUI_MobHPBar() = default;

public:
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

private:
	void			Ready_Presets();
	void			Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos);
	void			Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance);

private:


public:
	static CUI_MobHPBar*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

NS_END