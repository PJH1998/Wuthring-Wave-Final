#pragma once

#include "ContainerObject.h"
NS_BEGIN(Editor)
class CAnimationActor final : public CContainerObject
{
public:
	typedef struct tagAnimationActorDesc : CContainerObject::GAMEOBJECT_DESC
	{

	}ANIMATION_ACTOR_DESC;

private:
	explicit CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimationActor(const CAnimationActor& Prototype);
	virtual ~CAnimationActor() = default;

public:
	virtual	HRESULT Initialize_Prototype() override;
	virtual	HRESULT Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

public:
	virtual	CGameObject* Clone(void* pArg) override;
	static CAnimationActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

