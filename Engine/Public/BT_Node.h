#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBT_Node abstract : public CBase
{
public:
	enum PATTERN_STATE
{
	FAILURE,
	RUNNING,
	SUCCESS
 };
protected:
	CBT_Node();
	CBT_Node(const CBT_Node& Prototype);
	virtual ~CBT_Node() = default;

public:
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize_Clone(void* pArg) = 0;

public:
	virtual PATTERN_STATE tick(class CGameObject* pGameObject) = 0;

public:
	virtual CBT_Node* Clone(void* pArg) = 0;
	virtual void Free() override;
};
NS_END
