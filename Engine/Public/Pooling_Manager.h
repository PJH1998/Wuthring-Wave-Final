#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CPooling_Manager final : public CBase
{
private:
	explicit CPooling_Manager();
	virtual ~CPooling_Manager() = default;

public:
	HRESULT								Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg);
	HRESULT								Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg);
	HRESULT								Clear_Resource();

	void									Update_Pooling();

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	map<const _wstring, queue<class CGameObject*>>			m_PoolingObjects;
	map<const _wstring, list<class CGameObject*>>				m_ActiveObjects;


public:
	static		CPooling_Manager*	Create();
	virtual		void						Free();
};

NS_END