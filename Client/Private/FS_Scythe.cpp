#include "ClientPch.h"
#include "FS_Scythe.h"

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
	//Register_AllNotifies(pDesc->strFolderPath);

	//for (size_t i = 0; i < 5; ++i)
	//{
	//	m_pAttackVolume[i]->IsActivate(false);
	//}
	//m_pAttackTransform = m_pModelCom->Get_BoneMatrixPtr("HitCase");
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
		//m_pAttackVolume[m_eType]->IsActivate(false);
		return;
	}
	else if (m_fLifeTime <= 0.f)
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		//m_pAttackVolume[m_eType]->IsActivate(false);
		return;
	}
	m_fLifeTime -= fTimeDelta;
	//_matrix NonScaleMatrix = XMLoadFloat4x4(m_pAttackTransform) * m_pTransformCom->Get_WorldMatrix();
	//_vector vScale, vQuaternion, vTransition;
	//XMMatrixDecompose(&vScale, &vQuaternion, &vTransition, NonScaleMatrix);
	//NonScaleMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition);
	//XMStoreFloat4x4(&m_BoneCombindMatrix, NonScaleMatrix);
	//m_pAttackVolume[m_eType]->Update_Rigidbody(XMLoadFloat4x4(&m_BoneCombindMatrix), fTimeDelta);
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
#endif
}

void CFS_Scythe::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	SCYTHE_RESET* pDesc = static_cast<SCYTHE_RESET*>(pArg);
	m_strAnimKey = pDesc->strPatternKey;
	m_pAnimMachineCom->Reset(m_pModelCom, m_strAnimKey);
	if (m_strAnimKey == "SAttack04_1_Start")
		m_iState = 1;
	else
		m_iState = 0;
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

	// Com_Rigidbody(Head)
	//CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eBodyType = CRigidbody::BODY;
	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.eType = EMotionType::Kinematic;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	//RigidbodyDesc.vExtent = _float3(4.f, 4.f, 4.f);
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//
	//if (FAILED(Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first), pDesc->rigidBodyData.second,
	//	TEXT("Com_Rigidbody_Head"), reinterpret_cast<CComponent**>(&m_pAttackVolume[GGOBULTYPE::HEAD]), &RigidbodyDesc)))
	//	CRASH("Rigidbody");
	//
	//m_pAttackVolume[GGOBULTYPE::HEAD]->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
	//	OnCollide_Enter(iLayer, pDesc, Manifold);
	//	});
}

void CFS_Scythe::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	//if(wStrColliderTag == TEXT("Attack Trig"))
	//	m_pAttackVolume[]->IsActivate(Isactive);
}

void CFS_Scythe::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CFS_Scythe::Object_Func(const _wstring& wStrObjectTag)
{
}

void CFS_Scythe::OnCollide_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
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
		Safe_Release(m_pAttackVolume[i]);
	}

}
