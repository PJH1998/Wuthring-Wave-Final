#include "ClientPch.h"
#include "MonsterTest.h"
#include  "GameInstance.h"
#include "AnimMachine.h"

CMonsterTest::CMonsterTest(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMonsterTest::CMonsterTest(const CMonsterTest& Prototype)
	: CGameObject { Prototype }
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

	m_pTargetTransformCom = dynamic_cast<CTransform*>(m_pGameInstance->Get_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), 0, TEXT("Com_Transform")));
	if(nullptr == m_pTargetTransformCom)
		return E_FAIL;

	MONSTERTEST_DESC* pDesc = (MONSTERTEST_DESC*)pArg;

	Ready_Component(pDesc);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_iHP = 1;
	m_strCurrentAnimTag = "Born1";
	return S_OK;
}

void CMonsterTest::Priority_Update(_float fTimeDelta)
{
}

void CMonsterTest::Update(_float fTimeDelta)
{
	// 1. 행동트리로 상태 갱신
	m_pBehaviorTreeCom->tick(this);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	m_pAnimMachineCom->Update(fTimeDelta, m_pModelCom, &m_iState);

	// 
	//m_isAnimationFinished = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelta, nullptr);
	//m_pColliderCom->Update(vVelocity);
	
}

void CMonsterTest::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMonsterTest::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
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

void CMonsterTest::Ready_Component(MONSTERTEST_DESC* pDesc)
{
	// Com_Shader
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), 
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

	// Com_Model
	Add_Component(ENUM_CLASS(LEVEL::TEST), pDesc->szPrototypeModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr);

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
	pBlackBoard->Add_Data("iState", CBlackBoard::DATA_TYPE::INT, &m_iState);
	pBlackBoard->Add_Data("isAnimationFinished", CBlackBoard::DATA_TYPE::BOOL, &m_isAnimationFinished);
	pBlackBoard->Add_Checker("Attack1_Enable", [this]() ->_int {
		

		return 1;
		});

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_BehaviorTree_Test"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc);
#pragma endregion

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = "Born1";
	//Com_AnimMachine
	Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_AnimMachine"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc);
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

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
	//Safe_Release(m_pRigidbodyCom);
	//Safe_Release(m_pColliderCom);
}
