#include "ClientPch.h"
#include "Corosaurus.h"
#include "AttackVolume.h"

CCorosaurus::CCorosaurus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CActor { pDevice, pContext }
{
}

CCorosaurus::CCorosaurus(const CCorosaurus& Prototype)
	: CActor { Prototype }
{
}

HRESULT CCorosaurus::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCorosaurus::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	CORROSAURUS_DESC* pDesc = static_cast<CORROSAURUS_DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	m_vDistanceRange = _float2(4.f, 5.f);
	Ready_Component(pDesc);
	//Ready_PartObjects(pDesc);
	//CActor::Register_AllNotifies(pDesc->strFolderPath);

	return S_OK;
}

void CCorosaurus::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CCorosaurus::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);
	m_pBehaviorTreeCom->tick(this);

	After_Condition(fTimeDelta);
	m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta);
	_vector vVelocity = m_pTransformCom->Get_Velocity();

	if(m_isDist_Interp_Enable)
	{
		m_pColliderCom->Update(vVelocity / fTimeDelta * m_fDistance);
		m_isDist_Interp_Enable = false;
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);

	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Update(fTimeDelta);
	}
}

void CCorosaurus::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);
}

void CCorosaurus::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Render();
	}
#endif // DEBUG

}

void CCorosaurus::Collider_Active(const _wstring& wStrColliderTag, _bool isActive)
{

}

void CCorosaurus::Effect_Active(const _wstring& wStrEffectTag)
{
	/*if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);*/
}

HRESULT CCorosaurus::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CCorosaurus::Ready_Component(CORROSAURUS_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vDetectRange;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});

	// Com_Collider (Body)
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.8f;
	ColliderDesc.fRadius = 0.4f;
	XMStoreFloat4(&ColliderDesc.vQuat, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(90.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f)));
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});
	m_tCallDesc.pTransform = m_pTransformCom;
	m_tCallDesc.fAttack = 10.f;
	m_pColliderCom->Set_Desc(&m_tCallDesc);


	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Corrosaurus/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("Corrosaurus/Com_ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Corrosaurus/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_AnimMachine_Corrosaurus"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("Corrosaurus/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("ATKArrange", [this]() ->_bool { return AttackArrange(); });
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK1, 5.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK2, 5.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK3, 15.f); });
	pBlackBoard->Add_Condition("Attack4", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK4, 15.f); });
	pBlackBoard->Add_Condition("Attack7", [this]() ->_bool { return Attack(ATK_PATTERN::BURST, 50.f); });
	pBlackBoard->Add_Condition("Attack8", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK8, 15.f); });
	pBlackBoard->Add_Condition("isChase", [this]() ->_bool { return isChase(); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });
	
	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_BehaviorTree_Corrosaurus"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion
}

void CCorosaurus::Ready_PartObjects(CORROSAURUS_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Prop003_M");
	TriggerDesc.vExtent = _float3(0.5f, 0.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	m_pAtkVolumes[ATK_SOCKET::HEAD] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::HEAD])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::HEAD]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Tail006_M");
	TriggerDesc.vExtent = _float3(0.5f, 0.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::TAIL] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::TAIL])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::TAIL]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::PARRY;
	TriggerDesc.eTargetLayers = { COLLISIONLAYER::ATTACK, COLLISIONLAYER::SKILL, COLLISIONLAYER::KNOCKBACK };
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(2); // Root
	TriggerDesc.vExtent = _float3(2.f, 4.f, 4.f);
	TriggerDesc.vOffsetPos = _float3(0.0f, 0.f, -4.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pParryVolumes = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pParryVolumes)
		CRASH(m_pParryVolumes);
	m_pParryVolumes->TriggerActivate(false);
}

void CCorosaurus::Reset_Condition(_float fTimeDelta)
{
	if (m_isAnimationFinished)
	{

		if (m_iState & ENUM_CLASS(TEST_STATE::MOVE_LEFT))
		{
			m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -1.f * XM_PIDIV2);
		}
		else if (m_iState & ENUM_CLASS(TEST_STATE::MOVE_RIGHT))
		{
			m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XM_PIDIV2);
		}
		m_iState = ENUM_CLASS(TEST_STATE::NONE);
	}
	if (m_isDetecting)
	{
		Calculate_PosAndDir();
	}
}

void CCorosaurus::After_Condition(_float fTimeDelta)
{
	if (m_isTurnLerp)
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta);
}

void CCorosaurus::Calculate_PosAndDir()
{
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
	_vector vDir = vTargetPos - vPosition;
	m_fDistance = XMVectorGetX(XMVector3Length(vDir));
	vDir = XMVector3Normalize(vDir);
	m_fFrontDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK))));
	m_fRightDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT))));

	XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));
}

void CCorosaurus::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		if (!m_isAggro)
			m_isAggro = true;
	}
}

void CCorosaurus::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CCorosaurus::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		m_biHit = true;
#ifdef _DEBUG
		cout << "Be Hit! (Corro)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG

	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
	{
		m_biHit = true;
#ifdef _DEBUG
		cout << "Be Hit! SKILL (Corro)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		m_biHit = true;
		memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
#ifdef _DEBUG
		cout << "Knock Back! (Corro)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
	}
}

void CCorosaurus::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	m_isBlocked = true;
}

_bool CCorosaurus::isKnockDown()
{
	_bool isKnockDown{};
	if (m_fStamina <= 0.f)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::PARALYSIS);
		m_fStamina = m_fMaxStamina;
		isKnockDown = true;
	}
	else if (m_isBlocked)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BLOCK);
		isKnockDown = true;
		m_isBlocked = false;
	}

	if(m_biHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		isKnockDown = true;
		m_biHit = false;
	}

	return isKnockDown;
}

_bool CCorosaurus::isAttackEnable()
{
	if (!m_isDetecting)
		return false;

	return _bool();
}

_bool CCorosaurus::AttackArrange()
{
	if (m_iState == 0)
	{

	}
	else
	{
		_float fRand = m_pGameInstance->Rand_Normal();
		if (fRand < 0.5f)
		{
			m_iState |= ENUM_CLASS(TEST_STATE::MOVE_FORWARD);
		}
	}
	return true;
}

_bool CCorosaurus::Attack(_uint iIndex, _float fInterval)
{
	return _bool();
}

_bool CCorosaurus::isChase()
{
	return _bool();
}

_bool CCorosaurus::isPatrol()
{
	return _bool();
}

_bool CCorosaurus::Back()
{
	return m_fFrontDot < 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CCorosaurus::Front()
{
	return m_fFrontDot > 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CCorosaurus::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CCorosaurus::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
}

CCorosaurus* CCorosaurus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCorosaurus* pInstance = new CCorosaurus(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCorosaurus");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCorosaurus::Clone(void* pArg)
{
	CCorosaurus* pClone = new CCorosaurus(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CCorosaurus (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CCorosaurus::Free()
{
	__super::Free();

	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		Safe_Release(m_pAtkVolumes[i]);
	}
	
	Safe_Release(m_pParryVolumes);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
