#include"EditorPch.h"
#include "Edit_MapObject_Destruction_Piece.h"

CEdit_MapObject_Destruction_Piece::CEdit_MapObject_Destruction_Piece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice, pContext)
{
}

CEdit_MapObject_Destruction_Piece::CEdit_MapObject_Destruction_Piece(const CEdit_MapObject_Destruction_Piece& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Destruction_Piece::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_MapObject_Destruction_Piece::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	strcpy_s(m_ModelName, pDesc->ModelName);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	m_iNumLOD = m_pModelComArray.size() - 1;


	Sync_BoundingBox(m_pModelComArray[0]->Get_BoundingBox(), m_pTransformCom->Get_WorldMatrix());

	_vector vScale, vRotation, vTranslation;

	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

	XMStoreFloat3(&m_vScale, vScale);
	XMStoreFloat3(&m_vTranslation, vTranslation);
	m_vNewScale = m_vScale;
	m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
	m_vNewTranslation = m_vTranslation;
	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_eObjectType = pDesc->eObjectType;


	//XMStoreFloat3(&vImpluse, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), 0.f) * -100.f);
	m_pRigidbodyCom->Impulse(pDesc->vImpulse);
	return S_OK;
}

void CEdit_MapObject_Destruction_Piece::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Destruction_Piece::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{
		m_pRigidbodyCom->IsActivate(false);
	}
}

void CEdit_MapObject_Destruction_Piece::Late_Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Destruction_Piece::Render()
{
	_uint DrawModel = m_iLODIndex;
	//_uint DrawModel = 0;

	if (DrawModel > m_iNumLOD)
		DrawModel = m_iNumLOD;

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	for (_uint i = 0; i < m_pModelComArray[DrawModel]->Get_NumMesh(); ++i)
	{
		m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		_bool HasNormal = { true };
		_bool HasMask = { true };

		{
			if (FAILED(m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
				HasMask = false;


			if (HasMask)
			{
				m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

				if (FAILED(m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
					HasNormal = false;
			}
			else
			{
				m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0);

				if (FAILED(m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
					HasNormal = false;
			}

			m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
			m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

			m_pShaderCom->Begin(m_iShaderPassIndex);

			m_pModelComArray[DrawModel]->Render(i);
		}
	}
}
void CEdit_MapObject_Destruction_Piece::Render_Shadow()
{
}

void CEdit_MapObject_Destruction_Piece::Set_ImGuiOption()
{
}

HRESULT CEdit_MapObject_Destruction_Piece::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	m_iLevel = pDesc->iLevel;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	m_pModelComArray.resize(1);
	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	lstrcat(Model, Name);
	if (FAILED(Add_Component(pDesc->iLevel, Model,
		StringToWString(m_ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[0]), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;


	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Dynamic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.vExtent = m_pModelComArray[0]->Get_BoundingBox()->Extents;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

    return S_OK;
}

void CEdit_MapObject_Destruction_Piece::Bind_Resources()
{
}

CEdit_MapObject_Destruction_Piece* CEdit_MapObject_Destruction_Piece::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Destruction_Piece* pInstance = new CEdit_MapObject_Destruction_Piece(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Destruction_Piece::Clone(void* pArg)
{
	CEdit_MapObject_Destruction_Piece* pClone = new CEdit_MapObject_Destruction_Piece(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEdit_MapObject_Destruction_Piece::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	m_pModelComArray.clear();
}
