#pragma once
#include "Actor.h"
NS_BEGIN(Engine)
//class CModelAnim_Instance;
NS_END

NS_BEGIN(Client)
class CDummyNPC final : public CActor
{
public:
	typedef struct tagDummyNPCDesc : public CActor::ACTOR_DESC
	{
		_float3 vStartPos = {};
	}DUMMYNPC_DESC;

private:
	explicit CDummyNPC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CDummyNPC(const CDummyNPC& Prototype);
	virtual ~CDummyNPC() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;


public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) {};
	virtual void Effect_Active(const _wstring& wStrEffectTag) {};
	virtual void Object_Func(const _wstring& wStrObjectTag) {}; // 임시
	virtual void Hit_Judge(void* pArg = nullptr) {};// 임시
	// CActor을(를) 통해 상속됨

private:
//	CModelAnim_Instance* m_pModelInstanceCom = { nullptr };

public:
	CGameObject* Clone(void* pArg) override;
};
NS_END
