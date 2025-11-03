#include "ClientPch.h"
#include "UI_FontPreset.h"
#include "GameInstance.h"

CUI_FontPreset::CUI_FontPreset()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CUI_FontPreset::Initialize()
{
	// for Damage..

	// Const
	FONT_SINGLEDESC tDesc = {};
	tDesc.strFontTag		= L"WW_Bold";
	tDesc.fScale			= 50.f;	
	tDesc.vLifeTime			= { 0.0f, 500.0f }; // ksta
	tDesc.fFontOutlineWidth	= 3.f;
	tDesc.iShaderFlag		= ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_FIXED);

	// Const per Types..
	// - Heal (회복)
	tDesc.vColor			= { 0.427f, 0.898f, 0.412f, 1.0f };
	tDesc.vOutlineColor		= { 0.141f, 0.455f, 0.302f, 1.0f };
	m_FontTypeDesc.push_back(tDesc);
	// - Dark (인멸)
	tDesc.vColor			= { 0.831f, 0.310f, 0.682f, 1.0f };
	tDesc.vOutlineColor		= { 0.607f, 0.267f, 0.533f, 1.0f };
	m_FontTypeDesc.push_back(tDesc);
	// - Electro (전도)
	tDesc.vColor			= { 0.749f, 0.592f, 0.949f, 1.0f };
	tDesc.vOutlineColor		= { 0.667f, 0.498f, 0.776f, 1.0f };
	m_FontTypeDesc.push_back(tDesc);
	// - Fusion (용융)
	tDesc.vColor			= { 0.984f, 0.592f, 0.443f, 1.0f };
	tDesc.vOutlineColor		= { 0.620f, 0.306f, 0.212f, 1.0f };
	m_FontTypeDesc.push_back(tDesc);

	// Variables
	//tDesc.strText			= {};
	//tDesc.vScreenPos		= {};
	//tDesc.vTargetWorldPos	= {};
	//tDesc.vFontGradColor	= ;



	return S_OK;
}

void CUI_FontPreset::Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType, _uint iDmgAnimType)	// Heal, Dark, Etc..
{
	FONT_SINGLEDESC tDesc = m_FontTypeDesc[iDmgElemType];
	
	tDesc.strText			= to_wstring(iDamage);
	tDesc.vScreenPos		= { 500.f, 500.f };
	tDesc.vTargetWorldPos	= vTargetPos;
	//tDesc.vFontGradColor	= {};

	m_pGameInstance->Add_FloatingText(tDesc);
}

CUI_FontPreset* CUI_FontPreset::Create()
{
	CUI_FontPreset* pInstance = new CUI_FontPreset();

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CUI_FontPreset::Free()
{
	Safe_Release(m_pGameInstance);
	__super::Free();
}
