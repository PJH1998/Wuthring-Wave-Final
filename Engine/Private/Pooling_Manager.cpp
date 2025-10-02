#include "EnginePch.h"
#include "Pooling_Manager.h"

#include "GameInstance.h"
#include "GameObject.h"

CPooling_Manager::CPooling_Manager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
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

CPooling_Manager* CPooling_Manager::Create()
{
    return new CPooling_Manager();
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

    Safe_Release(m_pGameInstance);
}
