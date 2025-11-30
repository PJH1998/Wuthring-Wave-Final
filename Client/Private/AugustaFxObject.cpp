#include "ClientPch.h"
#include "AugustaFxObject.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"
#include "AugustaEnergyBlade.h"
#include "Player_Define.h"


CAugustaFxObject::CAugustaFxObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaFxObject::CAugustaFxObject(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaFxObject::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaFxObject::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
	Ready_PartObjects(pDesc);

	m_pTransformCom->Scale({ 0.7f, 0.7f, 0.7f }); // 간격 조정.
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(80.f, -70.f, 0.f, 1.f)); // 오프셋 조절.

    return S_OK;
}

void CAugustaFxObject::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);
	
	for (auto& pEnergyBlade : m_EnergyBlades)
		pEnergyBlade->Priority_Update(fTimeDelta);
}

void CAugustaFxObject::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);
	
	// Combined Matrix 
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

    _matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);

	for (auto& pEnergyBlade : m_EnergyBlades)
		pEnergyBlade->Update(fTimeDelta);	

#ifdef _DEBUG
	m_pRigidbodyCom->Update_Rigidbody(matWorld, fTimeDelta);
#endif // _DEBUG

}

void CAugustaFxObject::Late_Update(_float fTimeDelta)
{
    CProp::Late_Update(fTimeDelta);
	

	if (m_IsVisible)
	{
		for (auto& pEnergyBlade : m_EnergyBlades)
			pEnergyBlade->Late_Update(fTimeDelta);
	}
	

#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
#endif // _DEBUG


}

void CAugustaFxObject::Render()
{

#ifdef _DEBUG
	m_pRigidbodyCom->Render();
#endif // _DEBUG

}

void CAugustaFxObject::Activate(_bool IsActivate)
{
	CProp::Activate(IsActivate);
	m_pModelCom->Clear_Animation(m_strCurrentAnimName);

	for (auto& pObj : m_EnergyBlades)
		pObj->Activate(IsActivate);
}


void CAugustaFxObject::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	
}

void CAugustaFxObject::Ready_Components(const PROP_DESC* pDesc)
{
    // 1. Components
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
        , pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        CRASH("Compute Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");

	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	RigidbodyDesc.vExtent = _float3(0.2f, 0.2f, 0.2f); // x, y, z
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

}

void CAugustaFxObject::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;
	m_eCurLevel = pDesc->eLevel; // Level 지정.
}

void CAugustaFxObject::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaFxObject::Ready_PartObjects(const PROP_DESC* pDesc)
{
	//make_pair(L"EnergyBlade", L"Prototype_GameObject_Augusta_EnergyBlade");

	m_EnergyBlades.reserve(CHILD_END);
	_float3 vScale = {};
	_float3 vRotation = {};
	_float3 vPosition = {};

	vScale = { 1.f, 1.f, 1.f };
	vPosition = { 0.f, 0.f, 0.f };
	CProp::PROP_DESC Desc = { };

	Desc = PlayerData::GetAugustaEnergyBladeCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	for (_uint i = CHILD::BONE_001; i < CHILD::CHILD_END; ++i)
	{
		_string strBoneName = Desc.strBoneName;
		strBoneName += to_string(i + 1);

		Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str()); // SocketMatrix 채워넣기.
		Desc.pParentTransform = m_pTransformCom;
		ASSERT_CRASH(Desc.pSocketMatrix);
		CAugustaEnergyBlade* pEnergyBlade =  dynamic_cast<CAugustaEnergyBlade*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Augusta_EnergyBlade")
			, PROTOTYPE::GAMEOBJECT, &Desc));

		ASSERT_CRASH(pEnergyBlade);
		pEnergyBlade->Set_ParentMatrixPtr(&m_CombinedMatrix);
		m_EnergyBlades.emplace_back(pEnergyBlade);
	}
}

void CAugustaFxObject::Ready_AttackVolumes()
{
	
}

void CAugustaFxObject::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaFxObject* CAugustaFxObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaFxObject* pInstance = new CAugustaFxObject(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaFxObject");
    }
    return pInstance;
}

CGameObject* CAugustaFxObject::Clone(void* pArg)
{
    CAugustaFxObject* pInstance = new CAugustaFxObject(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaFxObject");
    }
    return pInstance;
}

void CAugustaFxObject::Free()
{
    CProp::Free();
	for (auto& pEnergyBlade : m_EnergyBlades)
		Safe_Release(pEnergyBlade);

	m_EnergyBlades.clear();

	Safe_Release(m_pRigidbodyCom);
}
