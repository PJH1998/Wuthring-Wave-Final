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
	m_pSocketMatrix = pDesc->pSocketMatrix;

#ifdef _DEBUG
	m_vOffsetTrans = pDesc->vOffsetTrans;
	m_vOffsetRotate = pDesc->vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(pDesc->vOffsetRadian.x, pDesc->vOffsetRadian.y, pDesc->vOffsetRadian.z),
		XMVectorSetW(XMLoadFloat3(&pDesc->vOffsetTrans), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG
	m_isActivate = false;
    return S_OK;
}

void CCoro_Rock::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CCoro_Rock::Update(_float fTimeDelta)
{
	_matrix ComBinedMatrix;
#ifdef _DEBUG
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(m_vOffsetRotate.x, m_vOffsetRotate.y, m_vOffsetRotate.z), XMVectorSetW(XMLoadFloat3(&m_vOffsetTrans), 1.f));
#else
	_matrix matOffset = XMLoadFloat4x4(&m_OffsetMatrix);
#endif // _DEBUG

	_matrix NonScaleMatrix = XMLoadFloat4x4(m_pSocketMatrix);
	_vector vScale, vQuaternion, vTransition;
	XMMatrixDecompose(&vScale, &vQuaternion, &vTransition, NonScaleMatrix);
	NonScaleMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition);
	ComBinedMatrix = matOffset * NonScaleMatrix * m_pParentTransform->Get_WorldMatrix();
	m_pTransformCom->Set_WorldMatrix(ComBinedMatrix);
	XMStoreFloat4x4(&m_CombinedMatrix, ComBinedMatrix);

	m_pRigidBodyCom->Update_Rigidbody(ComBinedMatrix, fTimeDelta);
}

void CCoro_Rock::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
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
		_bool HasNormal{};
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		HRESULT hr = m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);
		if (SUCCEEDED(hr))
			HasNormal = true;
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}
#ifdef _DEBUG
	if (m_pRigidBodyCom)
		m_pRigidBodyCom->Render();
#endif // _DEBUG
}

void CCoro_Rock::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pShaderCom->Begin(2);

		m_pModelCom->Render(i);
	}
}

void CCoro_Rock::Change_Layer(_uint iLayer)
{
	m_pRigidBodyCom->Change_Layer(iLayer);
}

void CCoro_Rock::Change_CollisionActive(_bool isActive)
{
	m_pRigidBodyCom->IsActivate(isActive);
}

void CCoro_Rock::Play_SFX(const _wstring& wstrSoundTag, _float fVolume, _float fMin, _float fMax)
{
	if(m_iSoundChannel == -1)
		return;
	m_pGameInstance->Play_Sound(wstrSoundTag, m_iSoundChannel, fVolume, m_pTransformCom, fMin, fMax);
}

void CCoro_Rock::Bind_SoundChannel(_bool isBind)
{
	if(isBind)
		m_iSoundChannel = m_pGameInstance->Register_Channel();
	else
	{
		m_pGameInstance->Return_Channel(m_iSoundChannel);
		m_iSoundChannel = -1;
	}
}

HRESULT CCoro_Rock::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

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
	RigidbodyDesc.vExtent = _float3(1.f, 4.f, 1.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Coro_Rock/Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_tCallback.pTransform = m_pParentTransform;
	m_tCallback.fAttack = pDesc->fAttackDmg;
	m_tCallback.eType = pDesc->eType;
	m_pRigidBodyCom->Set_Desc(&m_tCallback);
	m_pRigidBodyCom->IsActivate(false);

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MonsterProp"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Coro_Rock/Com_Shader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(m_pGameInstance->Get_CurrentLevel()), TEXT("Prototype_Component_Model_CoroRock"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Coro_Rock/Com_Model");
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
