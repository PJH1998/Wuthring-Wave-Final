#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CAnimTransiiton final : public CBase
{
	typedef vector<function<_bool()>> CONDITION;

private:
	explicit CAnimTransiiton();
	virtual ~CAnimTransiiton() = default;

public:
	HRESULT Initialize_Prototype(ifstream& File);

	_bool Can_Transit();

private:
	_string m_strNextState;
	CONDITION m_Conditions;

public:
	static CAnimTransiiton* Create(ifstream& File);
#ifdef _DEBUG
	static CAnimTransiiton* Create();
#endif // _DEBUG
	virtual void Free() override;
};
NS_END
