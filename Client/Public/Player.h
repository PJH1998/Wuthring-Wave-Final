#pragma once
#include "Player_Define.h"
#include "GameObject.h"

NS_BEGIN(Client)
// Player Container
class CPlayer final : public CGameObject
{
public:
	enum CHARACTERTYPE
	{
		NONE = -1,
		AUGUSTA = 0,
		GALBRENA = 1,
		PLAYER = 2,
		TYPE_END
	};

	enum SWITCH_STATE
	{
		SWITCH_NONE,     // 전환 대기 없음
		SWITCH_PENDING,  // 전환 준비 중
		SWITCH_READY     // 전환 준비 완료
	};

public:
	typedef struct tagPlayerPartyDesc
	{
		LEVEL eCurLevel = {LEVEL::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};

		_uint iPlayerCount = {};
		_wstring wStrInputControllerTag = {};
		vector<PLAYER_SPEC> PlayerSpecs = {};
	}PLAYER_DESC;

#pragma region 기본 함수들.
public:
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
	virtual	void	Render_Shadow() override;

#pragma endregion

public:
	void Change_CharacterCheck();
	void Change_Character(CHARACTERTYPE eNextCharacter);
	void Sync_Transform();
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	void Ensemble_Skill(CHARACTERTYPE eCharacter);
	// State에서 호출: Ensemble Skill이 끝났음을 알림
	void Notify_EnsembleEnd();

	void Perform_CharacterSwitch(CHARACTERTYPE eNextCharacter);
	void On_EnsembleEnd(CHARACTERTYPE eCharacter);

public:
	void OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	vector<class CCharacter*> m_Characters; 
	class CInputController* m_pInputControllerCom = { nullptr };
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };
	
	LEVEL m_eCurLevel = { LEVEL::END };
	_int m_iCurrentCharacterIdx = { CHARACTERTYPE::NONE };
	_int m_iPrevCharacterIdx = { CHARACTERTYPE::NONE };
	_int m_iEnsembleCharacterIdx = { CHARACTERTYPE::NONE };


private:
	class CGameSystem* m_pGameSystem = { nullptr };
	// LockOn
	vector<class CTransform*> m_TargetTransforms;
	class CTransform* m_pTargetTransform = { nullptr };
	_bool m_IsLockOn = { false };

private:
	void Sorting_Target();
	void Toggle_LockOn();

private:
	HRESULT Ready_Players(const PLAYER_DESC* pDesc);
	HRESULT Ready_Components(const PLAYER_DESC* pDesc);
	

public:
	static		CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;


};
NS_END

