#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CAnimNotify abstract: public CBase
{
public:
    explicit CAnimNotify(_float fTrackPosition);
    virtual ~CAnimNotify() = default;
    
    virtual void Execute() = 0;
    virtual json To_Json() const = 0;         // JSON 蹂??
    virtual const _string& Get_NotifyTypeName() const = 0; // ????대쫫

    void Set_ColliderCallBack(function<void(const _wstring&, _bool)> ColliderCallback) { m_ColliderCallback = ColliderCallback;  }
    void Set_EffectCallback(function<void()> EffectCallback) { m_EffectCallback = EffectCallback; }

#ifdef _DEBUG
    virtual void ImGui_Print() = 0;
#endif // _DEBUG

    const _float Get_TrackPosition() const { return m_fTrackPosition; }

protected:
    _float m_fTrackPosition = {}; // 臾댁“嫄댁쟻?쇰줈 ?꾩슂.
    _string m_strNotifyTypeName = {};
    function<void(const _wstring&, _bool)> m_ColliderCallback;
    function<void()> m_EffectCallback;


public:
    virtual void Free() override;
};
NS_END
