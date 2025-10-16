#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBlackBoard final : public CBase
{
public:
	enum DATA_TYPE
	{
		INT,
		FLOAT,
		STRING,
		BOOL,
		VECTOR3,
		VECTOR4,
		DATA_END
	};
	typedef map<_string, pair<DATA_TYPE, void*>> BLACKBOARD_DATA;

private:
	explicit CBlackBoard();
	virtual ~CBlackBoard() = default;

public:
	HRESULT Add_Data(const _string& strDataTag, DATA_TYPE eType, void* pValue);
	void* Get_Data(const _string& strDataTag);
	HRESULT Add_Checker(const _string& strDataTag, function<_int()> Checker);
	_int Get_Checker(const _string& strFuncTag);

#ifdef _DEBUG
	// 블랙보드에 바인딩 된 데이터 시각화
	void Bind_Data_to_GUI();
#endif // _DEBUG


private:
	BLACKBOARD_DATA			m_Datas;
	map<const _string, function<_int()>> m_Checkers;

private:
	_bool Find_Data(const _string& strDataTag);

public:
	static CBlackBoard* Create();
	virtual void Free() override;
};

NS_END
