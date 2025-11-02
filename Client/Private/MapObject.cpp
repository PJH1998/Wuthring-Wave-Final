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
	//_vector vPos = XMVectorSet(m_pGameInstance->Rand(-2000.f, 2000.f), m_pGameInstance->Rand(-2000.f, 2000.f), m_pGameInstance->Rand(-2000.f, 2000.f), 1.f);
	//m_pTransformCom->Set_State(STATE::POSITION, vPos);
	Ready_Component(pArg);
	m_iNumLOD = static_cast<_uint>(m_pModelComArray.size()) - 1;
	//Sync_BoundingBox(m_pModelComArray[0]->Get_BoundingBox(), m_pTransformCom->Get_WorldMatrix());
	//m_pGameInstance->Add_To_OctoTree(this, m_pModelComArray[0]->Get_BoundingBox());
	m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);

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
	//For_Test, when Object's m_iNumLOD is Lower Than m_iLODIndex Clip Operation Disable
	
	//if (m_iNumLOD <= m_iLODIndex)
	//	return;

	{
		//m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
		//m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
		//m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

		//_uint iNumMesh = m_pModelComArray[m_iLODIndex]->Get_NumMesh();
		//for (_uint i = 0; i < iNumMesh; ++i)
		//{
		//	m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//	m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);

		//	if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
		//		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		//	m_pShaderCom->Begin(0);

		//	m_pModelComArray[m_iLODIndex]->Render(i);
		//}
	}

	if (m_iNumLOD <= m_iLODIndex)
		m_iLODIndex = m_iNumLOD;

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelComArray[m_iLODIndex]->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);

		_bool HasNormal = { true };
		_bool HasMask = { true };

		if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
			HasMask = false;
		if (HasMask)
		{
			m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));
		m_pShaderCom->Begin(m_iShaderPassIndex);

		m_pModelComArray[m_iLODIndex]->Render(i);
	}
}

BoundingBox* CMapObject::Get_BoundingBox()
{
	return m_pModelComArray[0]->Get_BoundingBox();
}

void CMapObject::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());
	_uint V = pDesc->ModelName[strlen(pDesc->ModelName) - 1] - '0' + 1;

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	m_pModelComArray.resize(V);

	for (_uint i = 0; i < V; ++i)
	{
		_wstring ModelCom = Model;
		ModelCom.pop_back();
		ModelCom += to_wstring(i);

		_char ModelName[MAX_PATH] = {};
		sprintf_s(ModelName, "Com_Model%d", i);
		if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ModelCom,
			StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr)))
			CRASH("FAILED");

	}
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	if(pDesc->eObjectType != OBJECTTYPE::NONRIGID)
	{
		CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
		RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
		XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
		RigidbodyDesc.eShape = SHAPE::MESH;
		XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
		RigidbodyDesc.eType = EMotionType::Static;
		RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
		RigidbodyDesc.pModel = m_pModelComArray[0];

		Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
			TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
	}

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
	Safe_Release(m_pRigidbodyCom);
	Safe_Delete(m_pBoundingBox);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	m_pModelComArray.clear();
}
