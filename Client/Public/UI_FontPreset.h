#pragma once
#include "Base.h"

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
	void Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType, _uint iDmgAnimType);


private:
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<FONT_SINGLEDESC> m_FontTypeDesc = {};

public:
	static CUI_FontPreset* Create();
	virtual void Free() override;
};

NS_END
