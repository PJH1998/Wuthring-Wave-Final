#pragma once
#include "BT_Node.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBT_Action final : public CBT_Node
{
	CBT_Action(function<PATTERN_STATE(class CGameObject* pGameObject)> Action) :m_Action{ Action } {}
	CBT_Action(const CBT_Action& Prototype);
	virtual ~CBT_Action() = default;

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize_Clone(void* pArg) override;

	PATTERN_STATE tick(class CGameObject* pGameObject) override{ return m_Action(pGameObject); }

private:
	function<PATTERN_STATE(CGameObject* pGameObject)> m_Action;

public:
	static CBT_Action* Create(function<PATTERN_STATE(CGameObject* pGameObject)> Action);
	CBT_Node* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
