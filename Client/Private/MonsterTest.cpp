#include "ClientPch.h"
#include "MonsterTest.h"
#include  "GameInstance.h"
#include "MonsterBody.h"

CMonsterTest::CMonsterTest(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CMonsterTest::CMonsterTest(const CMonsterTest& Prototype)
	: CActor { Prototype }
{
}

HRESULT CMonsterTest::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonsterTest::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	/*m_pTargetTransformCom = dynamic_cast<CTransform*>(m_pGameInstance->Get_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), 0, TEXT("Com_Transform")));
	if(nullptr == m_pTargetTransformCom)
		return E_FAIL;*/

	MONSTERTEST_DESC* pDesc = (MONSTERTEST_DESC*)pArg;

	Ready_Component(pDesc);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_iHP = 1;
	return S_OK;
}

void CMonsterTest::Priority_Update(_float fTimeDelta)
{

	for(auto& Pair : m_PartObjects)
		Pair.second->Priority_Update(fTimeDelta);
}

void CMonsterTest::Update(_float fTimeDelta)
{
	// 1. 행동트리로 상태 갱신
	m_pBehaviorTreeCom->tick(this);

	for(auto& Pair : m_PartObjects)
		Pair.second->Update(fTimeDelta);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	//m_pAnimMachineCom->Update(fTimeDelta, m_pModelCom, &m_iState);

	// 
	//m_isAnimationFinished = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelta, nullptr);
	//m_pColliderCom->Update(vVelocity);
	
}

void CMonsterTest::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);
	for(auto& Pair : m_PartObjects)
		Pair.second->Late_Update(fTimeDelta);
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMonsterTest::Render()
{
	

#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
#endif
}

void CMonsterTest::Ready_Component(MONSTERTEST_DESC* pDesc)
{

	// Com_Rigidbody
	//CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::MESH;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.pModel = m_pModelCom;
	//CRigidbody::CAPSULEBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::CAPSULE;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Kinematic;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	//RigidbodyDesc.fHeight = 10.f;
	//RigidbodyDesc.fRadius = m_pGameInstance->Rand(5.f, 20.f);
	//RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::VIRTUAL;
	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	// Com_Collider
	//CCollider::COLLIDER_DESC ColliderDesc = {};
	////XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//ColliderDesc.vPos = _float3(0.f, 0.f, 0.f);
	//ColliderDesc.eType = EMotionType::Kinematic;
	//ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	//ColliderDesc.fHeight = 10.f;
	//ColliderDesc.fRadius = 20.f; //m_pGameInstance->Rand(5.f, 20.f);
	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", CBlackBoard::DATA_TYPE::MASK, &m_iState);
	//pBlackBoard->Add_Data("isAnimationFinished", CBlackBoard::DATA_TYPE::BOOL, &m_isAnimationFinished);
	/*pBlackBoard->Add_Condition("Attack1_Enable", [this]() ->_bool {
		

		return 1;
		});*/

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_BehaviorTree_Test"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion

}

void CMonsterTest::Ready_PartObjects(MONSTERTEST_DESC* pDesc)
{
	CMonsterBody::MONSTERBODY_DESC BodyDesc{};
	BodyDesc.pParentTransform = m_pTransformCom;
	BodyDesc.pState = &m_iState;
	BodyDesc.pAnimationTag = pDesc->pAnimationTag;
	if(FAILED(CContainerObject::Add_PartObject(TEXT("Part_Body"),m_pGameInstance->Get_CurrentLevel(), 
												TEXT("Prototype_GameObject_MonsterBody"), &BodyDesc)))
		CRASH("Part_Body")


}

CMonsterTest* CMonsterTest::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonsterTest* pInstance = new CMonsterTest(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Dummy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMonsterTest::Clone(void* pArg)
{
	CMonsterTest* pClone = new CMonsterTest(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Dummy (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMonsterTest::Free()
{
	__super::Free();

	Safe_Release(m_pBehaviorTreeCom);
	//Safe_Release(m_pRigidbodyCom);
	//Safe_Release(m_pColliderCom);
}
