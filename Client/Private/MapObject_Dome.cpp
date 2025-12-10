#include"ClientPch.h"
#include "MapObject_Dome.h"

CMapObject_Dome::CMapObject_Dome(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CMapObject_Dome::CMapObject_Dome(const CMapObject_Dome& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CMapObject_Dome::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Dome::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);

    return S_OK;
}

void CMapObject_Dome::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Dome::Update(_float fTimeDelta)
{
}

void CMapObject_Dome::Late_Update(_float fTimeDelta)
{
}

void CMapObject_Dome::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	//if (!m_IsRender)
	//	return;

	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	if (m_pModelCom->Get_MeshState(m_iLODIndex) != LOADSTATE::LOADED)
	{
		if (m_pModelCom->Get_MeshState(m_iLODIndex) == LOADSTATE::NOTLOADED)
			m_pModelCom->Request_LOD(m_iLODIndex);

		m_pGameInstance->Add_Render_StaticObject(this, m_iLODIndex = m_pModelCom->Get_ReadyLOD());
		return;
	}
	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map"), iIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix", pEffect);
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW), pEffect);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ), pEffect);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK, pEffect)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr, pEffect);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, pEffect);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, pEffect)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0, pEffect);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0, pEffect)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool), pEffect);
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool), pEffect);

		m_pShaderCom->Begin(m_iShaderPassIndex, pDeferredContext, pEffect);

		m_pModelCom->Render(m_iLODIndex, i, pDeferredContext);
	}
}

void CMapObject_Dome::Render_Shadow()
{
}

void CMapObject_Dome::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

BoundingBox* CMapObject_Dome::Get_BoundingBox()
{
    return m_pBoundingBox;
}

void CMapObject_Dome::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	_wstring ModelName = Model;
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();

	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
#ifdef _DEBUG
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
#endif

#ifndef _DEBUG
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
#endif
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
}

CMapObject_Dome* CMapObject_Dome::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Dome* pInstance = new CMapObject_Dome(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Dome::Clone(void* pArg)
{
	CMapObject_Dome* pClone = new CMapObject_Dome(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Dome::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
}
