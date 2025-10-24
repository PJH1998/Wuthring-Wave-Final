#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CAnimTransition final : public CBase
{
public:
	typedef struct tagTransitionData
	{
		_string strFrom;
		_string strTo;
		//조건에 사용할 const flag변수
		_uint iTargetState;
		_float fTargetTrackPos;
		//_uint iNextStateIndex; strTo에서 직접 찾기

	}TRANSITION_DESC;

private:
	explicit CAnimTransition();
	virtual ~CAnimTransition() = default;

public:
	const _float Get_TransitEnablePos() { return fTargetTrackPos; }

public:
	HRESULT Initialize_Prototype(json& jsonParser);

	//_bool Is_Transit(const _uint* pOwnerState, _int& iAnimStateIndex);
	_bool Is_Transit(const _uint* pOwnerState, _string& strNextState);

#ifdef _DEBUG
	void Get_TransitData(_string& strNextState, _uint& iTargetState, _float& fTransitEnablePos)
	{
		strNextState = m_strNextState;
		iTargetState = m_iTargetState;
		fTransitEnablePos = fTransitEnablePos;
	}
#endif // DEBUG

private:
	_string m_strNextState;
	//_int m_iNextStateIndex{ -1 };

	//조건에 사용할 const flag변수
	_uint m_iTargetState{};
	_float fTargetTrackPos{};

	typedef vector<function<_bool(const _uint*)>> CONDITION;
	CONDITION m_Conditions;

public:
	static CAnimTransition* Create(json& jsonParser);
#ifdef _DEBUG
	static CAnimTransition* Create();
#endif // _DEBUG
	virtual void Free() override;
};
NS_END
