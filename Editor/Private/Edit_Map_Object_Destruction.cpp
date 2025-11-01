#include"EditorPch.h"
#include "Edit_Map_Object_Destruction.h"
#include "Edit_Map_Object_Destruction_Piece.h"

CEdit_Map_Object_Destruction::CEdit_Map_Object_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_Map_Object_Destruction::CEdit_Map_Object_Destruction(const CEdit_Map_Object_Destruction& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_Map_Object_Destruction::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEdit_Map_Object_Destruction::Initialize_Clone(void* pArg)
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

	return S_OK;
}

void CEdit_Map_Object_Destruction::Priority_Update(_float fTimeDelta)
{
}

void CEdit_Map_Object_Destruction::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{
		m_IsDestroy = false;
	}

	if(m_pGameInstance->Get_DIKeyState(DIK_G) == KEYSTATE::DOWN)
	{
		m_IsDestroy = true;
		_string ModelName = m_ModelName;
		ModelName.pop_back();
		ModelName.pop_back();
		ModelName.pop_back();
		ModelName.pop_back();

		for (_uint i = 2; i < m_pBoneModel->Get_BoneSize() - 1; ++i)
		{
			_string Name = ModelName; // 예: "SM_Sev_Roc_24BS_"

			// 1. 문자열 스트림 생성
			std::stringstream ss;

			// 2. 스트림에 포맷팅 룰 적용
			//    (3자리로 고정하고, 빈 칸은 '0'으로 채우기)
			ss << std::setw(3) << std::setfill('0') << i - 2;

			// 3. 스트림의 문자열을 Name에 추가
			Name += ss.str(); // ss.str()이 "000", "001", ..., "010", ..., "100" 등을 반환
			Name += "_LOD0";
			CEdit_Map_Object_Destruction_Piece::MAP_LOAD Desc;
			Desc.eObjectType = OBJECTTYPE::INTERACTION;
			Desc.iLevel = m_iLevel;
			Desc.iShaderPassIndex = 0;
			strcpy_s(Desc.ModelName, Name.c_str());
			_vector vScale, vRot, vTrans;
			_float4x4 TestMat = *m_pBoneModel->Get_BoneMatrixPtr(i);
			XMMatrixDecompose(&vScale, &vRot, &vTrans, XMLoadFloat4x4(&TestMat));
			_vector TT = XMQuaternionNormalize(vRot);
			_float4x4 Mat;
			XMStoreFloat4x4(&Mat,
				XMMatrixRotationQuaternion(TT) *
				XMMatrixTranslationFromVector(vTrans) *
				m_pTransformCom->Get_WorldMatrix());
			Desc.WorldMatrix = &Mat;

			m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice")
				//m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
				, m_iLevel, TEXT("Layer_Destruction_Peice"), &Desc);
		}
	}	
}

void CEdit_Map_Object_Destruction::Late_Update(_float fTimeDelta)
{
	if (!m_IsDestroy)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_Map_Object_Destruction::Render()
{
	_uint DrawModel = m_iLODIndex;
	//_uint DrawModel = 0;

	if (DrawModel > m_iNumLOD)
		DrawModel = m_iNumLOD;

	Bind_Resources();

	for (_uint i = 0; i < m_pModelComArray[DrawModel]->Get_NumMesh(); ++i)
	{
		m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		_bool HasNormal = { true };
		_bool HasMask = { true };

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

void CEdit_Map_Object_Destruction::Render_Shadow()
{
}

void CEdit_Map_Object_Destruction::Set_ImGuiOption()
{
}

HRESULT CEdit_Map_Object_Destruction::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_iLevel = pDesc->iLevel;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};

	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	lstrcat(Model, Name);
	_uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;

	m_pModelComArray.resize(V);

	for (_uint i = 0; i < V; ++i)
	{
		_wstring ModelCom = Model;
 		ModelCom.pop_back();
		ModelCom += to_wstring(i);

		_char ModelName[MAX_PATH] = {};
		sprintf_s(ModelName, "Com_Model%d", i);

		if (FAILED(Add_Component(pDesc->iLevel, Model,
			StringToWString(m_ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr)))
			CRASH("FAILED");
	}

	_wstring BoneName = TEXT("Prototype_Component_Model_");
	BoneName += Name;
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName += TEXT("Bone");
	if (FAILED(Add_Component(pDesc->iLevel, BoneName,
		StringToWString(m_ModelName) + TEXT("_Bone"), reinterpret_cast<CComponent**>(&m_pBoneModel), nullptr)))
		CRASH("FAILED");

	m_pBoneModel->Update_BoneMatrix_Map();

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;

	return S_OK;
}

void CEdit_Map_Object_Destruction::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

CEdit_Map_Object_Destruction* CEdit_Map_Object_Destruction::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_Map_Object_Destruction* pInstance = new CEdit_Map_Object_Destruction(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_Map_Object_Destruction::Clone(void* pArg)
{
	CEdit_Map_Object_Destruction* pClone = new CEdit_Map_Object_Destruction(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEdit_Map_Object_Destruction::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pBoneModel);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	m_pModelComArray.clear();
}
