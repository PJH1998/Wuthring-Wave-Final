#include"ClientPch.h"
#include "MapObject.h"
#include "MapObject_Destruction.h"
#include"GameSystem.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
	: CStaticObject{ Prototype },m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
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

	Sync_Sectors();

	//m_pGameInstance->Begin_ShadowMap();
	//Render_Shadow();
	//m_pGameInstance->End_ShadowMap();

	if (FAILED(m_pGameInstance->Add_Render_ShadowMapObject(this)))
		return E_FAIL;

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

void CMapObject::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	if (!m_IsRender)
		return;

	_uint iLODIndex = m_iLODIndex;
	if (m_iNumLOD <= iLODIndex)
		iLODIndex = m_iNumLOD;

	ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map"), iIndex);
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix", pEffect);
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW), pEffect);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ), pEffect);

	_uint iNumMesh = m_pModelComArray[iLODIndex]->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		_bool HasNormal = { true };
		_bool HasMask = { true };

		if (FAILED(m_pModelComArray[iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect)))
		{
			m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect);
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

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool), pEffect);
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool), pEffect);
		m_pShaderCom->Begin(m_iShaderPassIndex, pDeferredContext, pEffect);
		
		m_pModelComArray[iLODIndex]->Render(i, pDeferredContext);

		// Clear Pre Resource
		m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, pEffect);
		m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, pEffect);
		m_pModelComArray[iLODIndex]->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, pEffect);
	}
}

void CMapObject::Render_Shadow()
{
	m_pTransformCom->Bind_Matrix(m_pShadowShaderCom, "g_WorldMatrix");

	for (auto& iSector : m_Sectors)
	{
		m_pGameInstance->Bind_ShadowMap_Resources_StaticObject(m_pShadowShaderCom, "g_ShadowMapViewMatrix", "g_ShadowMapProjMatrix", iSector);

		_uint iLayer = m_pGameInstance->Get_ShadowMapLayer(iSector);
		if (FAILED(m_pShadowShaderCom->Bind_Value("g_iShadowMapLayer", &iLayer, sizeof(_uint))))
			CRASH("Failed Bind ShadowMapLayer");

		_uint iNumMesh = m_pModelComArray[0]->Get_NumMesh();

		for (_uint i = 0; i < iNumMesh; ++i)
		{
			m_pShadowShaderCom->Begin(8);

			m_pModelComArray[0]->Render(i);
		}
	}
}

BoundingBox* CMapObject::Get_BoundingBox()
{
	return m_pBoundingBox;
}

void CMapObject::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());
	//_uint V = pDesc->ModelName[strlen(pDesc->ModelName) - 1] - '0' + 1;
	_uint V = 1;

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
	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");
	// ShadowShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_ShadowShader"), reinterpret_cast<CComponent**>(&m_pShadowShaderCom), nullptr)))
		CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	if (pDesc->eObjectType != OBJECTTYPE::NONRIGID)
	{
		CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
		RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
		XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
		RigidbodyDesc.eShape = SHAPE::MESH;
		XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
		RigidbodyDesc.eType = EMotionType::Static;
		RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
		RigidbodyDesc.pModel = m_pModelComArray[0];

		//CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
		//RigidbodyDesc.vPos = pDesc->vBoundingPos;
		//RigidbodyDesc.eShape = SHAPE::BOX;
		//RigidbodyDesc.eType = EMotionType::Static;
		//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
		//RigidbodyDesc.vExtent = pDesc->vBoundingExtends;
		
		Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
			TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	}
	else
	{
		m_pGameSystem->TriggerRegister(11, [this](void* pArg) {
			m_isActivate = false;
			m_IsRender = false;
			});
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
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);
	Safe_Release(m_pRigidbodyCom);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	m_pModelComArray.clear();
}
