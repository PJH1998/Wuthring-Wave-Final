#include"ClientPch.h"
#include "Sonoro_Manager.h"
#include"MapObject_Sonoro.h"
#include"MapObject_NonSonoro.h"
#include "SonoraChange.h"

CSonoro_Manager::CSonoro_Manager()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CSonoro_Manager::Initialize()
{
	m_vUpSpeed = _float4(0.f, 0.6f, 0.f, 0.f);
	return S_OK;
}

_bool* CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_Sonoro* pObjects, _bool** SonoroMode)
{
	if (eType == OBJECTTYPE::SONORA)
	{
		{
			lock_guard<mutex> lock(m_Mutex);
			m_SonoroObjects.push_back(pObjects);
			*SonoroMode = &m_SonoroRigidActive;
			Safe_AddRef(pObjects);
		}
		return &m_SonoroRender;
	}

	CRASH("Failed");
	return nullptr;
}

_bool* CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_NonSonoro* pObjects, _bool** SonoroMode)
{
	if (eType == OBJECTTYPE::NONSONORA || eType == OBJECTTYPE::NONSONORA_FLOOR)
	{
		{
			lock_guard<mutex> lock(m_Mutex);
			m_NonSonoroObjects.push_back(pObjects);
			*SonoroMode = &m_SonoroRigidActive;
			Safe_AddRef(pObjects);
		}

		return &m_SonoroRender;
	}

	CRASH("Failed");
	return nullptr;
}

void CSonoro_Manager::Update(_float fTimeDelta)
{
	if (!m_IsUpdate)
		return;

	m_fTriggerdTime += fTimeDelta;

	if (m_SonoroRigidActive)
	{
		if (m_fTriggerdTime >= 5.f)
		{
			m_SonoroRender = !m_SonoroRender;
			m_IsUpdate = !m_IsUpdate;
			m_fTriggerdTime = 0.f;

			for (auto& pObject : m_SonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);

			//코드 변경 가능성 高

			for (auto& pObject : m_NonSonoroObjects)
				pObject->ReturnPos();
		}
		else
			for (auto& pObject : m_NonSonoroObjects)
				pObject->Turn_Sonoro(XMLoadFloat4(&m_vUpSpeed), m_fTriggerdTime);
	}
	else
	{

		if (m_fTriggerdTime >= 4.f)
		{
			for (auto& pObject : m_NonSonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);

			for (auto& pObject : m_SonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);
			m_SonoroRender = !m_SonoroRender;
			m_IsUpdate = !m_IsUpdate;
			m_fTriggerdTime = 0.f;
		}
	}
}

_bool CSonoro_Manager::Change_Sonoro(_bool IsSonoro)
{
	//처음 실행하면 True가 들어옴.
	if (m_IsUpdate)
		return false;

	//버튼을 누르고 딜레이시간 이후에 슬금슬금 올라가게.
	m_SonoroRigidActive = !m_SonoroRigidActive;
	
	CSonoraChange::SONORA_CHANGE_DESC Desc = {};
	Desc.fEffectTime = m_SonoroRigidActive == true ? 5.f : 4.f;
	Desc.fRadialTime = m_SonoroRigidActive == true ? 2.f : 1.f;
	Desc.fFadeTime = m_SonoroRigidActive == true ? 2.f : 1.f;

	m_pGameInstance->Spawn_PoolingObject(TEXT("Pooling_SFX_SonoraChange"), XMMatrixIdentity(), &Desc);

	if (m_SonoroRigidActive)
	{
		_float4 vCamPos = *m_pGameInstance->Get_CamPos();

		for (auto& pObject : m_NonSonoroObjects)
		{
			pObject->Compute_DelayTime(vCamPos);
		}
	}
	//else
	//{	
	//	for (auto& pObject : m_NonSonoroObjects)
	//		pObject->Change_Collision_Layer(m_SonoroRigidActive);

	//	m_SonoroRender = !m_SonoroRender;
	//}

	m_IsUpdate = !m_IsUpdate;
	return true;
}

const _tchar* CSonoro_Manager::Get_SonoroText()
{
	return m_SonoroRender ?
		TEXT("소노라 떠나기") :
		TEXT("소노라 진입하기");
}


CSonoro_Manager* CSonoro_Manager::Create()
{
	CSonoro_Manager* pInstance = new CSonoro_Manager();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Sonoro_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSonoro_Manager::Free()
{
	for (auto& pObject : m_SonoroObjects)
		Safe_Release(pObject);
	m_SonoroObjects.clear();

	for (auto& pObject : m_NonSonoroObjects)
		Safe_Release(pObject);
	m_NonSonoroObjects.clear();

	Safe_Release(m_pGameInstance);
}
