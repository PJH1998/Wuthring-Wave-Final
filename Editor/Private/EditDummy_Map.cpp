#include "EditorPch.h"
#include "EditDummy_Map.h"

CEditDummy_Map::CEditDummy_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEditDummy { pDevice, pContext }
{
}

CEditDummy_Map::CEditDummy_Map(const CEditDummy_Map& Prototype)
	: CEditDummy { Prototype }
{
}

HRESULT CEditDummy_Map::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditDummy_Map::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Failed to Cloned : Dummy_Map");

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMY_MAP_DESC* pDesc = static_cast<DUMMY_MAP_DESC*>(pArg);

	if (FAILED(Ready_Component(pDesc->PreTransformMatrix)))
		return E_FAIL;

	return S_OK;
}

void CEditDummy_Map::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Map::Update(_float fTimeDelta)
{
}

void CEditDummy_Map::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEditDummy_Map::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			HasNormal = true;

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}
}

void CEditDummy_Map::Render_Shadow()
{
}

HRESULT CEditDummy_Map::Ready_Component(_fmatrix PreTransformMatrix)
{
	m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Map/Rock/SM_Sev_Roc_01AL/SM_Sev_Roc_01AL_LOD0.dat");
	ASSERT_CRASH(m_pModelCom);

	m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
	ASSERT_CRASH(m_pShaderCom);

	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	return S_OK;
}

CEditDummy_Map* CEditDummy_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditDummy_Map* pInstance = new CEditDummy_Map(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : EditDummy_Map");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEditDummy_Map::Clone(void* pArg)
{
	CEditDummy_Map* pInstance = new CEditDummy_Map(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : EditDummy_Map");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditDummy_Map::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
}
