#pragma once
#include "Component.h"
#include "BlackBoard.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBehavior_Tree final : public CComponent
{
public:
	typedef struct tagBehaviorTreeDesc
	{
		CBlackBoard* pBlackBoard;
	}BEHAVIOR_TREE_DESC;

private:
	CBehavior_Tree(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBehavior_Tree(const CBehavior_Tree& Prototype);
	virtual ~CBehavior_Tree() = default;

public:
	HRESULT Initialize_Prototype(class CBT_Node* pRoot);
	virtual HRESULT Initialize_Clone(void* pArg) override;
	
	void tick(class CGameObject* pGameObject);

#ifdef _DEBUG
	void BlackBoardInfo();
#endif // _DEBUG


private:
	CBT_Node* m_pRoot = { nullptr };
	CBlackBoard* m_pBlackBoard = {nullptr};

public:
	static CBehavior_Tree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBT_Node* pRoot);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
