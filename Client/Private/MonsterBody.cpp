#include "ClientPch.h"
#include "MonsterBody.h"
#include "GameInstance.h"

CMonsterBody::CMonsterBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CMonsterBody::CMonsterBody(const CMonsterBody& Prototype)
	: CPartObject { Prototype }
{
}

HRESULT CMonsterBody::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonsterBody::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MONSTERBODY_DESC* pDesc = static_cast<MONSTERBODY_DESC*>(pArg);

	Ready_Component(pDesc);

	m_pState = pDesc->pState;

	return S_OK;
}

void CMonsterBody::Priority_Update(_float fTimeDelta)
{
}

void CMonsterBody::Update(_float fTimeDelta)
{
	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	m_pAnimMachineCom->Update(fTimeDelta, m_pModelCom, m_pState);

	//m_isAnimationFinished = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelta, nullptr);
}

void CMonsterBody::Late_Update(_float fTimeDelta)
{
	if(FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
		CRASH(this);
}

void CMonsterBody::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for(_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}
}

void CMonsterBody::Ready_Component(MONSTERBODY_DESC* pDesc)
{
	// Com_Shader
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH(m_pShaderCom)

	// Com_Model
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::TEST), pDesc->szPrototypeModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH(m_pModelCom)

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	Add_Component(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_AnimMachine"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc);
}

CMonsterBody* CMonsterBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonsterBody* pInstance = new CMonsterBody(pDevice, pContext);

	if(FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CMonsterBody");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMonsterBody::Clone(void* pArg)
{
	CMonsterBody* pClone = new CMonsterBody(*this);

	if(FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CMonsterBody (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMonsterBody::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimMachineCom);
}
