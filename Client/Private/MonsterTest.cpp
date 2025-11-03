#include "ClientPch.h"
#include "MonsterTest.h"
#include "Ggobul.h"

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

	m_pTransformCom->Scale({ 1.f, 1.f, 1.f});
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
#pragma region ATTACK_STATE
	m_fAttackCoolTime[0] = 3.f;
	m_fAttackCoolTime[1] = 7.f;
	m_fAttackCoolTime[2] = 7.f;
	m_fAttackCoolTime[3] = 5.f;
	m_fAttackCoolTime[4] = 7.f;
	m_fAttackCoolTime[5] = 7.f;
	m_fAttackCoolTime[6] = 7.f;
	m_fAttackCoolTime[7] = 7.f;
	m_fAttackCoolTime[8] = 4.f;
	m_fAttackCoolTime[9] = 3.f;
#pragma endregion
	//Ready_PartObjects(pDesc);
	Ready_Component(pDesc);

	m_iHP = 1;
	m_fParalysisAcc = 5.f;
	return S_OK;
}

void CMonsterTest::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
	if(m_isTrigger == true)
		m_isDetecting = true;
	else
		m_isDetecting = false;
	m_isTrigger = false;
}

void CMonsterTest::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);
	// 1. 행동트리로 상태 갱신
	m_pBehaviorTreeCom->tick(this);

	//그로기 특수상황
	if(m_isParalysis)
	{
		if(m_isKnockDownTrig)
			m_iState = ENUM_CLASS(TEST_STATE::PARALYSIS);
		else
		{
			m_iState = (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::MOVE_FORWARD));
			m_isKnockDownTrig = true;
		}
	}
	else
		m_isKnockDownTrig = m_isParalysis;

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	//m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); // gpu
	m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); //cpu

	_vector vVelocity = m_pTransformCom->Get_Velocity();
	m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	// y축 수직 회전 lerp 사용할 함수 : CTransform->LookLerp
	// y축 수직으로  look fix할 함수 : CTransform->LookDir

}

void CMonsterTest::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);
	//for(auto& Pair : m_PartObjects)
	//	Pair.second->Late_Update(fTimeDelta);
#ifdef _DEBUG
	if(KEYSTATE::DOWN == m_pGameInstance->Get_DIKeyState(DIK_APOSTROPHE))
		m_isParalysis = true;
#endif // _DEBUG
	m_pRigidBodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CMonsterTest::Render()
{
	if(FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");


	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for(_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG

	//m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CMonsterTest::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if(iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{

		m_isTrigger = true;
		CTransform* pTransform = static_cast<CTransform*>(pOther);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));

	}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::ENEMY)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::NONE)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::DETECT)){}
	//else
	//	m_isTrigger = false;
}

void CMonsterTest::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
}

void CMonsterTest::Effect_Active(const _wstring& wStrEffectTag)
{
	//if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
	//	return;
	//
	//_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	//m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}

void CMonsterTest::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring strTypeTag = wStrObjectTag.substr(0, Index);
	_wstring strAnimTag = wStrObjectTag.substr(Index);
	if(strTypeTag == TEXT("GGOBUL"))
	{
		CGgobul::GGOBUL_RESET Desc{};
		Desc.eType = CGgobul::GGOBULTYPE::KNIFE;
		//Desc.pWorldMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		//Desc.vInitPosition = m_vTargetPosition;
		//XMStoreFloat3(&Desc.vInitDirection,m_pTransformCom->Get_State(STATE::LOOK));
		Desc.strPatternKey = WStringToString(strAnimTag);
		_matrix WorldMatrix;
		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
		_vector vRight = XMVector3Cross(vLook, XMVectorSet(0.f, 1.f, 0.f, 0.f));
		_vector vUp = XMVector3Cross(vLook, vRight);
		WorldMatrix.r[ENUM_CLASS(STATE::RIGHT)] = vRight;
		WorldMatrix.r[ENUM_CLASS(STATE::UP)] = vUp;
		WorldMatrix.r[ENUM_CLASS(STATE::LOOK)] = vLook;
		WorldMatrix.r[ENUM_CLASS(STATE::POSITION)] = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Ggobul"), WorldMatrix, &Desc);
	}
}

HRESULT CMonsterTest::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CMonsterTest::Ready_Component(MONSTERTEST_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(25.f, 13.f, 25.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.8f;
	ColliderDesc.fRadius = 0.4f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});

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
	AnimMachineDesc.pAnimationTag.assign(pDesc->pAnimationTag);
	//Com_AnimMachine
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_AnimMachine_FalseSovereign"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("MonsterTest/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("DodgeCooldown", [this]() ->_bool { return DodgeCooldown();});
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(0, 3.f); });
	pBlackBoard->Add_Condition("Attack10", [this]() ->_bool { return Attack(9, 4.f); });
	pBlackBoard->Add_Condition("Attack4", [this]() ->_bool { return Attack(3, 5.f); });
	pBlackBoard->Add_Condition("Attack7", [this]() ->_bool { return Attack(6, 6.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(2, 8.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(1, 10.f); });
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

void CMonsterTest::Reset_Condition(_float fTimeDelta)
{
	if(m_isAnimationFinished)
	{
		m_iState = ENUM_CLASS(TEST_STATE::NONE);
		
	}
	if(m_isDetecting)
	{
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
		_vector vDir = vTargetPos - vPosition;
		m_fDistance = XMVectorGetX(XMVector3Length(vDir));
		vDir = XMVector3Normalize(vDir);
		m_fFrontDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK))));
		m_fRightDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT))));

		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));
#ifdef _DEBUG
		//cout << "x : " << m_vTargetPosition.x << " y : " << m_vTargetPosition.y << " z : " << m_vTargetPosition.z << endl;
		//cout << "distance: " << m_fDistance << endl;
#endif
	}
	for(_uint i = 0; i < 10; ++i)
	{
		if(m_fAttackAcc[i] > 0.f)
			m_fAttackAcc[i] -= fTimeDelta;
	}
	if(m_fDodgeCoolTime > 0.f)
		m_fDodgeCoolTime -= fTimeDelta;

	if(m_isParalysis)
	{
		//if(m_isKnockDownTrig)
		//	m_iState |= ENUM_CLASS(TEST_STATE::PARALYSIS);
		//else
		//{
		//	m_iState |= (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::MOVE_FORWARD));
		//	m_isKnockDownTrig = true;
		//}
		m_fParalysisAcc -= fTimeDelta;
		if(m_fParalysisAcc <= 0.f)
		{
			//그로기 유지시간 정의하기
			m_fParalysisAcc = 5.f;
			m_isParalysis = false;
		}
	}
	else
		m_isKnockDownTrig = m_isParalysis;
}

void CMonsterTest::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
#ifdef _DEBUG
		cout << "On Hit! (False Sovereign)" << endl;
#endif // _DEBUG
	}
}

_bool CMonsterTest::isKnockDown()
{
	//현재 그로기 상태 여부 판단. 행동트리에서 상태 제어 X
	//if(false == m_isParalysis)
	//	return false;
	//else
	//{
	//	m_iState |= ENUM_CLASS(TEST_STATE::PARALYSIS);
	//}

	return m_isParalysis;
}

_bool CMonsterTest::isAttackEnable()
{
	if(!m_isDetecting)
		return false;
	_bool Result{};
	//for(_uint i = 0; i < 2; ++i)
	//{
	//	if(m_fAttackAcc[i] <= 0.f)
	//	{
	//		Result = true;
	//		break;
	//	}
	//}
	if(m_fAttackAcc[0] <= 0.f) Result = true;
	if(m_fAttackAcc[1] <= 0.f) Result = true;
	if(m_fAttackAcc[2] <= 0.f) Result = true;
	if(m_fAttackAcc[3] <= 0.f) Result = true;
	if(m_fAttackAcc[6] <= 0.f) Result = true;
	if(m_fAttackAcc[9] <= 0.f) Result = true;
	if(Result)
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	return Result;
}

_bool CMonsterTest::DodgeCooldown()
{
	_bool Result = m_isDetecting && m_fDodgeCoolTime <= 0.f;
	if(Result)
		m_fDodgeCoolTime = 7.f;
	return Result;
}

_bool CMonsterTest::Attack(_uint iIndex, _float fInterval)
{
	_bool Result = (m_fAttackAcc[iIndex] <= 0.f) && m_fDistance < fInterval;
	if(Result)
	{
		m_fAttackAcc[iIndex] = m_fAttackCoolTime[iIndex];
		//if (iIndex == 2)
		//{
		//	CGgobul::GGOBUL_RESET Desc{};
		//	Desc.eType = CGgobul::GGOBULTYPE::KNIFE;
		//	Desc.pWorldMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		//	Desc.strPatternKey = "SAttack03";
		//	//Desc.
		//	//m_pGameInstance->Spawn_PoolingObject()
		//}
	}
	return Result;
}

_bool CMonsterTest::Back()
{
	return m_fFrontDot < 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CMonsterTest::Front()
{
	return m_fFrontDot > 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CMonsterTest::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CMonsterTest::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
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
}
