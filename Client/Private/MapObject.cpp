#include"ClientPch.h"
#include "MapObject.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
	: CStaticObject{ Prototype }
{
}

HRESULT CMapObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapObject::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	Ready_Component(pArg);
	m_iNumLOD = m_pModelComArray.size() - 1;
	Sync_BoundingBox(m_pModelComArray[0]->Get_BoundingBox(0), m_pTransformCom->Get_WorldMatrix());
	m_pGameInstance->Add_To_OctoTree(this, m_pModelComArray[0]->Get_BoundingBox(0));

	/*
	?쎈뒗 ?쒖꽌.
	        _uint Length = strlen(m_ModelName);
        event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
        event.File.write(m_ModelName, Length);
        event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
        _float4x4 WorldMatrix;
        XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
        event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
	*/

	return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{
}

void CMapObject::Update(_float fTimeDelta)
{

}

void CMapObject::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelComArray[m_iLODIndex]->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);

		if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		m_pShaderCom->Begin(0);

		m_pModelComArray[m_iLODIndex]->Render(i);
	}
}

void CMapObject::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_pModelComArray.resize(4);

	// Com_Shader
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

	for (_uint i = 0; i < 4; ++i)
	{
		_wstring ModelCom = Model;
		ModelCom += to_wstring(i);
		_char ModelName[MAX_PATH] = {};
		sprintf_s(ModelName, "Com_Model%d", i);
		// Com_Model
		Add_Component(ENUM_CLASS(LEVEL::TEST), ModelCom,
			StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr);
	}

	// Com_Rigidbody
	//CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::MESH;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.pModel = m_pModelCom;
	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.pModel = m_pModelComArray[0];

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
}

CMapObject* CMapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject* pInstance = new CMapObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject::Clone(void* pArg)
{
	CMapObject* pClone = new CMapObject(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);
	m_pModelComArray.clear();
	Safe_Release(m_pRigidbodyCom);
}
