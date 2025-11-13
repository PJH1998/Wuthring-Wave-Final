#include"ClientPch.h"
#include "Sonoro_Manager.h"
#include"MapObject_Sonoro.h"
#include"MapObject_NonSonoro.h"

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

	if (m_fTriggerdTime >= 5.f)
	{
		m_SonoroRender = !m_SonoroRender;
		m_IsUpdate = !m_IsUpdate;
		m_fTriggerdTime = 0.f;

		//코드 변경 가능성 高
		for (auto& pObject : m_NonSonoroObjects)
			pObject->ReturnPos();
	}
	else
		for (auto& pObject : m_NonSonoroObjects)
			pObject->Turn_Sonoro(XMLoadFloat4(&m_vUpSpeed), m_fTriggerdTime);
}

void CSonoro_Manager::Change_Sonoro(_bool IsSonoro)
{
	//처음 실행하면 True가 들어옴.
	
	//버튼을 누르고 딜레이시간 이후에 슬금슬금 올라가게.
	m_SonoroRigidActive = !m_SonoroRigidActive;
	
	//이런 느낌으로 카메라 이벤트 실행.

	//m_pGameSystem->Play_Action(TEXT("Action_Asphodel_Barrens_Start"), m_pTransformCom->Get_WorldMatrix(), false);


	//for (auto& pObject : m_NonSonoroObjects)
	//	pObject-> Change_Collision_Layer(m_SonoroRigidActive);

	for (auto& pObject : m_SonoroObjects)
		pObject->Change_Collision_Layer(m_SonoroRigidActive);

	if (m_SonoroRigidActive)
	{
		_float4 vCamPos = *m_pGameInstance->Get_CamPos();

		for (auto& pObject : m_NonSonoroObjects)
		{
			pObject->Compute_DelayTime(vCamPos);
		}
		m_IsUpdate = !m_IsUpdate;
	}
	else
	{
		for (auto& pObject : m_NonSonoroObjects)
			pObject->Change_Collision_Layer(m_SonoroRigidActive);

		m_SonoroRender = !m_SonoroRender;
	}

	//if (m_LastSonoroMode != IsSonoro)
	//{
	//	_float4 vCamPos = *m_pGameInstance->Get_CamPos();

	//	for (auto& pObject : m_NonSonoroObjects)
	//	{
	//		pObject->Compute_DelayTime(vCamPos);
	//	}
	//	m_IsUpdate = !m_IsUpdate;
	//}
	//else if (m_LastSonoroMode == IsSonoro)
	//{
	//	m_SonoroRender = !m_SonoroRender;
	//}
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

	//for (auto& pObject : m_SonoroObjects)
	//	Safe_Release(pObject);
	//m_SonoroObjects.clear();

	//for (auto& pObject : m_NonSonoroObjects)
	//	Safe_Release(pObject);
	//m_NonSonoroObjects.clear();

	Safe_Release(m_pGameInstance);
}
