#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"
#include "Factory.h"

#include "UI_FontPreset.h"
#include "UI_ControlHelper.h"
#include "UI_StatusSyncer.h"

IMPLEMENT_SINGLETON(CGameSystem)

CGameSystem::CGameSystem()
{
}

void CGameSystem::Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pParser = CParser::Create(pDevice, pContext);
	ASSERT_CRASH(m_pParser);

	m_pFactory = CFactory::Create(pDevice, pContext);
	ASSERT_CRASH(m_pFactory);


	m_pUI_FontPreset = CUI_FontPreset::Create();
	ASSERT_CRASH(m_pUI_FontPreset);

	m_pUI_ControlHelper = CUI_ControlHelper::Create();
	ASSERT_CRASH(m_pUI_ControlHelper);

	m_pUI_StatusSyncer = CUI_StatusSyncer::Create();
	ASSERT_CRASH(m_pUI_ControlHelper);

}

const vector<vector<_string>>& CGameSystem::Load_CSV(const _char* pFilePath)
{
	return m_pParser->Load_CSV(pFilePath);
}

void CGameSystem::Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel)
{
	return m_pParser->Ready_Prototype_Map(pFilePath, eLevel);
}

void CGameSystem::Clone_MapObjects(LEVEL eLevel, _uint iIndex)
{
	m_pParser->Clone_MapObjects(eLevel, iIndex);
}

void CGameSystem::Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix)
{
	m_pFactory->Create_MonsterDummy(eLayerLevel, vPos, PreTransformationMatrix);
}

void CGameSystem::Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat)
{
	m_Stats = eCharacterStat;
}


void CGameSystem::Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType, _uint iDmgAnimType)
{
	m_pUI_FontPreset->Render_Damage(vTargetPos, iDamage, iDmgElemType, iDmgAnimType);
}

CCustom_UI* CGameSystem::Find_RootUI(_wstring strName)
{
	return m_pUI_ControlHelper->Find_RootUI(strName);
}

CCustom_UI* CGameSystem::Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName)
{
	return m_pUI_ControlHelper->Find_ChildUI(strRootUIName, strChildUIName);
}

HRESULT CGameSystem::HUD_FadeOut()
{
	return m_pUI_ControlHelper->HUD_FadeOut();
}

HRESULT CGameSystem::HUD_FadeIn()
{
	return m_pUI_ControlHelper->HUD_FadeIn();
}

HRESULT	CGameSystem::Sync_Status_toHUD(CHARACTER_STAT& eStat)
{
	return m_pUI_StatusSyncer->Sync_Status_toHUD(eStat);
}

void CGameSystem::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
	Safe_Release(m_pFactory);

	Safe_Release(m_pUI_FontPreset);
	Safe_Release(m_pUI_ControlHelper);
	Safe_Release(m_pUI_StatusSyncer);
}
