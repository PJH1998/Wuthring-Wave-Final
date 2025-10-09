#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CPooling_Manager final : public CBase
{
private:
	explicit CPooling_Manager();
	virtual ~CPooling_Manager() = default;

public:
	HRESULT								Initialize();

	// Object Pooling
	HRESULT								Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg);
	HRESULT								Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg);
	HRESULT								Clear_Resource();
	void									Update_Pooling();

	// Thread Pooling
	void									Add_Work(function<void()> Work);
	_bool									IsWorkFinish() { return 0 == m_iLiveWork; }

private:
	class CGameInstance*				m_pGameInstance = { nullptr };
	// Pooling 대기
	map<const _wstring, queue<class CGameObject*>>			m_PoolingObjects;
	// 활성화된 Object
	map<const _wstring, list<class CGameObject*>>				m_ActiveObjects;

	// Thread
	vector<thread>					m_Threads;
	// Hardware에서 제공하는 CPU Core 개수
	_uint								m_iNumThread = {};
	// Thread에 할당할 작업들
	queue<function<void()>>	m_Works;
	// Mutex (Data 참조 시, 순서대로 1개의 Thread만 접근 가능하게 하도록)
	mutex								m_Mutex;
	// Thread Wait 상태 만들기 위한 객체
	condition_variable				m_CV;
	// Thread All Stop
	_bool								m_isAllStop = { false };
	// 진행중인 Work Count
	atomic<_int>					m_iLiveWork = {};

private:
	void									Work_Thread();

public:
	static		CPooling_Manager*	Create();
	virtual		void						Free();
};

NS_END