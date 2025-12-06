#include "ClientPch.h"
#include "Projectile.h"

CProjectile::CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CProjectile::CProjectile(const CProjectile& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CProjectile::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CProjectile::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	PROJECTILEDESC* pDesc = static_cast<PROJECTILEDESC*>(pArg);
	Ready_Component(pDesc);
	m_iTargetLayers = pDesc->iTargetLayers;
	m_wstrEffectTag = pDesc->wstrEffectTag;
	m_isCollisionDestroy = pDesc->isCollisionDestroy;
	m_fMaxLifeTime = pDesc->fLifeTime;
	m_fMaxDelay = pDesc->fMaxDelay;
	m_isActivate = false;
    return S_OK;
}

void CProjectile::Priority_Update(_float fTimeDelta)
{
	if (m_fLifeTime < m_fMaxLifeTime)
		m_fLifeTime += fTimeDelta;
	else
		m_isCollision = true;
}

void CProjectile::Update(_float fTimeDelta)
{
	m_pTransformCom->Go_Straight(fTimeDelta);

	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
	if (m_fDelay <= 0.f)
	{
		PREFAB_INFO EffectDesc{};
		EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
		EffectDesc.pModelPtr = nullptr;
		m_pGameInstance->Spawn_PoolingObject(m_wstrEffectTag, m_pTransformCom->Get_WorldMatrix(), &EffectDesc);
		m_fDelay = m_fMaxDelay;
	}
	else
		m_fDelay -= fTimeDelta;
}

void CProjectile::Late_Update(_float fTimeDelta)
{
	if (m_isCollision)
	{
		if(m_isCollisionDestroy || m_fLifeTime >= m_fMaxLifeTime)
		{
			m_isActivate = false;
			m_pRigidBodyCom->IsActivate(false);
			return;
		}
	}
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
		return;
}

void CProjectile::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");

	if(m_pModelCom)
	{
		_uint iNumMesh = m_pModelCom->Get_NumMesh();
		ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
		m_pContext->VSSetShaderResources(0, 16, pNullSRV);
		m_pContext->PSSetShaderResources(0, 16, pNullSRV);
		m_pContext->CSSetShaderResources(0, 16, pNullSRV);

		for (_uint i = 0; i < iNumMesh; ++i)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
			//m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));
			_bool HasNormal { false };
			if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = true;
			if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
				CRASH("Ready g_HasNormal Failed");
			m_pShaderCom->Begin(0);

			m_pModelCom->Render(i);
		}
	}
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CProjectile::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	PROJECTILERESET* pDesc = static_cast<PROJECTILERESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_isCollision = false;
	m_pRigidBodyCom->IsActivate(true);
	m_isActivate = true;
	m_fLifeTime = 0.f;
	m_fDelay = 0.f;
}

HRESULT CProjectile::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CProjectile::Ready_Component(PROJECTILEDESC* pDesc)
{
	CRigidbody::SPHEREBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::SPHERE;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = pDesc->iLayer;
	RigidbodyDesc.fRadius = pDesc->fRadius;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = pDesc->fAttackDamage;
	//m_CallBack.pCondition = &m_iState;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = pDesc->eType;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
	m_pRigidBodyCom->IsActivate(false);
	// Com_Shader 
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MonsterProp"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Arrow/Com_Shader");

	if(0 != pDesc->wstrModelTag.length())
	{
		// Com_Model
		if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), pDesc->wstrModelTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
			CRASH("Arrow/Com_Model");
	}
}

void CProjectile::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	for( auto& iTarget : m_iTargetLayers)
	{
		if (iLayer == iTarget)
		{
			m_isCollision = true;
#ifdef _DEBUG
			cout << "On Hit! (Projectile)" << endl;
#endif // _DEBUG
			return;
		}
	}

}

CProjectile* CProjectile::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CProjectile* pInstance = new CProjectile(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CProjectile");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CProjectile::Clone(void* pArg)
{
	CProjectile* pClone = new CProjectile(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CProjectile (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CProjectile::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
