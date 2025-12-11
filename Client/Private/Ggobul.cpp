#include "ClientPch.h"
#include "Ggobul.h"
#include "AttackVolume.h"
#include "GameSystem.h"

CGgobul::CGgobul(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CGgobul::CGgobul(const CGgobul& Prototype)
	:CActor { Prototype }
	, m_pGameSystem { CGameSystem::GetInstance()}
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CGgobul::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGgobul::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	GGOBUL_DESC* pDesc = static_cast<GGOBUL_DESC*>(pArg);
	Ready_Component(pDesc);
	Ready_Volumes(pDesc);
	Register_AllNotifies(pDesc->strFolderPath);

	for (size_t i = 0; i < GGOBULTYPE::END; ++i)
	{
		if(nullptr != m_pAttackVolumes[i])
			m_pAttackVolumes[i]->TriggerActivate(false);
	}
	m_pAttackTransform = m_pModelCom->Get_BoneMatrixPtr("HitCase");
	m_MeshEnables.resize(m_pModelCom->Get_NumMesh(), true);
	//풀링 오브젝트 자체적으로 activate 끄기
	
	m_isActivate = false;
	
	m_ShaderIndices[GGOBUL_SHADER::FX] = ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL);

    return S_OK;
}

void CGgobul::Priority_Update(_float fTimeDelta)
{
}

void CGgobul::Update(_float fTimeDelta)
{
	_bool isAnimFinished{};
	if (nullptr != m_pRootMatrix)
	{
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(m_pRootMatrix));
	}
	_float fTimeRatio = m_pGameSystem->TimeLack(COLLISIONLAYER::ENEMY);
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, isAnimFinished, fTimeDelta * fTimeRatio);
	if (isAnimFinished)
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		if (nullptr != m_pAttackVolumes[m_eType])
			m_pAttackVolumes[m_eType]->TriggerActivate(false);
		m_pRootMatrix = nullptr;
		return;
	}

	if(nullptr != m_pAttackVolumes[m_eType])
		m_pAttackVolumes[m_eType]->Update(fTimeDelta);
}

void CGgobul::Late_Update(_float fTimeDelta)
{
	//뼈 공격 볼륨 동기화 설정, 뼈에다가 맞추려면 sync 사용 X
	//m_pRigidBodyCom[m_eType]->Sync_Rigidbody(m_pTransformCom);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
		return;
}

void CGgobul::Render()
{
	Bind_Resources();

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (false == m_MeshEnables[i])
			continue;


		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}
#ifdef _DEBUG
	if (nullptr != m_pAttackVolumes[m_eType])
		m_pAttackVolumes[m_eType]->Render();
#endif
}

void CGgobul::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (false == m_MeshEnables[i])
			continue;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

void CGgobul::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	GGOBUL_RESET* pDesc = static_cast<GGOBUL_RESET*>(pArg);
	m_eType = pDesc->eType;
	m_strAnimKey = pDesc->strPatternKey;
	m_pAnimMachineCom->Reset(m_pModelCom, m_strAnimKey);
	m_pModelCom->Clear_Animation(m_strAnimKey);
	//for (_uint i = 0; i < GGOBULTYPE::END; ++i)
	//{
	//	m_pAttackVolume[i]->TriggerActivate(false);
	//}

		//				  body, down,  hammer, head, knife, fx
	if (m_strAnimKey == "SAttack03_1")
		m_MeshEnables = { true, true, false, false, false, false };
	else if (m_strAnimKey == "SAttack03_2")
		m_MeshEnables = { true, true, false, false, false, false };
	else if (m_strAnimKey == "SBehit_Block")
		m_MeshEnables = { true, true, true, false, false, false };
	else if (m_strAnimKey == "SAttack01_1")
		m_MeshEnables = { true, true, false, true, false, false };
	else if (m_strAnimKey == "SAttack02_2")
	{
		m_pRootMatrix = pDesc->pRootMatrix;
		m_MeshEnables = { true, false, false, true, false, true };
	}
	else
		m_MeshEnables = { true, false, false, true, false, false };
	m_iState = ENUM_CLASS(TEST_STATE::NONE);
	m_isActivate = true;
}

void CGgobul::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CGgobul::Ready_Component(GGOBUL_DESC* pDesc)
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
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = "SAttack01_2";
	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_AnimMachine_Ggobul"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("Ggobul/Com_AnimMachine");

}

void CGgobul::Ready_Volumes(GGOBUL_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("HitCase");
	TriggerDesc.vExtent = _float3(2.f, 2.f, 2.f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = m_fAttackDmg;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHit_Enter(iLayer, pOther, Manifold); 
		};

	//Head
	m_pAttackVolumes[GGOBULTYPE::HEAD] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[GGOBULTYPE::HEAD])
		CRASH(m_pAttackVolumes[GGOBULTYPE::HEAD]);
	m_pAttackVolumes[GGOBULTYPE::HEAD]->TriggerActivate(false);

	//Hammer
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_SKILL;
	TriggerDesc.vExtent = _float3(4.f, 4.f, 4.f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = m_fAttackDmg * 1.4f;

	m_pAttackVolumes[GGOBULTYPE::HAMMER] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[GGOBULTYPE::HAMMER])
		CRASH(m_pAttackVolumes[GGOBULTYPE::HAMMER]);
	m_pAttackVolumes[GGOBULTYPE::HAMMER]->TriggerActivate(false);

	//Knife
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_SKILL;
	TriggerDesc.vExtent = _float3(10.f, 4.f, 4.f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = m_fAttackDmg * 1.2f;

	m_pAttackVolumes[GGOBULTYPE::KNIFE] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolumes[GGOBULTYPE::KNIFE])
		CRASH(m_pAttackVolumes[GGOBULTYPE::KNIFE]);
	m_pAttackVolumes[GGOBULTYPE::KNIFE]->TriggerActivate(false);
}

void CGgobul::OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
#ifdef _DEBUG
	cout << "On Hit! GGOBUL)" << endl;
#endif // _DEBUG
}

void CGgobul::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	if(wStrColliderTag == TEXT("Attack"))
		if (nullptr != m_pAttackVolumes[m_eType])
			m_pAttackVolumes[m_eType]->TriggerActivate(Isactive);
}

void CGgobul::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CGgobul::Object_Func(const _wstring& wStrObjectTag)
{
	if (wStrObjectTag == TEXT("Knife"))
	{
		m_MeshEnables[GGOBUL_SHADER::HEAD0] = false;
		m_MeshEnables[GGOBUL_SHADER::KNIFE0] = true;
		m_pAttackVolumes[m_eType]->TriggerActivate(false);
		m_eType = GGOBULTYPE::KNIFE;
		m_pAttackVolumes[m_eType]->TriggerActivate(true);
	}
	else if (wStrObjectTag == TEXT("Hammer"))
	{
		m_MeshEnables[GGOBUL_SHADER::HEAD0] = false;
		m_MeshEnables[GGOBUL_SHADER::HAMMER0] = true;
		m_pAttackVolumes[m_eType]->TriggerActivate(false);
		m_eType = GGOBULTYPE::HAMMER;
		m_pAttackVolumes[m_eType]->TriggerActivate(true);
	}
	else if (wStrObjectTag == TEXT("Head"))
	{
		m_MeshEnables[GGOBUL_SHADER::HEAD0] = true;
		m_MeshEnables[GGOBUL_SHADER::KNIFE0] = false;
		m_MeshEnables[GGOBUL_SHADER::HAMMER0] = false;
		m_pAttackVolumes[m_eType]->TriggerActivate(false);
		m_eType = GGOBULTYPE::HEAD;
		m_pAttackVolumes[m_eType]->TriggerActivate(true);
	}
}

CGgobul* CGgobul::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGgobul* pInstance = new CGgobul(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CGgobul");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGgobul::Clone(void* pArg)
{
	CGgobul* pClone = new CGgobul(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CGgobul (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CGgobul::Free()
{
	__super::Free();

	Safe_Release(m_pAnimMachineCom);
	for (size_t i = 0; i < GGOBULTYPE::END; ++i)
	{
		Safe_Release(m_pAttackVolumes[i]);
	}
	Safe_Release(m_pGameSystem);
}
