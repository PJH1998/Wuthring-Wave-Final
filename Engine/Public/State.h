#pragma once
#include "Base.h"

NS_BEGIN(Engine)

using InputCondition = function<bool()>;

class ENGINE_DLL CState abstract : public CBase
{
public:
    // 초기에 등록해야 하는 정보들.
    typedef struct tagStateData
    {
        class CGameObject* pOwner = { nullptr };
        _string strAnimName = {};
        _float fSpeed = {};
        _float fExitTrackPosition = {};
        _float fRootMotionRate = { 0.1f };
        _bool IsRootMotion = { true };
    }STATE_DATA;

protected:
    explicit CState() = default;
    virtual ~CState() = default;

public:
    virtual HRESULT Initialize(const STATE_DATA& StateData);
    virtual void OnEnter(); // 들어갔을 시.
    virtual void OnUpdate(_float fTimeDelta); // Update
    virtual void OnExit(); // Exit 시
    virtual void Change_State(class CStateMachine* pStateMachine, const _string& strStateName);
    

public:
    const string& Check_Transition(class CStateMachine* pStateMachine);

protected:
    void Add_Transition(const _string& strStateName, InputCondition condition);

protected:
    STATE_DATA m_StateData = {};
    _float m_fTrackPosition = {};
    _bool m_IsAnimationEnd = { false };
    vector<pair<_string, InputCondition>> m_InputTransitions;
   

public:
    virtual void Free() override;
};
NS_END

