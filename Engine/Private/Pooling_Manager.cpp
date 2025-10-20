#include "EnginePch.h"
#include "Pooling_Manager.h"

#include "GameInstance.h"
#include "GameObject.h"

CPooling_Manager::CPooling_Manager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CPooling_Manager::Initialize()
{
	m_iNumThread = thread::hardware_concurrency();
	m_Threads.reserve(m_iNumThread);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	for (_uint i = 0; i < m_iNumThread; ++i)
		m_Threads.emplace_back([this]() { this->Work_Thread(); });

	return S_OK;
}

HRESULT CPooling_Manager::Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg)
{
    auto iter = m_PoolingObjects.find(strPoolingTag);
    if (iter != m_PoolingObjects.end())
        return E_FAIL;

    for (_uint i = 0; i < iNumObjects; ++i)
    {
        CGameObject* pObject = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::GAMEOBJECT, pArg));
        if (nullptr == pObject)
            return E_FAIL;
        m_PoolingObjects[strPoolingTag].push(pObject);
        m_pGameInstance->Add_GameObject_ToLayer(iLayerLevelID, strLayerTag, pObject);
        Safe_AddRef(pObject);
    }

    return S_OK;
}

HRESULT CPooling_Manager::Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg)
{
    auto iter = m_PoolingObjects.find(strPoolingTag);
    if (iter == m_PoolingObjects.end())
        return E_FAIL;

	if (0 == m_PoolingObjects[strPoolingTag].size())
		return S_OK;

    CGameObject* pObject = m_PoolingObjects[strPoolingTag].front();
	m_PoolingObjects[strPoolingTag].pop();
    pObject->Reset(WorldMatrix, pArg);
	m_ActiveObjects[strPoolingTag].push_back(pObject);

    return S_OK;
}

HRESULT CPooling_Manager::Clear_Resource()
{
    for (auto& Pair : m_PoolingObjects)
    {
        while (false == Pair.second.empty())
        {
            Safe_Release(Pair.second.front());
            Pair.second.pop();
        }
    }
    m_PoolingObjects.clear();

    return S_OK;
}

void CPooling_Manager::Update_Pooling()
{
	for (auto& Pair : m_ActiveObjects)
	{
		for (auto iter = Pair.second.begin(); iter != Pair.second.end();)
		{
			if (false == (*iter)->IsActivate())
			{
				m_PoolingObjects[Pair.first].push(*iter);
				iter = Pair.second.erase(iter);
			}
			else
				++iter;
		}
	}
}

void CPooling_Manager::Add_Work(function<void()> Work)
{
	{
		lock_guard<mutex> lock(m_Mutex);
		m_Works.push(Work);
	}
	m_CV.notify_one();
}

void CPooling_Manager::Wait_Thread_End()
{
	while (false == IsWorkFinish())
	{

	}
}

void CPooling_Manager::Work_Thread()
{
	while (true)
	{
		unique_lock<mutex> lock(m_Mutex);
		m_CV.wait(lock, [this]() { return 0 < m_Works.size() || true == m_isAllStop; });

		// Client 醫낅즺 ?? Thread 紐⑤몢 醫낅즺
		if (true == m_isAllStop)
			return;

		function<void()> Work = move(m_Works.front());
		m_Works.pop();
		lock.unlock();

		m_iLiveWork.fetch_add(1);
		Work();
		m_iLiveWork.fetch_sub(1);

	}
}

CPooling_Manager* CPooling_Manager::Create()
{
	CPooling_Manager* pInstance = new CPooling_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Pooling_Manager");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CPooling_Manager::Free()
{
    __super::Free();

    for (auto& Pair : m_PoolingObjects)
    {
        while (false == Pair.second.empty())
        {
            Safe_Release(Pair.second.front());
            Pair.second.pop();
        }
    }
    m_PoolingObjects.clear();

	for (auto& Pair : m_ActiveObjects)
	{
		for (auto& pObject : Pair.second)
			Safe_Release(pObject);
		Pair.second.clear();
	}
	m_ActiveObjects.clear();

	m_isAllStop = true;
	m_CV.notify_all();
	for (auto& Thread : m_Threads)
		Thread.join();

    Safe_Release(m_pGameInstance);
}
