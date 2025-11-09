#include "ClientPch.h"
#include "UI_FontPreset.h"
#include "GameInstance.h"
#include "UI_Text_Damage.h"

CUI_FontPreset::CUI_FontPreset()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CUI_FontPreset::Initialize()
{
	// for Damage..

	// Const
	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = {};
	tDesc.strFontTag		= L"WW_Bold";
	tDesc.fScale			= 0.5f;	
	tDesc.vLifeTime			= { 0.0f, 10.0f }; // ksta
	tDesc.fFontOutlineWidth	= 2.f;
	tDesc.iShaderFlag		= ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	tDesc.isTargetExist		= true;
	tDesc.isInstance		= true;
	tDesc.vScreenPos		= _float2{ 0.f, 0.f };
	tDesc.strUIName			= L"DamageFont";
	tDesc.iPassType			= 0;

	// Const per Types..
	// - None (쓰지마셈)
	tDesc.vColor			= { 1.000f, 0.000f, 1.000f, 1.0f };
	tDesc.vOutlineColor		= { 1.000f, 0.000f, 1.000f, 1.0f };
	m_FontTypeDesc.push_back(tDesc);
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

void CUI_FontPreset::Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType, _float fSpawnRange)	// Heal, Dark, Etc..
{
	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = m_FontTypeDesc[iDmgElemType];
	
	

	tDesc.strText			= L"테스트입니다 " + to_wstring(iDamage);
	tDesc.vScreenPos		= { 0.f, 0.f };		// ksta : 반드시. 이게 존재하면 해당 방향으로 드리프트 발생.
	tDesc.vTargetWorldPos	= vTargetPos;

	tDesc.vecInstanceDescs.resize(tDesc.strText.size());
	for (_uint i = 0; i < tDesc.vecInstanceDescs.size(); i++)
		tDesc.vecInstanceDescs[i].matExtraData.m[0][0] = 1.f;


	tDesc.vColor			= m_FontTypeDesc[iDmgElemType].vColor;
	tDesc.vOutlineColor		= m_FontTypeDesc[iDmgElemType].vOutlineColor;

	tDesc.vTargetWorldPos = {
		vTargetPos.x + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.y + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.z + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.w
	};

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Text_Damage", _fmatrix(), &tDesc);
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
