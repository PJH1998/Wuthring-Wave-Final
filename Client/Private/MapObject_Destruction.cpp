#include"ClientPch.h"
#include "MapObject_Destruction.h"
#include"GameSystem.h"
#include"MapObject_Destruction_Debris.h"

CMapObject_Destruction::CMapObject_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice, pContext)
{
}

CMapObject_Destruction::CMapObject_Destruction(const CMapObject_Destruction& Prototype)
	:CStaticObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Destruction::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapObject_Destruction::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	m_iNumLOD = m_pModelComArray.size() - 1;

	//m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	//if (!m_pBoundingBox)
	//	CRASH("Failed");
	//m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);
	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_vImpulsePos = pDesc->m_vImpulsePos;
	m_vImpulsePower = pDesc->m_vImpulsePower;


	m_iTriggerIndex = pDesc->iTriggerIndex;

	m_pGameSystem->TriggerRegister(m_iTriggerIndex, [this](void* pArg) {
		if (m_IsDestroy)
			Spawn_Particles();
		});
	m_IsDestroy = false;
	return S_OK;
}

void CMapObject_Destruction::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
		m_IsDestroy = false;
}

void CMapObject_Destruction::Update(_float fTimeDelta)
{
}

void CMapObject_Destruction::Late_Update(_float fTimeDelta)
{
	if (!m_IsDestroy)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject_Destruction::Render()
{
	if (m_IsDestroy)
		return;

	_uint DrawModel = m_iLODIndex;
	//_uint DrawModel = 0;

	if (DrawModel > m_iNumLOD)
		DrawModel = m_iNumLOD;

	Bind_Resources();

	for (_uint i = 0; i < m_pModelComArray[DrawModel]->Get_NumMesh(); ++i)
	{

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

		// TODO
		// Clear 함수 변경 필요
		m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
	}
}


void CMapObject_Destruction::Render_Shadow()
{
}

HRESULT CMapObject_Destruction::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	
	MultiByteToWideChar(CP_ACP, 0, pDesc->ModelName, -1, Name, strlen(pDesc->ModelName));
	lstrcat(Model, Name);
	//_uint V = pDesc->ModelName[strlen(pDesc->ModelName) - 1] - '0' + 1;
	_uint V = 1;

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
		StringToWString(pDesc->ModelName) + TEXT("_Bone"), reinterpret_cast<CComponent**>(&m_pBoneModel), nullptr)))
		CRASH("FAILED");

	m_pBoneModel->Update_BoneMatrix_Map();

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;

	_string ModelName = pDesc->ModelName;
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
		CMapObject_Destruction_Debris::MAP_LOAD Desc;
		Desc.iLevel = pDesc->iLevel;
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

		_vector Pos = XMVectorSetW(XMLoadFloat3(&m_vImpulsePos), 1.f);
		_vector Power = XMVectorSetW(XMLoadFloat3(&m_vImpulsePower), 0.f);

		_vector vDeltaPos = XMVectorSetW(XMLoadFloat3(reinterpret_cast<_float3*>(&Mat.m[3])) - Pos, 0.f);

		XMStoreFloat3(&Desc.vImpulse, vDeltaPos * Power);
		if (FAILED(m_pGameInstance->Add_PoolingObject(pDesc->iLevel, TEXT("Prototype_GameObject_MapObject_Destruction_Debris")
			, pDesc->iLevel, TEXT("Layer_Destruction_Debris"), TEXT("24BS_Debris") + to_wstring(i), 3, &Desc)))
			return S_OK;
	}
	return S_OK;
}

void CMapObject_Destruction::Spawn_Particles()
{
	m_IsDestroy = true;
	for(_uint i=2; i<m_pBoneModel->Get_BoneSize();++i)
	{
		CMapObject_Destruction_Debris::RESET_DESC ResetDesc{};

		_vector vScale, vRot, vTrans;
		_float4x4 BoneMat = *m_pBoneModel->Get_BoneMatrixPtr(i);
		XMMatrixDecompose(&vScale, &vRot, &vTrans, XMLoadFloat4x4(&BoneMat));
		vRot = XMQuaternionNormalize(vRot);
		vTrans += XMVectorSet(m_pGameInstance->Rand(0.f, 3.f), m_pGameInstance->Rand(0.f, 3.f), m_pGameInstance->Rand(0.f, 3.f), 0.f);
		_float4x4 Mat;
		XMStoreFloat4x4(&Mat,
			XMMatrixRotationQuaternion(vRot) *
			XMMatrixTranslationFromVector(vTrans) *
			m_pTransformCom->Get_WorldMatrix());



		_vector Pos = XMVectorSetW(XMLoadFloat3(&m_vImpulsePos), 1.f);
		_vector Power = XMVectorSetW(XMLoadFloat3(&m_vImpulsePower), 0.f);

		_vector vDeltaPos = XMVectorSetW(XMLoadFloat3(reinterpret_cast<_float3*>(&Mat.m[3])) - Pos, 0.f);

		
		XMStoreFloat3(&ResetDesc.vImpulse, vDeltaPos * Power);
		
		m_pGameInstance->Spawn_PoolingObject(TEXT("24BS_Debris") + to_wstring(i), XMLoadFloat4x4(&Mat), &ResetDesc);
	}
}

void CMapObject_Destruction::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

CMapObject_Destruction* CMapObject_Destruction::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Destruction* pInstance = new CMapObject_Destruction(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Destruction");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Destruction::Clone(void* pArg)
{
	CMapObject_Destruction* pClone = new CMapObject_Destruction(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Destruction (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Destruction::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pBoneModel);
	Safe_Release(m_pGameSystem);
	Safe_Delete(m_pBoundingBox);

	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);
	m_pModelComArray.clear();


}
