#include "ClientPch.h"
#include "Levi_Anchor.h"
#include "GameSystem.h"

CLevi_Anchor::CLevi_Anchor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CLevi_Anchor::CLevi_Anchor(const CLevi_Anchor& Prototype)
	: CGameObject { Prototype }
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
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
	m_iSoundChannel = -1;
    return S_OK;
}

void CLevi_Anchor::Priority_Update(_float fTimeDelta)
{
	if (m_isDisolve && m_fLifeTime < m_fMaxLifeTime)
		m_fLifeTime += fTimeDelta;
}

void CLevi_Anchor::Update(_float fTimeDelta)
{
	_float fTimeRatio = m_pGameSystem->TimeLack(COLLISIONLAYER::ENEMY);

	_vector vDir = XMVectorSetW(XMLoadFloat3(&m_vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 1.f);
	if (XMVectorGetX(XMVector3Dot(m_pTransformCom->Get_State(STATE::LOOK), vDir)) >= 0.f)
		m_pTransformCom->Go_Straight(fTimeDelta * fTimeRatio);
	else
	{
		if (false == m_isDisolve)
		{
			m_isDisolve = true;
			m_pRigidBodyCom->IsActivate(false);

			//터지는 이펙트 스폰
			PREFAB_INFO Info = {};
			Info.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
			Info.pModelPtr = nullptr;

			_matrix Matrix = m_pTransformCom->Get_WorldMatrix();
			
			_vector Trans = {}, Scale = {}, Rot = {};
			XMMatrixDecompose(&Scale, &Rot, &Trans, Matrix);

			m_pGameInstance->Spawn_PoolingObject(TEXT("Leviatan_Anchor"), XMMatrixTranslationFromVector(Trans), &Info);
			m_pGameInstance->Play_Sound_Dynamic(TEXT("boss_fuludelisi_attack51_p2 (SFX)"), m_iSoundChannel, 0.4f);
		}
	}
	
	if(false == m_isDisolve)
		m_pRigidBodyCom->Update_Rigidbody(XMMatrixTranslation(0.f, 0.f, -1.f) * m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CLevi_Anchor::Late_Update(_float fTimeDelta)
{
	if(m_fLifeTime >= m_fMaxLifeTime)
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel);
		m_pGameInstance->Return_Channel(m_iSoundChannel);
		m_iSoundChannel = -1;
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
			_bool HasNormal = { false };
			if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = true;
			if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
				CRASH("Ready g_HasNormal Failed");
			//m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));
			m_pShaderCom->Begin(ENUM_CLASS(SHADER_MONSTERPROP::DEFAULTPASS));

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
	m_isDisolve = false;
	m_isActivate = true;
	m_fLifeTime = 0.f;
	m_iSoundChannel = m_pGameInstance->Register_Channel();
	//데칼 스폰 vTargetPos 기준으로 호출하면 될듯.
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
	m_pRigidBodyCom->IsActivate(false);

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
		MSG_BOX("Failed to Create : CLevi_Drop");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Anchor::Clone(void* pArg)
{
	CLevi_Anchor* pClone = new CLevi_Anchor(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Drop (Clone)");
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
	Safe_Release(m_pGameSystem);
}
