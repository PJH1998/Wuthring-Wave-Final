#include"EditorPch.h"
#include "Edit_Meteo.h"
#include"Map_Interface.h"
#include"Event_Level.h"
#include"Level_Map.h"

CEdit_Meteo::CEdit_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_Meteo::CEdit_Meteo(const CEdit_Meteo& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_Meteo::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_Meteo::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Components(pArg);

	_char Tag[MAX_PATH] = "Meteo";
	MAP_CREATE event(Tag, this);

	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Meteo"), [this](const MAP_SAVE& event) {
		if (!m_isActivate)
			return;

		auto iter = event.ModelName.find(m_ModelName);

		if (iter == event.ModelName.end())
			event.ModelName.insert(m_ModelName);

		_uint Length = strlen(m_ModelName);
		event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
		event.File.write(m_ModelName, Length);

#ifdef _DEBUG
		if (!strcmp(m_pShaderCom->Get_PassName(m_iShaderPassIndex), "SelectedObject"))
			m_iShaderPassIndex = 0;
#endif // _DEBUG

		

		event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
		event.File.write(reinterpret_cast<const char*>(&m_eObjectType), sizeof(OBJECTTYPE));
		_float4x4 Mat;
		XMStoreFloat4x4(&Mat, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const char*>(&Mat), sizeof(_float4x4));

		event.File.write(reinterpret_cast<const _char*>(&m_vSourPos), sizeof(_float4));
		event.File.write(reinterpret_cast<const _char*>(&m_vDestPos), sizeof(_float4));

		event.File.write(reinterpret_cast<const _char*>(&m_fDuration), sizeof(_float));
		event.File.write(reinterpret_cast<const _char*>(&m_farchY), sizeof(_float));

		//여기 들어가면 발동되는 거
		event.File.write(reinterpret_cast<const _char*>(&m_iTriggerIndex), sizeof(_uint));

		//이게 발동시킬 거
		event.File.write(reinterpret_cast<const _char*>(&m_iTriggerActiveIndex), sizeof(_int));
		


		//이거를 로컬로 보내거나 회전이랑 스케일까지 전부 변환된 걸 보내야함.

		});


	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
    return S_OK;
}

void CEdit_Meteo::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
	{
		m_Test = true;
	}
}

void CEdit_Meteo::Update(_float fTimeDelta)
{

#ifdef _DEBUG
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (m_iLevel == ENUM_CLASS(LEVEL::MAP))
		{
			if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
			{
				if (!m_IsRender)
					return;
				{
					_float fDistance = {};
					_vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), m_pTransformCom->Get_WorldMatrix_Inv());
					_vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), m_pTransformCom->Get_WorldMatrix_Inv()));
					if (m_pModelCom->Is_Picked(RayPos, RayDir, &fDistance))
					{
						MAP_PICK event(this, fDistance);

						m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
					}
				}
			}
		}

#endif
		//m_pModelCom = m_pModelComArray[m_iLODIndex];
	}

#ifdef _DEBUG
	if (m_Test)
		LerpPos(fTimeDelta);
}
#endif // _DEBUG



void CEdit_Meteo::Late_Update(_float fTimeDelta)
{
	if (m_IsRender)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_Meteo::Render()
{
	_uint DrawModel = m_iLODIndex;
	//_uint DrawModel = 0;

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	if (DrawModel > m_iNumLOD)
		DrawModel = m_iNumLOD;

	for (_uint i = 0; i < m_pModelComArray[DrawModel]->Get_NumMesh(); ++i)
	{

		_bool HasNormal = { true };
		_bool HasMask = { true };

		{
			if (FAILED(m_pModelComArray[DrawModel]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
			{
				m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
				HasMask = false;
			}


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
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);

		m_pModelComArray[DrawModel]->Render(i);
	}
}

void CEdit_Meteo::LerpPos(_float fTimeDelta)
{
	m_fFall += fTimeDelta;
	_float Time = m_fFall / m_fDuration;
	//Y값은 다른 거로 쓰기.
	_vector current_xz = XMVectorLerp(XMLoadFloat4(&m_vSourPos.float4), XMLoadFloat4(&m_vDestPos.float4), Time);

	_float y_arc = sin(Time * XM_PI) * m_farchY + current_xz.m128_f32[1];
	_vector CurrentPos = XMVectorSetY(current_xz, y_arc);

	m_pTransformCom->Set_State(STATE::POSITION, CurrentPos);
	if (Time >= 1.f)
	{
		m_Test = !m_Test;
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&m_vSourPos.float4));
		m_fFall = 0.f;
	}
}

void CEdit_Meteo::Set_ImGuiOption()
{
	m_pMapInterface->Set_Transform(m_pTransformCom);

	ImGui::InputFloat4(" - Start Pos", m_vSourPos.arr, "%.2f");
	ImGui::SameLine();
	if (ImGui::Button("Start"))
		XMStoreFloat4(&m_vSourPos.float4, m_pTransformCom->Get_State(STATE::POSITION));

	ImGui::SameLine();
	if (ImGui::Button("Move To Start"))
		m_pTransformCom->Set_State(STATE::POSITION, m_vSourPos.Vec);
	ImGui::InputFloat4(" - Dest Pos", m_vDestPos.arr, "%.2f");
	if (ImGui::Button("Dest"))
		XMStoreFloat4(&m_vDestPos.float4, m_pTransformCom->Get_State(STATE::POSITION));
	ImGui::SameLine();
	if (ImGui::Button("Move To Dest"))
		m_pTransformCom->Set_State(STATE::POSITION, m_vDestPos.Vec);
	ImGui::InputFloat(" - Duration", &m_fDuration, 0.1f, 0.1f, "%.1f");
	ImGui::InputFloat("- Arch Y", &m_farchY, 10.f, 10.f, "%.2f");
	ImGui::InputScalar("TriggerIndex : ", ImGuiDataType_U32, &m_iTriggerIndex);
	ImGui::InputInt("TriggerActiveIndex : ", &m_iTriggerActiveIndex);

	if (ImGui::Button("Test"))
		m_Test = true;
	ImGui::SameLine();

	if (ImGui::Button("Destroy"))
		m_isActivate = false;

}

void CEdit_Meteo::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	strcpy_s(m_ModelName, pDesc->ModelName);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));
	m_eObjectType = pDesc->eObjectType;
	m_iLevel = pDesc->iLevel;

	m_farchY = pDesc->fArchY;
	m_fDuration = pDesc->fDuration;
	m_vDestPos.float4 = pDesc->vDestPos;
	m_vSourPos.float4 =pDesc->vSourPos;
	m_iTriggerIndex = pDesc->TriggerIndex;
	m_iTriggerActiveIndex = pDesc->TriggerActiveIndex;

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
	//m_pGameInstance->Wait_Thread_End();

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Failed");
	m_pModelCom = m_pModelComArray[0];
	m_iNumLOD = m_pModelComArray.size() - 1;
}

CEdit_Meteo* CEdit_Meteo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_Meteo* pInstance = new CEdit_Meteo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_Meteo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_Meteo::Clone(void* pArg)
{
	CEdit_Meteo* pInstance = new CEdit_Meteo(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_Meteo (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_Meteo::Free()
{
	__super::Free();
	m_pModelCom = nullptr;

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pMapInterface);
	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);
	Safe_Release(m_pRigidbodyCom);

	m_pModelComArray.clear();
}
