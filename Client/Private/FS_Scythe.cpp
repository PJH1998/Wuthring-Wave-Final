#include "ClientPch.h"
#include "FS_Scythe.h"
#include "AttackVolume.h"
#include "GameSystem.h"

CFS_Scythe::CFS_Scythe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CFS_Scythe::CFS_Scythe(const CFS_Scythe& Prototype)
	:CActor { Prototype }
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
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
	Register_AllNotifies(pDesc->strFolderPath);
	m_iSoundChannel = -1;
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
	_float fTimeRatio = m_pGameSystem->TimeLack(COLLISIONLAYER::ENEMY);
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, isAnimFinished, fTimeDelta * fTimeRatio);
	if(!m_iState && isAnimFinished)
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		m_pGameInstance->Return_Channel(m_iSoundChannel);
		m_iSoundChannel = -1;
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
		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));

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
	//_float temp{};
	//m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, m_strAnimKey, 0.f, &temp, false);
	for (_uint i = 0; i < 5; ++i)
	{
		//if (nullptr != m_pAttackVolumes[i])
		//	m_pAttackVolumes[i]->TriggerActivate(true);
		m_isVolumeActive[i] = true;
	}
	if (m_strAnimKey == "SAttack04_1_Start")
	{
		m_iState = 1;
		for (_uint i = 0; i < 5; ++i)
			m_pAttackVolumes[i]->Change_Layer(COLLISIONLAYER::ENEMY_ATTACK);
	}
	else
	{
		for (_uint i = 0; i < 5; ++i)
			m_pAttackVolumes[i]->Change_Layer(COLLISIONLAYER::ENEMY_HARDATTACK);
		m_iState = 0;
		m_isVolumeActive[4] = false;
	}
	m_fLifeTime = 5.f;
	m_isActivate = true;
	m_iSoundChannel = m_pGameInstance->Register_Channel();
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
	TriggerDesc.test = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer) {
		this->OnHit_Enter(iLayer, pOther, Manifold, eVolumeLayer);
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
	if(wStrColliderTag == TEXT("Attack"))
	{
		for (_uint i = 0; i < 5; ++i)
		{
			if(nullptr != m_pAttackVolumes[i] && m_isVolumeActive[i])
				m_pAttackVolumes[i]->TriggerActivate(Isactive);
		}
	}
}

void CFS_Scythe::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CFS_Scythe::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrPartTag = wStrObjectTag.substr(Index + 1);
	if(wstrTypeTag == TEXT("Sound"))
		Sound_Active(wstrPartTag);
}

void CFS_Scythe::Sound_Active(const _wstring& wStrObjectTag)
{
	if (m_iSoundChannel == -1)
		return;
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrPartTag = wStrObjectTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Appear"))
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("SFX_Enemy_Weizuoshenwang_Longche_Battle_Apper_1 (SFX)"), m_iSoundChannel, 0.1f, m_pTransformCom, 0.01f, 2.f);
	}
	else if (wstrTypeTag == TEXT("Loop"))
	{
		if (false == m_isPlay)
		{
			m_isPlay = true;
			m_pGameInstance->Play_Sound_Dynamic(TEXT("SFX_Enemy_Weizuoshenwang_Longche_Battle_Loop (SFX)"), m_iSoundChannel, 0.1f, m_pTransformCom, 0.01f, 2.f);
		}
	}
	else if (wstrTypeTag == TEXT("Seperate"))
	{
		if (wstrPartTag == TEXT("1"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("SFX_Enemy_Weizuoshenwang_Longche_Battle_SickleSweepsAcross1 (SFX)"), m_iSoundChannel, 0.2f, m_pTransformCom, 0.01f, 2.f);
		}
		else if (wstrPartTag == TEXT("2"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("SFX_Enemy_Weizuoshenwang_Longche_Battle_SickleSweepsAcross2 (SFX)"), m_iSoundChannel, 0.2f, m_pTransformCom, 0.01f, 2.f);
		}
	}
}

void CFS_Scythe::OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::NONE))
		return;
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
//		CAMERA_SHAKE ShakeDesc{};
//		if (eVolumeLayer == COLLISIONLAYER::ENEMY_HARDATTACK)
//		{
//			ShakeDesc.fAmplitude = 2.f;
//			ShakeDesc.fDuration = 0.15f;
//			ShakeDesc.fFovKick = 0.f;
//			ShakeDesc.fFrequency = 60.f;
//			ShakeDesc.vRotation = _float3(0.05f, 0.13f, 0.f);
//			ShakeDesc.vTranslation;
//			ShakeDesc.vTranslation;
//#ifdef _DEBUG
//			cout << "Hard" << endl;
//#endif // _DEBUG
//		}
//		else if (eVolumeLayer == COLLISIONLAYER::ENEMY_ATTACK)
//		{
//			ShakeDesc.fAmplitude = 1.f;
//			ShakeDesc.fDuration = 0.1f;
//			ShakeDesc.fFovKick = 0.f;
//			ShakeDesc.fFrequency = 60.f;
//			ShakeDesc.vRotation = _float3(0.075f, 0.075f, 0.f);
//			ShakeDesc.vTranslation;
//#ifdef _DEBUG
//			cout << "Common" << endl;
//#endif // _DEBUG
//		}
//		m_pGameInstance->OnShake(ShakeDesc);
#ifdef _DEBUG
		cout << "On Hit! scythe)" << endl;
#endif // _DEBUG
	}
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
	Safe_Release(m_pGameSystem);
}
