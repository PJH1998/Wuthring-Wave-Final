#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CAugustaGroundLandSlide final : public CGroundState
{
private:
    enum LANDSTATE
    {
        MOVE = 0,
		HIT,
		EXIT,
		LAND,
        END
    };

private:
    explicit CAugustaGroundLandSlide() = default;
    virtual ~CAugustaGroundLandSlide() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[LANDSTATE::END] = {};

	_float m_fSoundTimer = {};
	_float m_fMaxTime = {};
	_wstring m_strSoundTag = {};

	SLIDE_DATA m_SlideData = {};
	_uint m_iWayPoint = {};

	_float3 m_vStart = {};

private:
    virtual void Handle_Input() override;
	void Process_Timer(_float fTimeDelta);
    void Update_LandAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CAugustaGroundLandSlide* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
