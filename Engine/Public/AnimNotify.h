#pragma once
#include "Base.h"

class CAnimNotify abstract : public CBase
{
public:
    explicit CAnimNotify(_float fTrackPosition);
    virtual ~CAnimNotify() = default;
    
    virtual void Execute() = 0;
    virtual const _string& Get_NotifyTypeName() const = 0;  // 타입 이름
    virtual json To_Json() const = 0;         // JSON 변환

    const _float Get_TrackPosition() const { return m_fTrackPosition; }

protected:
    _float m_fTrackPosition; // 무조건적으로 필요.
    class CGameInstance* m_pGameInstance = { nullptr };

public:
    virtual void Free() override;
};

