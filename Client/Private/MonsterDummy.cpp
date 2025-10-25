#include "ClientPch.h"
#include "MonsterDummy.h"

CMonsterDummy::CMonsterDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMonsterDummy::CMonsterDummy(const CMonsterDummy& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CMonsterDummy::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMonsterDummy::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Failed to Cloned : MonsterDummy");

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;


	MONSTER_DUMMY_DESC* pDesc = static_cast<MONSTER_DUMMY_DESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vPos), 1.f));
	Ready_Component(pDesc->PreTransformationMatrix);

    return S_OK;
}

void CMonsterDummy::Priority_Update(_float fTimeDelta)
{
}

void CMonsterDummy::Update(_float fTimeDelta)
{
	// GUI
	ImGui::Begin("Monster Dummy");

	_vector vCurrentPos = m_pTransformCom->Get_State(STATE::POSITION);
	_char szPos[MAX_PATH] = {};
	sprintf_s(szPos, MAX_PATH, "X : %.2f / Y : %.2f / Z : %.2f", vCurrentPos.m128_f32[0], vCurrentPos.m128_f32[1], vCurrentPos.m128_f32[2]);
	ImGui::Text(szPos);

	ImGui::Text("Trans");
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&m_vPos), "%.2f");
	if (ImGui::Button("Apply"))
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vPos), 1.f));
		m_pColliderCom->Set_Position(XMLoadFloat3(&m_vPos));
	}
	ImGui::End();
	// =========

	_vector vVelocity = XMVectorSet(0.f, -9.8f, 0.f, 0.f);
	m_pColliderCom->Update(vVelocity);
}

void CMonsterDummy::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMonsterDummy::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

		_bool HasNormal = { false };
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			HasNormal = true;

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
#endif
}

void CMonsterDummy::Render_Shadow()
{
}

void CMonsterDummy::Render_OutLine()
{
}

void CMonsterDummy::Ready_Component(const _fmatrix& PreTransformMatrix)
{
	// Com_Model
	m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat");
	ASSERT_CRASH(m_pModelCom);

	// Com_Shader
	m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
	ASSERT_CRASH(m_pShaderCom);

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 9.f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 5.f;
	ColliderDesc.fRadius = 5.f;
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	m_pColliderCom->Set_Desc(m_pTransformCom);

}

CMonsterDummy* CMonsterDummy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonsterDummy* pInstance = new CMonsterDummy(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : MonsterDummy");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonsterDummy::Clone(void* pArg)
{
	CMonsterDummy* pInstance = new CMonsterDummy(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : MonsterDummy");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonsterDummy::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pColliderCom);
}
