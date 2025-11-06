#include"ClientPch.h"
#include "Sonoro_Manager.h"
#include"MapObject_Sonoro.h"
#include"MapObject_NonSonoro.h"

CSonoro_Manager::CSonoro_Manager()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

void CSonoro_Manager::Initialize()
{
	m_vUpSpeed = _float4(0.f, 0.4f, 0.f, 0.f);
}

void CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_Sonoro* pObjects)
{
	if (eType == OBJECTTYPE::SONORA)
	{
		m_SonoroObjects.push_back(pObjects);
		Safe_AddRef(pObjects);
	}
}

void CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_NonSonoro* pObjects)
{
	if (eType == OBJECTTYPE::NONSONORA || eType == OBJECTTYPE::NONSONORA_FLOOR)
	{
		m_NonSonoroObjects.push_back(pObjects);
		Safe_AddRef(pObjects);
	}
}

void CSonoro_Manager::Update(_float fTimeDelta)
{
	m_fTriggerdTime += fTimeDelta;

	if (m_fTriggerdTime >= 3.f)
	{

	}
	else
		for (auto& pObject : m_NonSonoroObjects)
			pObject->Turn_Sonoro(XMLoadFloat4(&m_vUpSpeed));
}

void CSonoro_Manager::Change_Sonoro(_bool IsSonoro)
{
	//버튼을 누르고 딜레이시간 이후에 슬금슬금 올라가게.
	if (m_LastSonoroMode != IsSonoro)
	{
		_float4 vCamPos = *m_pGameInstance->Get_CamPos();
		
		for (auto& pObject : m_NonSonoroObjects)
			pObject->Compute_DelayTime(vCamPos);

	}
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
