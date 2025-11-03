#include "MapObject_Destruction.h"
#include"EditorPch.h"
#include "Edit_MapObject_Destruction.h"
#include "Edit_MapObject_Destruction_Piece.h"
#include"Level_Map.h"
#include"Event_Level.h"
#include"Map_Interface.h"

CEdit_MapObject_Destruction::CEdit_MapObject_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_MapObject_Destruction::CEdit_MapObject_Destruction(const CEdit_MapObject_Destruction& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Destruction::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEdit_MapObject_Destruction::Initialize_Clone(void* pArg)
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
	m_vImpulsePos[0] = pDesc->m_vImpulsePos.x;
	m_vImpulsePos[1] = pDesc->m_vImpulsePos.y;
	m_vImpulsePos[2] = pDesc->m_vImpulsePos.z;

	m_vImpulsePower[0] = pDesc->m_vImpulsePower.x;
	m_vImpulsePower[1] = pDesc->m_vImpulsePower.y;
	m_vImpulsePower[2] = pDesc->m_vImpulsePower.z;

	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);

#ifdef _DEBUG
	_char Tag[MAX_PATH] = "Destruction";
	MAP_CREATE event(Tag, this);

	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Destruction"), [this](const MAP_SAVE& event) {
		if (!m_isActivate)
			return;

		//寃쎈줈 吏?뺥븷 ???곸쐞 ?대뜑???ㅼ뿉 LOD 鍮쇨퀬. ?대뜑瑜?吏?? 洹몃━怨?洹??덉뿉 ?덈뒗 ?대뜑 ?섏쐞 1媛??뚮㈃??.dat???쎄퀬 媛앹껜 ?덉뿉 ?ｊ린?

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
			auto iter = event.ModelName.find(Name);

			if (iter == event.ModelName.end())
				event.ModelName.insert(Name);
		}

		{
			auto iter = event.ModelName.find(m_ModelName);

			if (iter == event.ModelName.end())
				event.ModelName.insert(m_ModelName);
		}

		{

			auto iter = event.ModelName.find(m_BoneModelName);

			if (iter == event.ModelName.end())
				event.ModelName.insert(m_BoneModelName);
		}

		_uint Length = strlen(m_ModelName);
		event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
		event.File.write(m_ModelName, Length);

		if (!strcmp(m_pShaderCom->Get_PassName(m_iShaderPassIndex), "SelectedObject"))
			m_iShaderPassIndex = 0;
		event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
		event.File.write(reinterpret_cast<const char*>(&m_eObjectType), sizeof(OBJECTTYPE));
		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));

		_float3 vBoundingBoxPos = m_pModelComArray[0]->Get_BoundingBox()->Center;
		_float3 vBoundingBoxExtends = m_pModelComArray[0]->Get_BoundingBox()->Extents;
		event.File.write(reinterpret_cast<const _char*>(&vBoundingBoxPos), sizeof(_float3));
		event.File.write(reinterpret_cast<const _char*>(&vBoundingBoxExtends), sizeof(_float3));

		event.File.write(reinterpret_cast<const _char*>(&m_vImpulsePos), sizeof(_float3));
		event.File.write(reinterpret_cast<const _char*>(&m_vImpulsePower), sizeof(_float3));

		event.File.write(reinterpret_cast<const _char*>(&m_iTriggerIndex), sizeof(_uint));

		});
#endif
	m_iTriggerIndex = pDesc->iTriggerIndex;

	return S_OK;
}

void CEdit_MapObject_Destruction::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{
		m_IsDestroy = false;
	}
}

void CEdit_MapObject_Destruction::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (m_iLevel == ENUM_CLASS(LEVEL::MAP))
		{
			if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
			{
				/*    _bool IsIn = { false };
					for (_uint i = 0; i < m_pModelComArray[0]->Get_NumMesh(); ++i)
					{
						IsIn = m_pGameInstance->IsIn_WorldSpace(m_pModelComArray[0]->Get_BoundingBox(i));
						if (IsIn)
							break;
					}*/
					//if (IsIn)
				{
					//?ш린???대┃ 理쒖쟻???섎젮硫??꾨윭?ㅽ? 而щ쭅源뚯?.

					_float fDistance = {};
					//?붾뱶??諛붽퓭?쇳븿.
					_vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), m_pTransformCom->Get_WorldMatrix_Inv());
					_vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), m_pTransformCom->Get_WorldMatrix_Inv()));
					if (m_pModelComArray[0]->Is_Picked(RayPos, RayDir, &fDistance))
					{
						MAP_PICK event(this, fDistance);

						m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
					}
				}
			}
		}
	}
#endif
}

void CEdit_MapObject_Destruction::Late_Update(_float fTimeDelta)
{
	if (!m_IsDestroy)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Destruction::Render()
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

void CEdit_MapObject_Destruction::Render_Shadow()
{

}

void CEdit_MapObject_Destruction::Set_ImGuiOption()
{
#ifdef _DEBUG
	ImGui::Text(m_ModelName);
	ImGui::SameLine();

	m_pMapInterface->Set_Transform(m_pTransformCom);


	ImGui::Text("ImPulse Position");
	{
		ImGui::PushItemWidth(300.0f);
		ImGui::InputFloat3("ImPulse_Pos", m_vImpulsePos);
	}
	ImGui::SameLine();
	if (ImGui::Button("Cam Pos"))
	{
		_float4 Pos = *m_pGameInstance->Get_CamPos();
		m_vImpulsePos[0] = Pos.x;
		m_vImpulsePos[1] = Pos.y;
		m_vImpulsePos[2] = Pos.z;
	}

	ImGui::Text("ImPulse Power");
	{
		ImGui::PushItemWidth(300.0f);
		ImGui::InputFloat3("ImPulse_Power", m_vImpulsePower);
	}

	ImGui::InputScalar("TriggerIndex", ImGuiDataType_U32, &m_iTriggerIndex);

	if (ImGui::Button("Create_Particles"))
		Create_Particles();

	if (ImGui::Button("Destroy"))
		m_isActivate = false;
#endif
}

void CEdit_MapObject_Destruction::Create_Particles()
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
		CEdit_MapObject_Destruction_Piece::MAP_LOAD Desc;
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

		_vector Pos = XMVectorSet(m_vImpulsePos[0], m_vImpulsePos[1], m_vImpulsePos[2], 1.f);
		_vector Power = XMVectorSet(m_vImpulsePower[0], m_vImpulsePower[1], m_vImpulsePower[2], 0.f);

		_vector vDeltaPos = XMVectorSetW(XMLoadFloat3(reinterpret_cast<_float3*>(&Mat.m[3])) - Pos, 0.f);

		XMStoreFloat3(&Desc.vImpulse, vDeltaPos * Power);
		//m_pGameInstance->Add_PoolingObject(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice")
		//	, m_iLevel, TEXT("Layer_Destruction_Peice"), TEXT("Pool_Test")+to_wstring(i), 2, &Desc);

		//m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice")
		//	//m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
		//	, m_iLevel, TEXT("Layer_Destruction_Peice"), &Desc);

		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Test") + to_wstring(i), XMLoadFloat4x4(Desc.WorldMatrix), &Desc);
	}
}

HRESULT CEdit_MapObject_Destruction::Ready_Component(void* pArg)
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

		if (FAILED(Add_Component(pDesc->iLevel, ModelCom,
			StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr)))
			CRASH("FAILED");
	}

	_wstring BoneName = Name;
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName.pop_back();
	BoneName += TEXT("Bone");
	
	strcpy_s(m_BoneModelName, WStringToString(BoneName).c_str());

	if (FAILED(Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Model_") + BoneName,
		StringToWString(m_ModelName) + TEXT("_Bone"), reinterpret_cast<CComponent**>(&m_pBoneModel), nullptr)))
		CRASH("FAILED");

	m_pBoneModel->Update_BoneMatrix_Map();

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;

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
		CEdit_MapObject_Destruction_Piece::MAP_LOAD Desc;
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

		_vector Pos = XMVectorSet(m_vImpulsePos[0], m_vImpulsePos[1], m_vImpulsePos[2], 1.f);
		_vector Power = XMVectorSet(m_vImpulsePower[0], m_vImpulsePower[1], m_vImpulsePower[2], 0.f);

		_vector vDeltaPos = XMVectorSetW(XMLoadFloat3(reinterpret_cast<_float3*>(&Mat.m[3])) - Pos, 0.f);

		XMStoreFloat3(&Desc.vImpulse, vDeltaPos * Power);
		if (FAILED(m_pGameInstance->Add_PoolingObject(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice")
			, m_iLevel, TEXT("Layer_Destruction_Peice"), TEXT("Pool_Test") + to_wstring(i), 7, &Desc)))
			return S_OK;
	}
	return S_OK;
}

void CEdit_MapObject_Destruction::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}


CEdit_MapObject_Destruction* CEdit_MapObject_Destruction::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Destruction* pInstance = new CEdit_MapObject_Destruction(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Destruction::Clone(void* pArg)
{
	CEdit_MapObject_Destruction* pClone = new CEdit_MapObject_Destruction(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEdit_MapObject_Destruction::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pBoneModel);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);

	Safe_Release(m_pMapInterface);
	m_pModelComArray.clear();
}
