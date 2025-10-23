#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CAnimTransition final : public CBase
{
	typedef vector<function<_bool(const _uint*)>> CONDITION;

private:
	explicit CAnimTransition();
	virtual ~CAnimTransition() = default;

public:
	const _float Get_TransitEnablePos() { return fTransitEnablePos; }

public:
	HRESULT Initialize_Prototype(json& jsonParser);

	//_bool Is_Transit(const _uint* pOwnerState, _int& iAnimStateIndex);
	_bool Is_Transit(const _uint* pOwnerState, _string& strNextState);

private:
	_string m_strNextState;
	//_int m_iNextStateIndex{ -1 };
	_uint m_iTargetState{};
	_float fTransitEnablePos;
	CONDITION m_Conditions;

public:
	static CAnimTransition* Create(json& jsonParser);
#ifdef _DEBUG
	static CAnimTransition* Create();
#endif // _DEBUG
	virtual void Free() override;
};
NS_END
