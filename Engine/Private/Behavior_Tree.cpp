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
