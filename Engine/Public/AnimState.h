#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CModel;
class CAnimTransition;
class CAnimMachine;

class ENGINE_DLL CAnimState : public CBase
{
public:
	typedef struct tagAnimStateDesc
	{
		//_float* pTrackPosition;
		_bool isBlend;
		_bool isRootMotion;
		_float fRootMotionRate;
		_float fTransitTrackPos;
		_float fAnimationSpeed;
		_uint iConstAnimRunning;
	}ANIMSTATE_DESC;
protected:
	explicit CAnimState() = default;
	virtual ~CAnimState() = default;

public:
#ifdef _DEBUG
	virtual HRESULT Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
#endif
	virtual HRESULT Initialize(json& jsonParser);
	virtual void Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag);
	virtual void Update(CAnimMachine* pAnimMachine, CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag, _float fTrackPosition/*, ANIMSTATE_DESC& StateData*/);
	virtual void Exit(CModel* pModelCom, _uint* pOwnerState);
	//virtual void Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom);
	//virtual void Reset() PURE;

private:
	_string m_strAnimationTag;
	_uint m_iConstAnimRunning{};

	ANIMSTATE_DESC m_StateData{};

	vector<CAnimTransition*> m_Transitions;
	
public:
#ifdef _DEBUG
	static CAnimState* Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
#endif
	static CAnimState* Create(json& jsonParset);
	//virtual CAnimState* Clone() PURE;
	virtual void Free() override;
};
NS_END
