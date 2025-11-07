#include"ClientPch.h"
#include "MapObject_Instance.h"

CMapObject_Instance::CMapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }

{
}

CMapObject_Instance::CMapObject_Instance(const CMapObject_Instance& Prototype)
	: CStaticObject{ Prototype }
{
}

HRESULT CMapObject_Instance::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);

	m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);
    return S_OK;
}

void CMapObject_Instance::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Instance::Update(_float fTimeDelta)
{
}

void CMapObject_Instance::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_StaticObject(this);
}

void CMapObject_Instance::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{

	_uint iLODIndex = m_iLODIndex;
	if (m_iNumLOD <= iLODIndex)
		iLODIndex = m_iNumLOD;

	ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map_Instance"), iIndex);
	//m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix", pEffect);
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW), pEffect);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ), pEffect);

	_uint iNumMesh = m_pModelComArray[iLODIndex]->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		_bool HasNormal = { true };
		_bool HasMask = { true };

		if (FAILED(m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect)))
		{
			//m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect);
			HasMask = false;
		}
		if (HasMask)
		{
			m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, pEffect);

			if (FAILED(m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, pEffect)))
				HasNormal = false;
		}
		else
		{
			m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0, pEffect);

			if (FAILED(m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0, pEffect)))
				HasNormal = false;
		}

		//m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool), pEffect);
		//m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool), pEffect);
		m_pShaderCom->Begin(0, pDeferredContext, pEffect);

		m_pModelComArray[iLODIndex]->Render(i, pDeferredContext);

		// Clear Pre Resource
		//m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, pEffect);
		//m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, pEffect);
		//m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect);
	}
}

void CMapObject_Instance::Render_Shadow()
{
}

void CMapObject_Instance::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	//m_iSaveIndex = pDesc->iSaveIndex;

	CMesh_Instance::MESH_INST_DESC Desc{};
	Desc.iNumInstance = pDesc->iNumInstance;
	Desc.pTransformMatrix = pDesc->InstanceWorldMatrix;

	_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
	ProtoName += StringToWString(pDesc->ModelName);

	//_uint V = pDesc->ModelName[strlen(pDesc->ModelName) - 1] - '0' + 1;
	_uint V = 1;

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	m_pModelComArray.resize(V);

	for (_uint i = 0; i < V; ++i)
	{
		//_wstring ModelCom = Model;
		//ModelCom.pop_back();
		//ModelCom += to_wstring(i);

		_char ModelName[MAX_PATH] = {};
		sprintf_s(ModelName, "Com_Model%d", i);
		if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ProtoName,
			StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), &Desc)))
			CRASH("FAILED");


	}
	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	// ShadowShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_ShadowShader"), reinterpret_cast<CComponent**>(&m_pShadowShaderCom), nullptr)))
		CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	////RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	//XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.vPos = pDesc->vBoundingPos;
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.vExtent = pDesc->vBoundingExtends;

	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.vPos = m_pBoundingBox->Center;
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	//RigidbodyDesc.vExtent = m_pBoundingBox->Extents;

	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));
}

CMapObject_Instance* CMapObject_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Instance* pInstance = new CMapObject_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Instance::Clone(void* pArg)
{
	CMapObject_Instance* pClone = new CMapObject_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	m_pModelComArray.clear();
}
