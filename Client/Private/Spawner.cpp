#include "ClientPch.h"
#include "Spawner.h"

CSpawner::CSpawner(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSpawner::CSpawner(const CSpawner& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CSpawner::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpawner::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	SPAWNERDESC* pDesc = static_cast<SPAWNERDESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f));

	Ready_Component(pDesc);
	
	m_fSpawnTime = pDesc->fSpawnTime;
	m_wstrPoolTags = pDesc->wstrPoolTags;
	m_iNumSpawnObjects = m_wstrPoolTags.size();
	for (_uint i = 0; i < m_iNumSpawnObjects; ++i)
	{
		_vector vTransition = XMVectorSetW(XMLoadFloat3(&pDesc->vSpawnPositions[i]), 1.f);
		_vector vQuaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(
			pDesc->vSpawnPositions[i].x), XMConvertToRadians(pDesc->vSpawnPositions[i].y), XMConvertToRadians(pDesc->vSpawnPositions[i].z));
		_float4x4 InitMatrix{};
		XMStoreFloat4x4(&InitMatrix, XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition));
		m_SpawnMatrix.push_back(InitMatrix);
	}

    return S_OK;
}

void CSpawner::Priority_Update(_float fTimeDelta)
{
	if (m_fTimeAcc <= m_fSpawnTime)
		m_fTimeAcc += fTimeDelta;
}

void CSpawner::Update(_float fTimeDelta)
{
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CSpawner::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::STATIC, this)))
		return;
}

void CSpawner::Render()
{
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif // _DEBUG
}

void CSpawner::Ready_Component(SPAWNERDESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});
}

void CSpawner::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (m_fTimeAcc >= m_fSpawnTime)
		{
			for (_uint i = 0; i < m_iNumSpawnObjects; ++i)
			{
				m_pGameInstance->Spawn_PoolingObject(m_wstrPoolTags[i], XMLoadFloat4x4(&m_SpawnMatrix[i]), nullptr);
			}
			m_fTimeAcc = 0.f;
		}
	}
}

CSpawner* CSpawner::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpawner* pInstance = new CSpawner(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CSpawner");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSpawner::Clone(void* pArg)
{
	CSpawner* pClone = new CSpawner(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CSpawner (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpawner::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
}
