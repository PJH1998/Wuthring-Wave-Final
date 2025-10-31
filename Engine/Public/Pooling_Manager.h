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
	_bool									IsWorkFinish() { 
		this_thread::sleep_for(chrono::milliseconds(1));
		_int iLiveWork = m_iLiveWork.load(memory_order_acquire);
		_int iRemainWork = m_iRemainWork.load(memory_order_acquire);
		return 0 == iLiveWork && 0 == iRemainWork; }

	void									Wait_Thread_End();

private:
	class CGameInstance*				m_pGameInstance = { nullptr };
	// Pooling ?湲?
	map<const _wstring, queue<class CGameObject*>>			m_PoolingObjects;
	// ?쒖꽦?붾맂 Object
	map<const _wstring, list<class CGameObject*>>				m_ActiveObjects;

	// Thread
	vector<thread>					m_Threads;
	// Hardware Supported CPU Core
	_uint								m_iNumThread = {};
	// Thread???좊떦???묒뾽??
	queue<function<void()>>	m_Works;
	// Mutex (Data)
	mutex								m_Mutex;
	// Thread Wait ?곹깭 留뚮뱾湲??꾪븳 媛앹껜
	condition_variable				m_CV;
	// Thread All Stop
	_bool								m_isAllStop = { false };
	// 진행중인 Work Count
	atomic<_int>					m_iLiveWork = {};
	// 남아있는 Work Count
	atomic<_int>					m_iRemainWork = {};

private:
	void									Work_Thread();

public:
	static		CPooling_Manager*	Create();
	virtual		void						Free();
};

NS_END