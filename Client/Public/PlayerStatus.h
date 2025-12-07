#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Client)
class CPlayerStatus final : public CBase
{
public:
	explicit CPlayerStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPlayerStatus() = default;

	HRESULT Initialize(const vector<_string>& AbilityFolders);  // 캐릭별 CSV 폴더 배열
	void Update(_float fTimeDelta);  // 모든 Ability 업데이트 (쿨타임/자원)

public:
	void Set_CurrentCharIndex(_uint iIndex) { m_iCurrentCharIndex = iIndex; }


	// 읽기 전용 Getter (포인터 노출 X)
	_float Get_HpRatio(_uint iCharIndex) const;
	_float Get_CostRatio(_uint iCharIndex, COST_TYPE eType) const;
	_float Get_Cost(_uint iCharIndex, COST_TYPE eType) const;
	_float Get_RemainingCooldown(_uint iCharIndex, const _string& strSkillName) const;
	_float Get_MaxCooldown(_uint iCharIndex, const _string& strSkillName) const;
	_uint Get_CurrentCharIndex() const { return m_iCurrentCharIndex; }
	UI_TAB_UTILITY Get_UtilityType() const { return m_eUtilityType; }

	_bool Is_QTE() const { return m_IsQTE; }
	void Bind_QTE(_bool IsQTE) { m_IsQTE = IsQTE; }

	// Player/Character가 참조할 Getter (내부용)
	class CAbility* Get_Ability(_uint iCharIndex) const;

	void Bind_UtilityType(UI_TAB_UTILITY eUtility) { m_eUtilityType = eUtility; }



private:
	void Register_AbilityFiles(const _string& strFolderPath);

public:
	static CPlayerStatus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const vector<_string>& AbilityFolders);
	virtual void Free() override;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	class CGameInstance* m_pGameInstance = { nullptr };

	vector<class CAbility*> m_Abilities;  // 캐릭별 Ability (소유)
	_uint m_iCurrentCharIndex = {};     // 기본 Augusta (스위칭 동기)
	UI_TAB_UTILITY m_eUtilityType = { UI_TAB_UTILITY::NOTHING };
	_bool m_IsQTE = { false };
};
NS_END

