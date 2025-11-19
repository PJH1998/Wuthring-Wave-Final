#include"EditorPch.h"
#include "Edit_MapObject_Test.h"
CEdit_MapObject_Test::CEdit_MapObject_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_MapObject_Test::CEdit_MapObject_Test(const CEdit_MapObject_Test& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Test::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_MapObject_Test::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(pArg);
	//m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_pGameInstance->Rand(-1600.f, 1600.f), m_pGameInstance->Rand(-1600.f, 1600.f), m_pGameInstance->Rand(-1600.f, 1600.f), 1.f));
	_float3 vPos;
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
	m_pTransformCom->Get_State(STATE::POSITION);
	m_pBoundingBox = new BoundingBox(vPos, _float3(200.f, 200.f, 200.f));
	m_iNumLOD = m_pModelCom->Get_LastLODIndex();
	m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);
	AddRef();
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vPos = m_pBoundingBox->Center;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.vExtent = _float3(20.f,20.f,20.f);

	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);


	return S_OK;
}

void CEdit_MapObject_Test::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Test::Update(_float fTimeDelta)
{

}

void CEdit_MapObject_Test::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::BLEND, this);
	if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
		m_iIndex = 1;
	if (m_pGameInstance->Get_DIKeyState(DIK_P) == KEYSTATE::DOWN)
		m_iIndex = 2;
	if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
		m_iIndex = 3;
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
		m_iIndex = 0;
	//아니면 여기에서 Late Render를 하기 전에 LOD 파악해서 바꿔치기 하는 방법도 존재. <- 여기다가 하는 게 좀 더 좋을듯.
	//m_pGameInstance->Add_To_RenderTest(m_iIndex, this);
	//m_pModelCom->Render(0, 0);
}

void CEdit_MapObject_Test::Render()
{
}

void CEdit_MapObject_Test::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	if (m_pModelCom->Get_MeshState(m_iLODIndex) != LOADSTATE::LOADED)
	{
		if (m_pModelCom->Get_MeshState(m_iLODIndex) == LOADSTATE::NOTLOADED)
			m_pModelCom->Request_LOD(m_iLODIndex);
		//여기에 어떤 LOD인덱스 부분에 넣을건지도 넣어야됨.
		m_pGameInstance->Add_Render_StaticObject(this, m_iLODIndex = m_pModelCom->Get_ReadyLOD());
		return;
	}
	ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map"), iIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix", pEffect);
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW),pEffect);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ),pEffect);
	_bool HasNormal = { true };
	_bool HasMask = { true };

	for (_uint i = 0; i < m_pModelCom->Get_NumMesh(m_iLODIndex); ++i)
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

void CEdit_MapObject_Test::Render_Shadow()
{
}

void CEdit_MapObject_Test::Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime)
{
	m_pModelCom->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
}

HRESULT CEdit_MapObject_Test::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	pDesc->ModelName;
	m_iLevel = pDesc->iLevel;
	strcpy_s(m_ModelName, pDesc->ModelName);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};

	MultiByteToWideChar(CP_ACP, 0, pDesc->ModelName, -1, Name, strlen(pDesc->ModelName));
	lstrcat(Model, Name);
	_uint V = pDesc->ModelName[strlen(pDesc->ModelName) - 1] - '0' + 1;

	_wstring ModelCom = Model;
	//ModelCom.pop_back();
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh"),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
	//	return E_FAIL;

	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_DeferredShader_Map"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");
	return S_OK;
}

CEdit_MapObject_Test* CEdit_MapObject_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Test::Clone(void* pArg)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Test::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidbodyCom);
	
}