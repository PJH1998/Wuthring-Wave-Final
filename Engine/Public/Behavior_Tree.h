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
	enum BT_TYPE
	{
		ACTION, SELECTOR, SEQUENCE
	};

private:
	struct Link
	{
		size_t iInNodeIndex;
		size_t iInSlotIndex;
		size_t iOutNodeIndex;
		size_t iOutSlotIndex;
	};
	typedef struct Condition_tag
	{
		_string ValueName;
		_string ConditionName;
		_string ConstName;
	}CONDITION_TAG;

	struct NodeDat
	{
		BT_TYPE eType;
		_uint iTargetState;
		vector<Link> Transition;
		CONDITION_TAG Conditions;
	};

private:
	CBehavior_Tree(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBehavior_Tree(const CBehavior_Tree& Prototype);
	virtual ~CBehavior_Tree() = default;

public:
	HRESULT Initialize_Prototype(class CBT_Node* pRoot);
	//HRESULT Initialize_Prototype(const _char* BehaviorTreeDataPath);
	virtual HRESULT Initialize_Clone(void* pArg) override;
	
	void tick(class CGameObject* pGameObject);

#ifdef _DEBUG
	void BlackBoardInfo();
#endif // _DEBUG


private:
	CBT_Node* m_pRoot = { nullptr };
	CBlackBoard* m_pBlackBoard = {nullptr};

	vector<NodeDat> m_NodesDatas;

private:
	void Load_Tree_Graph(const _char* BehaviorTreeDataPath);
	CBT_Node* Create_Node(_uint iIndex);

public:
	static CBehavior_Tree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBT_Node* pRoot);
	//static CBehavior_Tree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* BehaviorTreeDataPath);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
