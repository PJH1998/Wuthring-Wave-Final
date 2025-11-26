#include "ClientPch.h"
#include "CLeviatan.h"
#include "AttackVolume.h"
#include "GameSystem.h"

CLeviatan::CLeviatan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CLeviatan::CLeviatan(const CLeviatan& Prototype)
	: CActor { Prototype }
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
}

HRESULT CLeviatan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeviatan::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	LEVIATAN_DESC* pDesc = static_cast<LEVIATAN_DESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vInitRotate.x), XMConvertToRadians(pDesc->vInitRotate.y), XMConvertToRadians(pDesc->vInitRotate.z));
	m_pTransformCom->Rotation_Quaternion(vQuat);

#pragma region ATTACK_STATE
	m_fAttackCoolTime[ATK_PATTERN::ATTACK1] = 3.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK2] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK3] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK4] = 5.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK5] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK6] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK7] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK9] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK10] = 4.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK11] = 3.f;
#pragma endregion
	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
	_float temp{};
	m_pModelCom->Play_Animation_CPU(pDesc->pAnimationTag, 0.f, &temp);

	m_fHP = pDesc->fHP;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_fMaxStamina = pDesc->fMaxStamina;
	m_fStamina = m_fMaxStamina;
	m_fParalysisAcc = 5.f;
	m_fHitStopRatio = 1.f;
	m_ShaderIndices[LEVIATAN_SHADER::FX] = ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL);
	return S_OK;
}

void CLeviatan::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
	if (m_isTrigger == true)
		m_isDetecting = true;
	else
		m_isDetecting = false;
	m_isTrigger = false;

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Priority_Update(fTimeDelta);
	}
}

void CLeviatan::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);
	// 1. 행동트리로 상태 갱신
	if(m_pBehaviorTreeCom)
		m_pBehaviorTreeCom->tick(this);
	After_Condition(fTimeDelta);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	if(m_pAnimMachineCom)
		m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pFacialComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); // gpu
	//m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio); //cpu

	//3. 거리 보간
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (m_isDist_Interp_Enable)
	{
		_float temp = clamp(m_fDistanceNonY - 1.5f, 0.f, 1.f);
		m_pColliderCom->Update(vVelocity / fTimeDelta * temp);
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	
	// 탐지 볼륨 갱신
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	//4. 충돌 상호작용 볼륨 갱신
	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Update(fTimeDelta);
	}
	m_pParryVolume->Update(fTimeDelta);

	//5. 파츠 갱신
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Update(fTimeDelta);
	}
}

void CLeviatan::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_fStamina <= 0.f && m_fParalysisAcc >= 5.f)
		m_isParalysis = true;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Late_Update(fTimeDelta);
	}
}

void CLeviatan::Render()
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
		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		_bool HasMask = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			HasMask = true;

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (FAILED(m_pShaderCom->Bind_Value("g_HasSkinMask", &HasMask, sizeof(_bool))))
			CRASH("Ready g_HasSkinMask Failed");

		if (FAILED(m_pModelCom->Bind_MorphedResult(m_pShaderCom, i, "g_MorphedVertices")))
			CRASH("Bind Morph Result Failed");

		m_pShaderCom->Begin(m_ShaderIndices[i]);

		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}

#ifdef _DEBUG
#pragma region ATTACK_VOLUME
	for (_uint i = 0; i < ATK_SOCKET::END; ++i)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Render();
	}
	m_pParryVolume->Render();
#pragma endregion
	m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CLeviatan::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

void CLeviatan::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{

		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		if (false == m_isAggro)
			m_isAggro = true;
	}
}

void CLeviatan::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);

	if (wstrTypeTag == TEXT("Attack"))
	{
		if (wstrPartTag == TEXT("FL"))
			m_pAtkVolumes[ATK_SOCKET::FOOT_L]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("FR"))
			m_pAtkVolumes[ATK_SOCKET::FOOT_R]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("RAY"))
		{
			//m_pAtkVolumes[ATK_SOCKET::RAY1]->TriggerActivate(Isactive);
		}
		else if (wstrPartTag == TEXT("G"))
		{
			m_pAtkVolumes[ATK_SOCKET::WEAPON_GL]->TriggerActivate(Isactive);
		}
	}
	else if (wstrTypeTag == TEXT("Parry"))
	{
		m_pParryVolume->TriggerActivate(Isactive);
	}
	else if (wstrTypeTag == TEXT("Gravity"))
	{
		m_pColliderCom->Set_Gravity(Isactive);
	}
	else if (wstrTypeTag == TEXT("Lerp"))
	{
		m_isTurnLerp = Isactive;
	}
	else if (wstrTypeTag == TEXT("Distance"))
	{
		m_isDist_Interp_Enable = Isactive;
	}
}

void CLeviatan::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CLeviatan::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);
	if (false)
	{

	}
	else if (wstrTypeTag == TEXT("Type"))
	{
		if (wstrAnimTag == TEXT("ATK"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_ATTACK);
		}
		else if (wstrAnimTag == TEXT("HARD"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_HARDATTACK);
		}
		else if (wstrAnimTag == TEXT("SKILL"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_SKILL);
		}
	}
	else if (wstrTypeTag == TEXT("Look"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	else if (wstrTypeTag == TEXT("LookRev"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir) * -1.f);
		_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(0.f), 0.f);
		m_pTransformCom->Turn_Quaternion(vQuat);
	}
}

HRESULT CLeviatan::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CLeviatan::Ready_Component(LEVIATAN_DESC* pDesc)
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
	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_Enter(iLayer, pDesc, Manifold);
		});

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		,TEXT("Prototype_Component_Shader_ComputeVtxAnimMorph"), TEXT("Com_ComputeShaderFacial"), reinterpret_cast<CComponent**>(&m_pFacialComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFacial");

}

void CLeviatan::Ready_PartObjects(LEVIATAN_DESC* pDesc)
{
}

void CLeviatan::Calculate_PosAndDir()
{
}

void CLeviatan::Reset_Condition(_float fTimeDelta)
{
}

void CLeviatan::After_Condition(_float fTimeDelta)
{
}

void CLeviatan::OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (m_isAggro)
			return;
		//UI Binding (몬스터 데이터 찾기용 키값, 현재 체력 변수 주소, 현재 무력화게이지 변수 주소, 텍스트 출력용 한글 wtring)
		m_pGameSystem->HUD_Bind_BossStatus(TEXT("명식 레비아탄"), "Leviatan", &m_fHP, &m_fStamina, &m_isParalysis, &m_fParalysisRatio);
		m_pGameSystem->HUD_Toggle_BossStatusUI(true);
	}
}

void CLeviatan::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return;
}

void CLeviatan::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer)
{
}

void CLeviatan::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLeviatan::TurnFix()
{
}

void CLeviatan::TurnLerp(_bool isActive)
{
}

void CLeviatan::DistanceInterpolate(_bool isActive)
{
}

_bool CLeviatan::isKnockDown()
{
	return _bool();
}

_bool CLeviatan::isAttackEnable()
{
	return _bool();
}

_bool CLeviatan::DodgeCooldown()
{
	return _bool();
}

_bool CLeviatan::Attack(_uint iIndex, _float fInterval)
{
	return _bool();
}

void CLeviatan::Attack_Arrange()
{
}

_bool CLeviatan::Back()
{
	return _bool();
}

_bool CLeviatan::Front()
{
	return _bool();
}

_bool CLeviatan::Left()
{
	return _bool();
}

_bool CLeviatan::Right()
{
	return _bool();
}

CLeviatan* CLeviatan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeviatan* pInstance = new CLeviatan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Leviatan");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeviatan::Clone(void* pArg)
{
	CLeviatan* pClone = new CLeviatan(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Leviatan (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLeviatan::Free()
{
	__super::Free();
	
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pParryVolume);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
