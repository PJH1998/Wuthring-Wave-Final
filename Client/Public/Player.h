#pragma once
#include "Actor.h"
NS_BEGIN(Client)
// 플레이어 캐릭터의 부모 객체.
class CPlayer abstract : public CActor
{
public:
	typedef struct tagPlayerStat
	{
		_float fHp = {};
		_float fEnergyRate = {};
		_float fAttack = {};
	}PLAYER_STAT;


	typedef struct tagPlayerDesc : public CActor::ACTOR_DESC
	{
		class CPlayerParty* pOwner = { nullptr };
		pair<LEVEL, _wstring> stateMachineData = {};
		pair<LEVEL, _wstring> controllerData = {};
		vector<pair<_wstring, _wstring>> PartPrototypes;
		_float3 vScale = { 1.f, 1.f, 1.f};
		_float3 vRotation = { 0.f, 0.f, 0.f };
		_float3 vPostion = { 0.f, 0.f, 0.f };
		PLAYER_STAT eStat = {};

	}PLAYER_DESC;
	

#pragma region 기본 함수
protected:
	explicit CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;


#pragma endregion

#pragma region STATE에서 사용
public:
	_bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true);
	_bool Check_AnyInput(KEYINPUT  eKeyInput);
	_bool Check_AllInput(KEYINPUT eKeyInput);
	
#pragma endregion




protected:
	class CPlayerParty* m_pOwner = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	
	

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

