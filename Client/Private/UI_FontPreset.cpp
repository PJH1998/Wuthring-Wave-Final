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
	tDesc.vLifeTime			= { 0.0f, /*10.0f*/ 1.f };			// lifetime. 다른 시간관련 변수(사라지기까지 걸리는 시간 등)는 CUI_Text_Damage::Update_Instances 에서 상수로 제어 가능.
	tDesc.fFontOutlineWidth	= 2.f;
	tDesc.iShaderFlag		= ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	tDesc.isTargetExist		= true;
	tDesc.isInstance		= true;
	tDesc.vScreenPos		= _float2{ 0.f, 0.f };
	tDesc.strUIName			= L"DamageFont";
	tDesc.iPassType			= 0;

	m_FontTypeDesc.resize(ENUM_CLASS(TEXT_COLOR_TYPE::END));


	// Const per Types..
	// - None (쓰지마셈)
	tDesc.vColor			= { 1.000f, 0.000f, 1.000f, 1.0f };
	tDesc.vOutlineColor		= { 1.000f, 0.000f, 1.000f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::NONE)] = tDesc;

	// - Heal (회복)
	tDesc.vColor			= { 0.427f, 0.898f, 0.412f, 1.0f };
	tDesc.vOutlineColor		= { 0.141f, 0.455f, 0.302f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::HEAL)] = tDesc;
	// - Dark (인멸)
	tDesc.vColor			= { 0.831f, 0.310f, 0.682f, 1.0f };
	tDesc.vOutlineColor		= { 0.607f, 0.267f, 0.533f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::DARK)] = tDesc;
	// - Electro (전도)
	tDesc.vColor			= { 0.749f, 0.592f, 0.949f, 1.0f };
	tDesc.vOutlineColor		= { 0.667f, 0.498f, 0.776f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::ELEC)] = tDesc;
	// - Fusion (용융)
	tDesc.vColor			= { 0.984f, 0.592f, 0.443f, 1.0f };
	tDesc.vOutlineColor		= { 0.620f, 0.306f, 0.212f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::FUSI)] = tDesc;

	// - Title (제목용 색상)
	tDesc.vColor			= { 0.631f, 0.607f, 0.424f, 1.0f };
	tDesc.vOutlineColor		= { 0.035f, 0.027f, 0.016f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_TITLE)] = tDesc;
	// - Text (설명용 색상)
	tDesc.vColor			= { 0.718f, 0.729f, 0.757f, 1.0f };
	tDesc.vOutlineColor		= { 0.035f, 0.027f, 0.016f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_NORMAL)] = tDesc;
	// - Progress (로딩 창 진행률 숫자용 색상)
	tDesc.vColor			= { 0.961f, 0.957f, 0.937f, 1.0f };
	tDesc.vOutlineColor		= { 0.035f, 0.027f, 0.016f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_PROGRESS)] = tDesc;

	// - BossName (보스 이름용 색상)
	tDesc.vColor			= { 0.827f, 0.364f, 0.435f, 1.0f };
	tDesc.vOutlineColor		= { 0.203f, 0.188f, 0.192f, 1.0f };
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_BOSSNAME)] = tDesc;
	// - PlayerHP (플레이어 체력 표시용 색상)
	tDesc.vColor			= { 0.600f, 0.600f, 0.600f, 1.0f };
	tDesc.vOutlineColor		= { 0.900f, 0.900f, 0.900f, 0.5f };
	tDesc.fFontOutlineWidth = 4.f;
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_PLAYERHP)] = tDesc;
	// - TabUtil (탭 유틸리티 창 글자)
	tDesc.vColor			= { 0.950f, 0.950f, 0.950f, 1.0f };
	tDesc.vOutlineColor		= { 0.900f, 0.900f, 0.900f, 0.5f };
	tDesc.fFontOutlineWidth = 0.f;
	tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_TABUTIL)] = tDesc;
	// - SkillColldown (쿨타임용 글자)
	tDesc.vColor			= { 0.950f, 0.950f, 0.950f, 1.0f };
	tDesc.vOutlineColor		= { 0.100f, 0.100f, 0.100f, 0.8f };
	tDesc.fFontOutlineWidth = 6.f;
	tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	m_FontTypeDesc[ENUM_CLASS(TEXT_COLOR_TYPE::TT_SKILLCD)] = tDesc;





	// 필요한 색상이 있다면 ENUM 추가 및 여기에 프리셋 추가 후 사용하면 됩니다.



	// Variables
	//tDesc.strText			= {};
	//tDesc.vScreenPos		= {};
	//tDesc.vTargetWorldPos	= {};
	//tDesc.vFontGradColor	= ;



	return S_OK;
}

void CUI_FontPreset::Render_Damage(_float4 vTargetPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fSpawnRange)	// Heal, Dark, Etc..
{
	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = m_FontTypeDesc[ENUM_CLASS(eColorType)];
	
	

	tDesc.strText			= L"" + strText;
	tDesc.vScreenPos		= { 0.f, 0.f };		// ksta : 반드시. 이게 존재하면 해당 방향으로 드리프트 발생.
	tDesc.vTargetWorldPos	= vTargetPos;

	tDesc.vecInstanceDescs.resize(tDesc.strText.size());
	for (_uint i = 0; i < tDesc.vecInstanceDescs.size(); i++)
		tDesc.vecInstanceDescs[i].matExtraData.m[0][0] = 1.f;


	//tDesc.vColor			= m_FontTypeDesc[ENUM_CLASS(eDmgElemType)].vColor;
	//tDesc.vOutlineColor		= m_FontTypeDesc[ENUM_CLASS(eDmgElemType)].vOutlineColor;

	tDesc.vTargetWorldPos = {
		vTargetPos.x + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.y + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.z + m_pGameInstance->Rand(-fSpawnRange, +fSpawnRange),
		vTargetPos.w
	};

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Text_Damage", _fmatrix(), &tDesc);
}


CUI_Text* CUI_FontPreset::Create_FontToScreen(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName, _wstring strFontTag)
{
	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = m_FontTypeDesc[ENUM_CLASS(eColorType)];
	
	tDesc.strFontTag = strFontTag;
	tDesc.fScale = fFontScale;
	//tDesc.vLifeTime = { 0.0f, /*10.0f*/ 1.f };
	tDesc.fFontOutlineWidth = 2.f;
	tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_OUTLINE);
	tDesc.isTargetExist = false;
	tDesc.isInstance = true;
	tDesc.vScreenPos = vScreenPos;
	tDesc.strUIName = strUIName;
	tDesc.iPassType = 0;

	tDesc.strText = strText;

	tDesc.vecInstanceDescs.resize(tDesc.strText.size());
	for (_uint i = 0; i < tDesc.vecInstanceDescs.size(); i++)
		tDesc.vecInstanceDescs[i].matExtraData.m[0][0] = 1.f;

	// 인스턴스별 Transform 행렬 지정해주어야 함


	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CUI_Text* pTextObj = dynamic_cast<CUI_Text*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text", PROTOTYPE::GAMEOBJECT, &tDesc));

	ASSERT_CRASH(pTextObj); // test

	return pTextObj;
}

CUI_Text* CUI_FontPreset::Create_FontToScreen_Alpha(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName, _wstring strFontTag)
{
	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = m_FontTypeDesc[ENUM_CLASS(eColorType)];

	tDesc.strFontTag = strFontTag;
	tDesc.fScale = fFontScale;
	//tDesc.vLifeTime = { 0.0f, /*10.0f*/ 1.f };
	tDesc.fFontOutlineWidth = 2.f;
	tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	tDesc.isTargetExist = false;
	tDesc.isInstance = true;
	tDesc.vScreenPos = vScreenPos;
	tDesc.strUIName = strUIName;
	tDesc.iPassType = 0;

	tDesc.strText = strText;

	tDesc.vecInstanceDescs.resize(tDesc.strText.size());
	for (_uint i = 0; i < tDesc.vecInstanceDescs.size(); i++)
		tDesc.vecInstanceDescs[i].matExtraData.m[0][0] = 1.f;

	// 인스턴스별 Transform 행렬 지정해주어야 함


	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CUI_Text* pTextObj = dynamic_cast<CUI_Text*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text", PROTOTYPE::GAMEOBJECT, &tDesc));

	ASSERT_CRASH(pTextObj); // test

	return pTextObj;
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
