#include "ClientPch.h"
#include "Levi_Anchor.h"

CLevi_Anchor::CLevi_Anchor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CLevi_Anchor::CLevi_Anchor(const CLevi_Anchor& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CLevi_Anchor::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLevi_Anchor::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	ANCHORDESC* pDesc = static_cast<ANCHORDESC*>(pArg);
	Ready_Component(pDesc);
	//m_wstrEffectTag = pDesc->wstrEffectTag;
	m_fMaxLifeTime = 1.f;
	m_isActivate = false;
    return S_OK;
}

void CLevi_Anchor::Priority_Update(_float fTimeDelta)
{
	if (m_isDisolve && m_fLifeTime < m_fMaxLifeTime)
		m_fLifeTime += fTimeDelta;
}

void CLevi_Anchor::Update(_float fTimeDelta)
{
	_vector vDir = XMVectorSetW(XMLoadFloat3(&m_vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 1.f);
	if (XMVectorGetX(XMVector3Dot(m_pTransformCom->Get_State(STATE::LOOK), vDir)) >= 0.f)
		m_pTransformCom->Go_Straight(fTimeDelta);
	else
	{
		if (false == m_isDisolve)
		{
			m_isDisolve = true;
			m_pRigidBodyCom->IsActivate(false);
		}
	}
	
	if(false == m_isDisolve)
		m_pRigidBodyCom->Update_Rigidbody(XMMatrixTranslation(0.f, 0.f, -1.f) * m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CLevi_Anchor::Late_Update(_float fTimeDelta)
{
	if(m_fLifeTime >= m_fMaxLifeTime)
	{
		m_isActivate = false;
		return;
	}
	
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
		return;
}

void CLevi_Anchor::Render()
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
			m_pShaderCom->Begin(0);

			m_pModelCom->Render(i);
		}
	}
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CLevi_Anchor::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	ANCHORRESET* pDesc = static_cast<ANCHORRESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_vTargetPos = pDesc->vTargetPos;
	m_pRigidBodyCom->IsActivate(true);
	m_isActivate = true;
	m_fLifeTime = 0.f;
}

HRESULT CLevi_Anchor::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CLevi_Anchor::Ready_Component(ANCHORDESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL);
	RigidbodyDesc.vExtent = _float3(1.4f, 1.f, 1.f);
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
	m_CallBack.eType = TEXT_COLOR_TYPE::DARK;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);

	// Com_Shader 
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MonsterProp"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Anchor/Com_Shader");

	// Com_Model
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_Model_Leviatan_Anchor"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Anchor/Com_Model");
	
}

void CLevi_Anchor::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if(iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Levi Anchor)" << endl;
#endif // _DEBUG
	}
}

CLevi_Anchor* CLevi_Anchor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Anchor* pInstance = new CLevi_Anchor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Anchor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Anchor::Clone(void* pArg)
{
	CLevi_Anchor* pClone = new CLevi_Anchor(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Anchor (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Anchor::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
