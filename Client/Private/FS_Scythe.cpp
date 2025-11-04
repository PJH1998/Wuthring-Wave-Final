#include "ClientPch.h"
#include "FS_Scythe.h"
#include "AttackVolume.h"

CFS_Scythe::CFS_Scythe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CFS_Scythe::CFS_Scythe(const CFS_Scythe& Prototype)
	:CActor { Prototype }
{
}

HRESULT CFS_Scythe::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFS_Scythe::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	SCYTHE_DESC* pDesc = static_cast<SCYTHE_DESC*>(pArg);
	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	//Register_AllNotifies(pDesc->strFolderPath);

	//for (size_t i = 0; i < 5; ++i)
	//{
	//	m_pAttackVolume[i]->IsActivate(false);
	//}
	//m_pAttackTransform = m_pModelCom->Get_BoneMatrixPtr("HitCase");
	m_fAttackDamage = pDesc->fAttackDamage;
	//풀링 오브젝트 자체적으로 activate 끄기
	m_isActivate = false;
    return S_OK;
}

void CFS_Scythe::Priority_Update(_float fTimeDelta)
{
}

void CFS_Scythe::Update(_float fTimeDelta)
{
	_bool isAnimFinished{};
	m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, isAnimFinished, fTimeDelta);
	if(!m_iState && isAnimFinished)
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		for (_uint i = 0; i < 5; ++i)
		{
			if (nullptr != m_pAttackVolumes[i])
				m_pAttackVolumes[i]->TriggerActivate(false);
		}
		return;
	}
	else if (m_fLifeTime <= 0.f)
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		for (_uint i = 0; i < 5; ++i)
		{
			if (nullptr != m_pAttackVolumes[i])
				m_pAttackVolumes[i]->TriggerActivate(false);
		}
		return;
	}
	m_fLifeTime -= fTimeDelta;

#pragma region ATTACK_VOLUME
	for (_uint i = 0; i < 5; ++i)
	{
		if (nullptr != m_pAttackVolumes[i])
			m_pAttackVolumes[i]->Update(fTimeDelta);
	}
#pragma endregion
}

void CFS_Scythe::Late_Update(_float fTimeDelta)
{
	//뼈 공격 볼륨 동기화 설정, 뼈에다가 맞추려면 sync 사용 X
	//m_pRigidBodyCom[m_eType]->Sync_Rigidbody(m_pTransformCom);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CFS_Scythe::Render()
{
	Bind_Resources();

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}
#ifdef _DEBUG
	//m_pAttackVolume[m_eType]->Render();
	for (_uint i = 0; i < 5; ++i)
	{
		if (nullptr != m_pAttackVolumes[i])
			m_pAttackVolumes[i]->Render();
	}
#endif
}

void CFS_Scythe::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	SCYTHE_RESET* pDesc = static_cast<SCYTHE_RESET*>(pArg);
	m_strAnimKey = pDesc->strPatternKey;
	m_pAnimMachineCom->Reset(m_pModelCom, m_strAnimKey);
	for (_uint i = 0; i < 5; ++i)
	{
		//if (nullptr != m_pAttackVolumes[i])
		//	m_pAttackVolumes[i]->TriggerActivate(true);
		m_isVolumeActive[i] = true;
	}
	if (m_strAnimKey == "SAttack04_1_Start")
		m_iState = 1;
	else
	{
		m_iState = 0;
		m_isVolumeActive[4] = false;
	}
	m_fLifeTime = 5.f;
	m_isActivate = true;
}

void CFS_Scythe::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CFS_Scythe::Ready_Component(SCYTHE_DESC* pDesc)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = "Stand1";
	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_AnimMachine_Scythe"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("Ggobul/Com_AnimMachine");

}

void CFS_Scythe::Ready_PartObjects(SCYTHE_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Nub001");
	TriggerDesc.vExtent = _float3(0.3f, 1.2f, 0.7f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = m_fAttackDamage;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHit_Enter(iLayer, pOther, Manifold);
		};

	m_pAttackVolumes[0] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[0])
		CRASH(m_pAttackVolumes[0]);
	m_pAttackVolumes[0]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Nub002");
	m_pAttackVolumes[1] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[1])
		CRASH(m_pAttackVolumes[1]);
	m_pAttackVolumes[1]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Nub003");
	m_pAttackVolumes[2] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[2])
		CRASH(m_pAttackVolumes[2]);
	m_pAttackVolumes[2]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Nub004");
	m_pAttackVolumes[3] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[3])
		CRASH(m_pAttackVolumes[3]);
	m_pAttackVolumes[3]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Nub005");
	m_pAttackVolumes[4] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[4])
		CRASH(m_pAttackVolumes[4]);
	m_pAttackVolumes[4]->TriggerActivate(false);
}

void CFS_Scythe::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	if(wStrColliderTag == TEXT("Attack Trig"))
	{
		for (_uint i = 0; i < 5; ++i)
			m_pAttackVolumes[i]->TriggerActivate(m_isVolumeActive[i]);
	}
}

void CFS_Scythe::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CFS_Scythe::Object_Func(const _wstring& wStrObjectTag)
{
}

void CFS_Scythe::OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

CFS_Scythe* CFS_Scythe::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFS_Scythe* pInstance = new CFS_Scythe(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CFS_Scythe");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFS_Scythe::Clone(void* pArg)
{
	CFS_Scythe* pClone = new CFS_Scythe(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CFS_Scythe (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CFS_Scythe::Free()
{
	__super::Free();

	Safe_Release(m_pAnimMachineCom);
	for (size_t i = 0; i < 5; ++i)
	{
		Safe_Release(m_pAttackVolumes[i]);
	}

}
