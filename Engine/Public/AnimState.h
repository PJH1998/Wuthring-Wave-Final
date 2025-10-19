#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CModel;

class ENGINE_DLL CAnimState : public CBase
{
public:
	typedef struct tagAnimStateDesc
	{
		_float* pTrackPosition;
		_bool isBlend;
		_bool isRootMotion;
		_float fRootMotionRate;
	}ANIMSTATE_DESC;
protected:
	explicit CAnimState() = default;
	virtual ~CAnimState() = default;

public:
	virtual HRESULT Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
	virtual void Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag);
	virtual void Update(_float fTimeDelta, class CAnimMachine* pAnimMachine, _uint* pOwnerState, _string* pCurrentAnimTag/*, ANIMSTATE_DESC& StateData*/);
	virtual void Exit(CModel* pModelCom, _uint* pOwnerState);
	virtual void Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom);
	//virtual void Reset() PURE;

private:
	_string m_strAnimationTag;
	_string m_strAnimStateTag;
	ANIMSTATE_DESC m_StateData{};
	
public:
	static CAnimState* Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
	//virtual CAnimState* Clone() PURE;
	virtual void Free() override;
};
NS_END
