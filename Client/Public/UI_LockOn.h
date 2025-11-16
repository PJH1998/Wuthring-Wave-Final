#pragma once
#include "UI_Image.h"


NS_BEGIN(Engine)
class CTransform;
NS_END


NS_BEGIN(Client)

class CUI_LockOn final : public CUI_Image
{
public:
	typedef struct tUILockOnDesc {
		CTransform* pTargetTransform = nullptr;
	} UI_LOCKON_DESC;

public:
	explicit CUI_LockOn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_LockOn(const CUI_LockOn& Prototype);
	virtual ~CUI_LockOn() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

public:
	HRESULT			Bind_TargetTransform(CTransform* pTargetTransform) {m_pTargetTransform = pTargetTransform;}

private:
	void			Ready_Presets();

private:
	//class CGameSystem*		m_pGameSystem = { nullptr };
	CTransform*		m_pTargetTransform = { nullptr };

public:
	static CUI_LockOn*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END