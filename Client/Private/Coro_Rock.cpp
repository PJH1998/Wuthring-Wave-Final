#include "ClientPch.h"
#include "Coro_Rock.h"
#include "AttackVolume.h"

CCoro_Rock::CCoro_Rock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CCoro_Rock::CCoro_Rock(const CCoro_Rock& Prototype)
	: CPartObject{ Prototype }
{
}

HRESULT CCoro_Rock::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCoro_Rock::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	CORO_ROCK_DESC* pDesc = static_cast<CORO_ROCK_DESC*>(pArg);
	Ready_Component(pDesc);

    return S_OK;
}

void CCoro_Rock::Priority_Update(_float fTimeDelta)
{
}

void CCoro_Rock::Update(_float fTimeDelta)
{
}

void CCoro_Rock::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CCoro_Rock::Render()
{
	if (FAILED(Bind_Resources()))
		return;

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));

		m_pModelCom->Render(i);
	}
}

void CCoro_Rock::Change_Layer(_uint iLayer)
{
	m_pRigidBodyCom->Change_Layer(iLayer);
}

HRESULT CCoro_Rock::Bind_Resources()
{
	return S_OK;
}

void CCoro_Rock::Ready_Component(CORO_ROCK_DESC* pDesc)
{
	//Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_HARDATTACK);
	RigidbodyDesc.vExtent = _float3(2.f, 2.f, 2.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});
}

void CCoro_Rock::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Coro Rock)" << endl;
#endif // _DEBUG

	}
}

CCoro_Rock* CCoro_Rock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCoro_Rock* pInstance = new CCoro_Rock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCoro_Rock");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCoro_Rock::Clone(void* pArg)
{
	CCoro_Rock* pClone = new CCoro_Rock(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CCoro_Rock (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CCoro_Rock::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
