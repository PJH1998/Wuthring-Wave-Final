#pragma once
#include "Base.h"

#include "UI_Text_Damage.h"

NS_BEGIN(Client)

class CUI_FontPreset final : public CBase
{
	// 속성 타입 열거형 정의 등 필요


private:
	explicit CUI_FontPreset();
	virtual ~CUI_FontPreset() = default;

public:
	HRESULT Initialize();

public:
	void Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType, _float fSpawnRange);


private:
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<CUI_Text_Damage::TEXT_UI_TIMED_DESC> m_FontTypeDesc = {};

public:
	static CUI_FontPreset* Create();
	virtual void Free() override;
};

NS_END
