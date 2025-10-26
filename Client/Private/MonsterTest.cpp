#include "ClientPch.h"
#include "MonsterTest.h"
#include  "GameInstance.h"

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
	
	MONSTERTEST_DESC* pDesc = static_cast<MONSTERTEST_DESC*>(pArg);
	//Ready_PartObjects(pDesc);
	Ready_Component(pDesc);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_iHP = 1;
	return S_OK;
}

void CMonsterTest::Priority_Update(_float fTimeDelta)
{

	//for(auto& Pair : m_PartObjects)
	//	Pair.second->Priority_Update(fTimeDelta);
}

void CMonsterTest::Update(_float fTimeDelta)
{
	// 1. 행동트리로 상태 갱신
	m_pBehaviorTreeCom->tick(this);

	//for(auto& Pair : m_PartObjects)
	//	Pair.second->Update(fTimeDelta);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, &m_iState, m_isAnimationFinished, fTimeDelta);

	// 
	//m_isAnimationFinished = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelta, nullptr);
	//m_pColliderCom->Update(vVelocity);
	
}

void CMonsterTest::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);
	//for(auto& Pair : m_PartObjects)
	//	Pair.second->Late_Update(fTimeDelta);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMonsterTest::Render()
{
	if(FAILED(Bind_ShaderResources()))
		CRASH("Falied to Bind Resources");


	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for(_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
#endif
}

void CMonsterTest::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{

}

HRESULT CMonsterTest::Bind_ShaderResources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
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

	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	RigidbodyDesc.vExtent = _float3(1000.f, 400.f, 1000.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});

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

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 0.f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 10.f;
	ColliderDesc.fRadius = 20.f; //m_pGameInstance->Rand(5.f, 20.f);
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	m_pColliderCom->Set_Desc(m_pTransformCom);


	// Com_Shader
	if(FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("MonsterTest/Com_Shader");
	
	// Com_ComputeShader
	if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("MonsterTest/Com_ComputeShader");

	// Com_Model
	if(FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("MonsterTest/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_AnimMachine_Test"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("MonsterTest/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	//pBlackBoard->Add_Data("isAnimationFinished", CBlackBoard::DATA_TYPE::BOOL, &m_isAnimationFinished);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("DodgeCooldown", [this]() ->_bool { return DodgeCooldown();});
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attadk1(); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack2(); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_BehaviorTree_Test"),
		TEXT("MonsterTest/Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion

}

void CMonsterTest::Ready_PartObjects(MONSTERTEST_DESC* pDesc)
{
	//CMonsterBody::MONSTERBODY_DESC BodyDesc{};
	//BodyDesc.pParentTransform = m_pTransformCom;
	//BodyDesc.pState = &m_iState;
	//BodyDesc.szPrototypeModelTag = pDesc->szPrototypeModelTag;
	//BodyDesc.pAnimationTag = pDesc->pAnimationTag;
	//if(FAILED(CContainerObject::Add_PartObject(TEXT("Part_Body"),m_pGameInstance->Get_CurrentLevel(), 
	//											TEXT("Prototype_GameObject_MonsterBody"), &BodyDesc)))
	//	CRASH("Part_Body")


}

_bool CMonsterTest::isAttackEnable()
{
	return false;
}

_bool CMonsterTest::DodgeCooldown()
{
	return false;
}

_bool CMonsterTest::Attadk1()
{
	return false;
}

_bool CMonsterTest::Attack2()
{
	return false;
}

_bool CMonsterTest::Back()
{
	return false;
}

_bool CMonsterTest::Front()
{
	return false;
}

_bool CMonsterTest::Left()
{
	return false;
}

_bool CMonsterTest::Right()
{
	return false;
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
	Safe_Release(m_pAnimMachineCom);
	//Safe_Release(m_pRigidbodyCom);
	//Safe_Release(m_pColliderCom);
}
