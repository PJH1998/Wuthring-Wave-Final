#include "EnginePch.h"
#include "Behavior_Tree.h"
#include "BT_Node.h"

CBehavior_Tree::CBehavior_Tree(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent { pDevice , pContext }
{
}

CBehavior_Tree::CBehavior_Tree(const CBehavior_Tree& Prototype)
	:CComponent { Prototype }
	,m_pRoot { Prototype.m_pRoot }
{
	Safe_AddRef(m_pRoot);
}

HRESULT CBehavior_Tree::Initialize_Prototype(CBT_Node* pRoot)
{
    m_pRoot = pRoot;
    //Load_Tree_Graph(file);
	return S_OK;
}

HRESULT CBehavior_Tree::Initialize_Clone(void* pArg)
{
	BEHAVIOR_TREE_DESC* pDesc = static_cast<BEHAVIOR_TREE_DESC*>(pArg);
	if(pDesc == nullptr)
		return E_FAIL;

	m_pBlackBoard = pDesc->pBlackBoard;
	if(m_pBlackBoard == nullptr)
		return E_FAIL;
    
	return S_OK;
}

void CBehavior_Tree::tick(CGameObject* pGameObject)
{
    m_pRoot->tick(pGameObject, m_pBlackBoard);
}

#ifdef _DEBUG
void CBehavior_Tree::BlackBoardInfo()
{
    m_pBlackBoard->Bind_Data_to_GUI();
}
#endif // _DEBUG

void CBehavior_Tree::Load_Tree_Graph(ifstream& File)
{
	struct Link
	{
		size_t iInNodeIndex;
		size_t iInSlotIndex;
		size_t iOutNodeIndex;
		size_t iOutSlotIndex;
	};

	struct NodeData
	{
		BT_TYPE eType;
		vector<Link> Transition;
	};

	json BT_Data;
	File >> BT_Data;

	size_t iNumNodes = BT_Data["NumNode"];
	/*for(auto& NodeData : BT_Data["Nodes"])
	{
		BT_TYPE eType = NodeData["eType"];
		size_t iNumTransition = NodeData["NumTransition"];
		vector<GraphEditor::Link> Transition;
		for(auto& Transit : NodeData["Transitions"])
		{
			GraphEditor::Link tLink{Transit["InputNodeIndex"],
									Transit["InputSlotIndex"],
									Transit["OutputNodeIndex"],
									Transit["OutputSlotIndex"]};
			Transition.push_back(tLink);
			m_Links.push_back(tLink);
		}

		size_t iNumCondition = NodeData["NumCondition"];
		vector<CONDITION_TAG> Conditions;
		for(auto& Cond : NodeData["Conditions"])
		{
			CONDITION_TAG Condition{Cond["ValueName"], Cond["ConditionName"], Cond["ConstName"]};
			Conditions.push_back(Condition);
		}
		_float x{NodeData["Editor_PosX"]}, y{NodeData["Editor_PosY"]};
		_string szNodeName;
		szNodeName = "Count";
		szNodeName += to_string(m_iNodeCount).c_str();
		switch(eType)
		{
		case Editor::CASM_Interface::ACTION:
		{
			MyNode tNode = {"Action", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType};
			tNode.Transitions = Transition;
			tNode.Conditions = Conditions;
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SELECTOR:
		{
			MyNode tNode = {"Selector", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType};
			tNode.Transitions = Transition;
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SEQUENCE:
		{
			MyNode tNode = {"Sequence", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType};
			tNode.Transitions = Transition;
			m_Nodes.push_back(tNode);
			break;
		}
		default:
			break;
		}
	}*/
}

CBehavior_Tree* CBehavior_Tree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBT_Node* pRoot)
{
    CBehavior_Tree* pInstance = new CBehavior_Tree(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype(pRoot)))
    {
        MSG_BOX("Failed to Created : CBehavior_Tree");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CBehavior_Tree::Clone(void* pArg)
{
    CBehavior_Tree* pInstance = new CBehavior_Tree(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBehavior_Tree");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBehavior_Tree::Free()
{
	__super::Free();
	Safe_Release(m_pRoot);
    Safe_Release(m_pBlackBoard);
}
