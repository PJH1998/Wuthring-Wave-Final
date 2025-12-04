#include"EditorPch.h"
#include "Edit_FireFly_Manager.h"
#include"Level_Map.h"
#include"Event_Level.h"
#include"Model_Instance_FireFly.h"
#include"Edit_FireFly.h"

CEdit_FireFly_Manager::CEdit_FireFly_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pGameInstance(CGameInstance::GetInstance()),
	m_pDevice(pDevice),m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}
HRESULT CEdit_FireFly_Manager::Initialize()
{
	m_iLevel = ENUM_CLASS(LEVEL::MAP);
	m_vPickedPos.float_4 = _float4(0.f, 0.f, 0.f, 1.f);
	m_pGameInstance->Subscribe<FLY>(ENUM_CLASS(LEVEL::STATIC), TEXT("FLY"), [this](const FLY& event) {
		m_pFly = static_cast<CEdit_FireFly*>(event.pObject);
		Safe_AddRef(m_pFly);
		});

	Create_Fly();
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Light"), [this](const MAP_SAVE& event) {
		for (auto& pFly : m_Fly)
		{
			if (!pFly.second->IsActivate())
				continue;

		}
		});


    return S_OK;
}

void CEdit_FireFly_Manager::Set_ImGuiOption()
{
	ImGui::Begin("Flies");
	ImGuiID Light = ImGui::GetID("Fly");
	ImGui::BeginChildFrame(Light, ImVec2(100, 200));
	ImGui::Text("Current Fly");

	for (auto iter = m_Fly.begin(); iter != m_Fly.end(); iter++)
	{
		if (ImGui::Button(to_string(iter->first).c_str())) {
			m_iPickedIndex = iter->first;
			m_pPickedFly = iter->second;
		}
		//if (ImGui::IsItemHovered())
		//{
		//	_float3 vScale;
		//	_float3 vRot;
		//	_float3 vPos = _float3(iter->second.x, iter->second.y, iter->second.z);
		//	m_pFly->Move(XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
		//	m_pGameInstance->Use_Gizmo_Offset(&vScale, &vRot, &vPos);
		//}
	}
	ImGui::EndChildFrame();

	ImGui::Checkbox("Render Gizmo", &m_IsRenderGizmo);

	ImGui::InputFloat3("Pos", m_vPickedPos.arr);
	ImGui::SameLine();
	if (ImGui::Button("Get PickPos"))
		m_vPickedPos.float_4 = CLevel_Map::m_vPickedPos;

	ImGui::SameLine();
	if (ImGui::Button("Get CamPos"))
		m_vPickedPos.float_4 = *m_pGameInstance->Get_CamPos();

	_float3 vScale = _float3(1.f, 1.f, 1.f);
	_float3 vRotation = _float3(1.f, 1.f, 1.f);
	if (m_IsRenderGizmo)
	{
		_float3 vPos = _float3(m_vPickedPos.float_4.x, m_vPickedPos.float_4.y, m_vPickedPos.float_4.z);
		m_pPickedFly->Move(XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
		m_pGameInstance->Use_Gizmo_Offset(&vScale, &vRotation, &vPos);
	}
	/*if (ImGui::Button("Save"))
		m_Fly[m_iPickedIndex++] = m_vPickedPos.float_4;*/

	if (ImGui::Button("Delete"))
		m_Fly.erase(m_iPickedIndex++);

	ImGui::End();
}

void CEdit_FireFly_Manager::Create_Fly()
{
	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_FireFly"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance_FireFly.hlsl"), VTXMESHINSTANCE_FIREFLY::Elements, VTXMESHINSTANCE_FIREFLY::iNumElements));

	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_FireFly"),
		CEdit_FireFly::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
	_float Size = 0.00005f;
	_matrix PreTransformMatrix = XMMatrixScaling(Size, Size, Size);
	_string Path = "../../Client/Bin/Resource/Map/Asphodel_Barrens/FireFly/FireFly.dat";
	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Test"),
		CModel_Instance_FireFly::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(), true))))
		CRASH("Prototype Create Failed");

	CEdit_FireFly::MAP_LOAD Desc{};
	Desc.iNumInstance = 10;
	Desc.iSaveIndex = 0;
	Desc.iShaderPassIndex = 0;
	strcpy_s(Desc.ModelName, "Test");
	Desc.IsLoaded = false;
	Desc.vPerSin = _float2(2.f, 5.f);
	Desc.vPerCos = _float2(1.f, 4.f);
	Desc.vPerSin2 = _float2(2.f, 3.f);
	Desc.vRange = _float2(1.f, 3.f);
	_float4x4* pTest = new _float4x4[10];
	for (_uint i = 0; i < 10; ++i)
		XMStoreFloat4x4(&pTest[i], XMMatrixIdentity());
	Desc.InstanceWorldMatrix = pTest;
	m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_FireFly"), m_iLevel, TEXT("Layer_FireFly"), &Desc);
	Safe_Delete_Array(pTest);
}

void CEdit_FireFly_Manager::Map_Load(_float4 Desc)
{
	m_Fly.emplace(m_iPickedIndex++, Desc);
}

CEdit_FireFly_Manager* CEdit_FireFly_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_FireFly_Manager* pInstance = new CEdit_FireFly_Manager(pDevice, pContext);
	pInstance->Initialize();
	return pInstance;
}

void CEdit_FireFly_Manager::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pFly);
	Safe_Release(m_pGameInstance);
	
}
