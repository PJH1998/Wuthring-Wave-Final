#include "ClientPch.h"
#include "Animator_UI.h"
#include "UI_HUD.h"

#include "Ability.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "UI_Text.h"

//#define KSTA_UI_COOLDOWNTEST
//#define KSTA_UI_HPBARTEST
//#define KSTA_UI_HPBARBOSSTEST
//#define KSTA_UI_ENERGYBARTEST


CUI_HUD::CUI_HUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
	, m_pGameSystem ( CGameSystem::GetInstance() )
{
	Safe_AddRef(m_pGameSystem);
}

CUI_HUD::CUI_HUD(const CUI_HUD& Prototype)
    :CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_HUD::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_HUD::Initialize_Clone(void* pArg)
{
    //__super::Initialize_Clone(pArg);

    CGameObject::Initialize_Clone(pArg);
#ifdef KSTA_ON_TRANSFORM_CACHING
	m_vecCachedUITransform.resize(1);
#endif // KSTA_ON_TRANSFORM_CACHING

    Ready_Components(pArg);
    __super::Ready_Events();
	Ready_Presets();

    // Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
    _wstring strFilePath = 
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json";
    Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();
	Ready_ChildExtraComponents();

    // Load Animations from json.
    vector<_wstring> vecAnimFilePaths = {
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/PartyFrame_FadeOut.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/PartyFrame_FadeIn.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/Status_FadeOut.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/Status_FadeIn.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/A_FadeOut.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/A_FadeIn.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/SkillIcons_FadeOut.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/SkillIcons_FadeIn.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/BossStatus_FadeOut.json",
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/BossStatus_FadeIn.json",

        L"../../Client/Bin/Resource/UI/FJson/UIAnim/BossStatus_Initialize.json",
    };
    Load_Animations(vecAnimFilePaths);

	// 보스 UI는, 최초에 투명하게.
	m_pPlayerStatus = m_pGameSystem->Get_PlayerStatus();
	static_cast<CAnimator_UI*>(m_pUI_SectorT_BossStatus->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_Initialize");

	// 보스 UI용 + 플레이어 UI용 텍스트 객체 생성 및 부모연결
	Ready_BossUINameText();
	Ready_PlayerHPText();
	Ready_SkillCooldownText();
	
	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_HUD", this);

    return S_OK;
}

void CUI_HUD::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_HUD::Update(_float fTimeDelta)
{
	m_iSelectedCHIndex = m_pPlayerStatus->Get_CurrentCharIndex();
	//if (m_iSelectedCHIndex == 2)
	//	m_iSelectedCHIndex = 0;

	if (m_iSelectedCHIndex == 2)
		int i = 10;

	m_pAbility = m_pPlayerStatus->Get_Ability(m_iSelectedCHIndex);

	Update_Presets();

	Update_UI_SkillSection(fTimeDelta);
	Update_UI_SkillSection_Wave(fTimeDelta);
	Update_UI_SkillSection_BG(fTimeDelta);
	Update_UI_SkillSection_Utility(fTimeDelta);
	Update_UI_SkillFeedback_Trigger(fTimeDelta);
	Update_UI_SkillSection_OnFeedback(fTimeDelta);
	Update_UI_PlayerHPBar(fTimeDelta);
	Update_UI_BossHPBar(fTimeDelta);
	Update_UI_KeyGuide(fTimeDelta);

	Update_UI_PlayerEnergyFrame(fTimeDelta);
	Update_UI_Icon_HarmonyReady(fTimeDelta);
	Update_UI_PlayerEnergyBar(fTimeDelta);
	Update_UI_PlayerEnergyBar_Augusta(fTimeDelta);
	Update_UI_PlayerEnergyBar_Galbrena(fTimeDelta);

	Update_Text_PlayerHP();
	Update_Text_PlayerCD();


	m_fElapsedTime += fTimeDelta;
    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_HUD::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_HUD::Render()
{
    //__super::Render();                      // Nothing. �����׷� �߰��� �� ���� ���������� �˾Ƽ� �ڽĵ���� Render ����
}

void CUI_HUD::PreAssign_ChildUIs()
{
	m_pUI_SectorT_BossStatus = Find_ChildObject(L"SectorT_BossStatus");
	m_pUI_SectorB_Status = Find_ChildObject(L"SectorB_Status");

	m_pUI_Skill[0] = Find_ChildObject(L"Skill_Rover");
	m_pUI_Skill[1] = Find_ChildObject(L"Skill_Augusta");
	m_pUI_Skill[2] = Find_ChildObject(L"Skill_Galbrena");

	m_pUI_Change[0] = Find_ChildObject(L"Icon_Rover");
	m_pUI_Change[1] = Find_ChildObject(L"Icon_Augusta");
	m_pUI_Change[2] = Find_ChildObject(L"Icon_Galbrena");


	m_pUI_Skill_ReadyFrame = Find_ChildObject(L"Skill_ReadyFrame");
	m_pUI_Skill_ReadyWave = Find_ChildObject(L"Skill_ReadyWave");
	m_pUI_Skill_BG = Find_ChildObject(L"Skill_BackgroundImage");

	m_pUI_SectorRB_SkillIcons = Find_ChildObject(L"SectorRB_SkillIcons");
	m_pUI_Skill_Utility = Find_ChildObject(L"Skill_Utility");

	m_pUI_Feedback = Find_ChildObject(L"Skill_OnFeedback");

	m_pUI_HPBar = Find_ChildObject(L"Inst_HPBar");

	m_pUI_BossHPBar = Find_ChildObject(L"Inst_BossHPBar");
	m_pUI_BossSABar = Find_ChildObject(L"Inst_BossSABar");

	m_pUI_KeyButton = Find_ChildObject(L"Inst_KeyButton");

	m_pUI_Group_Rover = Find_ChildObject(L"Group_Rover");
	m_pUI_Group_Augusta = Find_ChildObject(L"Group_Augusta");
	m_pUI_Group_Galbrena = Find_ChildObject(L"Group_Galbrena");

	m_pUI_Frame_Rover_Dark = Find_ChildObject(L"Frame_Rover_Dark");
	m_pUI_Frame_Augusta = Find_ChildObject(L"Frame_Augusta");
	m_pUI_FrameGroup_Augusta_OtherEnergy = Find_ChildObject(L"FrameGroup_Augusta_OtherEnergy");
	m_pUI_FrameGroup_Augusta_UltMode = Find_ChildObject(L"FrameGroup_Augusta_UltMode");
	m_pUI_Frame_Galbrena = Find_ChildObject(L"Frame_Galbrena");
	m_pUI_Frame_Galbrena_Icon = Find_ChildObject(L"Frame_Galbrena_Icon");
	m_pUI_FrameGroup_Galbrena_RageMode = Find_ChildObject(L"FrameGroup_Galbrena_RageMode");

	m_pUI_Icon_ElementDark = Find_ChildObject(L"Icon_ElementDark");
	m_pUI_Icon_ElementThunder = Find_ChildObject(L"Icon_ElementThunder");
	m_pUI_Icon_ElementFire = Find_ChildObject(L"Icon_ElementFire");
	m_pUI_Icon_ElementGuage = Find_ChildObject(L"Icon_ElementGuage");

	m_pUI_Icon_HarmonyIndicator = Find_ChildObject(L"Icon_HarmonyIndicator");
	m_pUI_Icon_HarmonyIndicatorBG = Find_ChildObject(L"Icon_HarmonyIndicatorBG");


	m_pUI_EnergyInstItems = Find_ChildObject(L"Inst_EnergyItems");

	m_pUI_Frame_Augusta_Inst_SwordEnergy = Find_ChildObject(L"Frame_Augusta_Inst_SwordEnergy");
	m_pUI_Frame_Augusta_Inst_CenterPointEnergy = Find_ChildObject(L"Frame_Augusta_Inst_CenterPointEnergy");
	m_pUI_Frame_Augusta_Inst_UltModeEnergy = Find_ChildObject(L"Frame_Augusta_Inst_UltModeEnergy");


	// 아래는 확인 필요. 최초에 그냥 할당해버리면 못찾가에 중간에 할당해주어야 함
	m_pTextUI_PlayerHP = Find_ChildObject(L"UI_Text_Player_HP");
	m_pTextUI_BossName = Find_ChildObject(L"UI_Text_HUD_BossName");

}

void CUI_HUD::Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio)
{
	// 단순히, 보스 정보를 1회성으로 할당함.
	m_pCurBossHP		= pCurBossHP;
	m_pCurBossSA		= pCurBossSA;
	m_pGroggyLeftRatio	= pGroggyLeftRatio;
	m_pIsGroggy			= pIsGroggy;
	m_strMonsterKey		= pMonsterKey;

	// 텍스트 객체에, 출력될 텍스트를 변경
	CUI_Text* pTargetText = static_cast<CUI_Text*>(m_pTextUI_BossName);
	pTargetText->Change_Text(strUIBosssName, TEXT_ALIGN_TYPE::CENTER);
}

HRESULT CUI_HUD::Ready_Components(void* pArg)
{
    return S_OK;
}

HRESULT CUI_HUD::Ready_ChildExtraComponents()
{
	//m_pUI_Skill_ReadyFrame 에 추가 텍스쳐 적용

	m_pUI_Skill_ReadyFrame->Add_ExtraTexture(L"T_DistortionMap0_DM");	// Extra 0
	m_pUI_Skill_ReadyFrame->Add_ExtraTexture(L"T_Caustic_Noise");		// Extra 1

	m_pUI_BossHPBar->Add_ExtraTexture(L"T_DistortionMap0_DM");			// Extra 0

	m_pUI_Icon_ElementGuage->Add_ExtraTexture(L"T_DistortionMap0_DM");	// Extra 1

	return S_OK;
}

HRESULT CUI_HUD::Ready_Presets()
{
	// ========== Image Sizes ==========

	// 재정립 필요

	array<_uint, 2> iImgSize_Rover = { 14, 1 };
	array<_uint, 2> iImgSize_Augusta = { 7, 2 };
	array<_uint, 2> iImgSize_Galbrena = { 10, 1 };

	m_mapSkillTexIndices.emplace(L"Rover_E",					Calc_SpriteSpace(0, 0, iImgSize_Rover));
	m_mapSkillTexIndices.emplace(L"Rover_R",					Calc_SpriteSpace(2, 0, iImgSize_Rover));
	m_mapSkillTexIndices.emplace(L"Rover_E_Burst",				Calc_SpriteSpace(1, 0, iImgSize_Rover));

	m_mapSkillTexIndices.emplace(L"Augusta_E",					Calc_SpriteSpace(5, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_E_GriffonReady",		Calc_SpriteSpace(3, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_E_RiseReady",		Calc_SpriteSpace(4, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_LB_Burst",			Calc_SpriteSpace(0, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_LB_StrongATK",		Calc_SpriteSpace(1, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_R",					Calc_SpriteSpace(0, 1, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_R_Enforce",			Calc_SpriteSpace(1, 1, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_R_Ready",			Calc_SpriteSpace(6, 0, iImgSize_Augusta));
	m_mapSkillTexIndices.emplace(L"Augusta_E_EnforceBasicATK",	Calc_SpriteSpace(2, 0, iImgSize_Augusta));

	m_mapSkillTexIndices.emplace(L"Galbrena_E",					Calc_SpriteSpace(4, 0, iImgSize_Galbrena));
	m_mapSkillTexIndices.emplace(L"Galbrena_E_BurstOn",			Calc_SpriteSpace(5, 0, iImgSize_Galbrena));
	m_mapSkillTexIndices.emplace(L"Galbrena_R",					Calc_SpriteSpace(6, 0, iImgSize_Galbrena));
	m_mapSkillTexIndices.emplace(L"Galbrena_LB_Burst",			Calc_SpriteSpace(2, 0, iImgSize_Galbrena));


	m_arrUtilCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::GRAPPLE)]	= {_float2(0.00f, 0.25f), _float2(0.50f, 0.75f)}; 
	m_arrUtilCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::SENSOR)]	= {_float2(0.75f, 1.00f), _float2(0.25f, 0.50f)}; 
	m_arrUtilCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::FLIGHT)]	= {_float2(0.25f, 0.50f), _float2(0.75f, 1.00f)}; 
	m_arrUtilCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::LEVITATOR)]= {_float2(0.25f, 0.50f), _float2(0.50f, 0.75f)}; 
	m_arrUtilCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::NOTHING)]	= {_float2(0.75f, 1.00f), _float2(0.75f, 1.00f)};


	m_arrPlayerSymbolicColors[CLR_ROVER]			= _float4(0.808f, 0.322f, 0.612f, 1.0f);
	m_arrPlayerSymbolicColors[CLR_AUGUSTA]			= _float4(0.969f, 0.451f, 1.000f, 1.0f);
	m_arrPlayerSymbolicColors[CLR_GALBRENA]			= _float4(1.000f, 0.416f, 0.416f, 1.0f);
	m_arrPlayerSymbolicColors[CLR_AUGUSTA_ULT]		= _float4(0.992f, 0.749f, 0.341f, 1.0f);

	m_arrPlayerAdvSymbolicColors[CLR_ROVER]			= _float4(0.485f, 0.193f, 0.367f, 1.0f);
	m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA]		= _float4(0.581f, 0.271f, 0.600f, 1.0f);
	m_arrPlayerAdvSymbolicColors[CLR_GALBRENA]		= _float4(0.600f, 0.250f, 0.250f, 1.0f);
	m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA_ULT]	= _float4(0.595f, 0.449f, 0.205f, 1.0f);


	return S_OK;
}

HRESULT CUI_HUD::Ready_BossUINameText()
{
	// 생성
	_float2 vTextPos = { 0.f, -477.f };
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",	// 상호작용 글씨
		TEXT_COLOR_TYPE::TT_BOSSNAME,
		0.4f,
		L"UI_Text_HUD_BossName"
	);

	// 연결
	CCustom_UI* pAttacher = m_pUI_SectorT_BossStatus;
	pFont->Attach_AsChildToUI(pAttacher);

	// 중앙 정렬
	auto& bossNameDesc = pFont->Get_TextUIDesc();
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_BossName = pFont;
	return S_OK;
}

HRESULT CUI_HUD::Ready_PlayerHPText()
{
	// 생성
	_float2 vTextPos = { 0.f, 496.f };
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"0/0",	// 현재체력/최대체력 표시
		TEXT_COLOR_TYPE::TT_PLAYERHP,
		0.22f,
		L"UI_Text_Player_HP"
	);

	// 연결 및 중앙정렬
	CCustom_UI* pAttacher = m_pUI_SectorB_Status;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_PlayerHP = pFont;
	return S_OK;
}

HRESULT CUI_HUD::Ready_SkillCooldownText()
{
	_float2 vTextPos;
	CUI_Text* pFont;
	CCustom_UI* pAttacher;

#pragma region SKILL CD - R
	// 생성
	
	vTextPos = { 850.f, 415.f - 6.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"이건R",
		TEXT_COLOR_TYPE::TT_SKILLCD,
		0.35f,
		L"UI_Text_SkillCD_R"
	);

	// 연결 및 중앙정렬
	pAttacher = m_pUI_SectorRB_SkillIcons;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_SkillCD_R = pFont;
#pragma endregion

#pragma region SKILL CD - E
	// 생성
	vTextPos = { 650.f, 415.f - 6.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"얘는E",
		TEXT_COLOR_TYPE::TT_SKILLCD,
		0.35f,
		L"UI_Text_SkillCD_E"
	);

	// 연결 및 중앙정렬
	pAttacher = m_pUI_SectorRB_SkillIcons;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_SkillCD_E = pFont;
#pragma endregion

#pragma region SKILL CD - T
	// 생성
	vTextPos = { 550.f, 415.f - 6.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"요건T",
		TEXT_COLOR_TYPE::TT_SKILLCD,
		0.35f,
		L"UI_Text_SkillCD_T"
	);

	// 연결 및 중앙정렬
	pAttacher = m_pUI_SectorRB_SkillIcons;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_SkillCD_T = pFont;
#pragma endregion


	//m_pUI_SectorRB_SkillIcons 에 attach.


	return S_OK;
}

//HRESULT CUI_HUD::Ready_ChangeCooldownText()
//{
//	return S_OK;
//}

void CUI_HUD::Update_Presets()
{
	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);
	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;

	switch (m_iSelectedCHIndex)
	{
	case CH_ROVER:		m_arrPlayerColors[CH_ROVER]			= m_arrPlayerSymbolicColors[CLR_ROVER];
						m_arrPlayerAdvColors[CH_ROVER]		= m_arrPlayerAdvSymbolicColors[CLR_ROVER];		break;
	case CH_AUGUSTA:	m_arrPlayerColors[CH_AUGUSTA]		=
					(	isIn_Augusta_AdvUlt ||													// 궁 사용중이거나
						m_pPlayerStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) == 1.f ) ?	// 궁 게이지 100%일 때 색 다르게
															  m_arrPlayerSymbolicColors[CLR_AUGUSTA_ULT] :
															  m_arrPlayerSymbolicColors[CLR_AUGUSTA];		
						m_arrPlayerAdvColors[CH_AUGUSTA]		=
					(	isIn_Augusta_AdvUlt ||													// 궁 사용중이거나
						m_pPlayerStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) == 1.f ) ?	// 궁 게이지 100%일 때 색 다르게
															  m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA_ULT] :
															  m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA];
																										break;
	case CH_GALBRENA:	m_arrPlayerColors[CH_GALBRENA]		= m_arrPlayerSymbolicColors[CLR_GALBRENA];
						m_arrPlayerAdvColors[CH_GALBRENA]	= m_arrPlayerAdvSymbolicColors[CLR_GALBRENA];	break;
	}
}

void CUI_HUD::Update_UI_SkillSection(_float fTimeDelta)
{
	// Temp assumed Value
	_float fMaxChangeCD[3] = { 2.f, 2.f, 2.f};

	// ==============================

	CPlayerStatus* pStatus = m_pPlayerStatus;

	auto& UISlots = pStatus->Get_Ability(m_iSelectedCHIndex)->Get_UISkillSlots();

	_float fBasicSkillCD[CH_END][SK_END] = {
		{/* CH_ROVER	*/ UISlots[CAbility::KEY_E].fCurrentCoolTime,	UISlots[CAbility::KEY_R].fCurrentCoolTime},
		{/* CH_AUGUSTA  */ UISlots[CAbility::KEY_E].fCurrentCoolTime,	UISlots[CAbility::KEY_R].fCurrentCoolTime},
		{/* CH_GALBRENA */ UISlots[CAbility::KEY_E].fCurrentCoolTime,	UISlots[CAbility::KEY_R].fCurrentCoolTime}
	};
	_float fBasicSkillMaxCD[CH_END][SK_END] = {
		{/* CH_ROVER	*/ UISlots[CAbility::KEY_E].fMaxCoolTime,		UISlots[CAbility::KEY_R].fMaxCoolTime},
		{/* CH_AUGUSTA  */ UISlots[CAbility::KEY_E].fMaxCoolTime,		UISlots[CAbility::KEY_R].fMaxCoolTime},
		{/* CH_GALBRENA */ UISlots[CAbility::KEY_E].fMaxCoolTime,		UISlots[CAbility::KEY_R].fMaxCoolTime}
	};
	_float fChangeCD[CH_END] = {
		.0f, .0f, .0f
	};
	_float fChangeMaxCD[CH_END] = {
		2.f, 2.f, 2.f
	};

	// ==============================

    switch (m_iSelectedCHIndex)   {
    case CH_ROVER:		m_pUI_Skill[0]->Set_Active(true);
						m_pUI_Skill[1]->Set_Active(false);
						m_pUI_Skill[2]->Set_Active(false);	break;
    case CH_AUGUSTA:	m_pUI_Skill[0]->Set_Active(false);
        				m_pUI_Skill[1]->Set_Active(true);
        				m_pUI_Skill[2]->Set_Active(false);	break;
    case CH_GALBRENA:	m_pUI_Skill[0]->Set_Active(false);
						m_pUI_Skill[1]->Set_Active(false);
						m_pUI_Skill[2]->Set_Active(true);	break;
    }

	// ==============================
	// * [Skill Icon Updates] Apply CD Value.
	// ==============================
	for (_uint i = 0; i < CH_END; i++)
	{
		auto targetUI = m_pUI_Skill[i];

		// 스킬 ui 인스턴스 갯수는 캐릭터마다 다름. 이에 따라 인스턴스 갯수만큼 리사이징 및 할당 
		vector<_float4x4> vecVariantMat = {/* _float4x4() , _float4x4() */};	

		_uint iTargetNumInstance = static_cast<_uint>(targetUI->Get_UIDesc().vecInstanceDescs.size());
		vecVariantMat.resize(iTargetNumInstance);

		_float fLeftColorMul		= 0.4f;
		_float fPassedColorMul		= 0.8f;
		_float fFilledColorMul		= 0.95f;

        vecVariantMat[0].m[0][0] = fBasicSkillCD[i][SK_E] / fBasicSkillMaxCD[i][SK_E];
        vecVariantMat[0].m[0][1] = fLeftColorMul;
		vecVariantMat[0].m[0][2] = (fBasicSkillCD[i][SK_E] != 0.f)? fPassedColorMul : fFilledColorMul;

        vecVariantMat[1].m[0][0] = fBasicSkillCD[i][SK_R] / fBasicSkillMaxCD[i][SK_R];
        vecVariantMat[1].m[0][1] = fLeftColorMul;
		vecVariantMat[1].m[0][2] = (fBasicSkillCD[i][SK_R] != 0.f) ? fPassedColorMul : fFilledColorMul;;

		if (vecVariantMat.size() >= 3)
		{
			vecVariantMat[2].m[0][0] = 0.0f;	// for LB Btn
			vecVariantMat[2].m[0][1] = fLeftColorMul;
			vecVariantMat[2].m[0][2] = fPassedColorMul;
		}

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);


		auto& UISlots = m_pAbility->Get_UISkillSlots();
		UI_CHARACTERTYPE eCharacterType = static_cast<UI_CHARACTERTYPE>(m_iSelectedCHIndex);
		switch (eCharacterType)
		{
			// * [SK Icon Update] Rover
		case UI_CHARACTERTYPE::ROVER:
			Update_Icon_Rover(UISlots);
			break;

			// * [SK Icon Update] Augusta
		case UI_CHARACTERTYPE::AUGUSTA:
			Update_Icon_Augusta(UISlots);
			break;

			// * [SK Icon Update] Galbrena
		case UI_CHARACTERTYPE::GALBRENA:
			Update_Icon_Galbrena(UISlots);
			break;
		}

    }


    // ==============================
	// * [CH Change Update] CH Change Cooldown
	// ==============================
    for (_uint i = 0; i < CH_END; i++)
    {
        _float fCooldown = fChangeCD[i];
        auto targetUI = m_pUI_Change[i];

        vector<_float4x4> vecVariantMat = { _float4x4() };
        vecVariantMat[0].m[0][0] = 1.f - (fCooldown / fMaxChangeCD[i]);
        vecVariantMat[0].m[0][1] = 1.0f;
        vecVariantMat[0].m[0][2] = 0.8f;

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_RECT),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);
    }



	// ==============================
	// * [Skill Ready Circle] Skill Ready Indicator
	// ==============================
	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isReady_Augusta_StrongATK = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType) == UI_AUGUSTA_STATE::LB_STRONG_READY;
	_bool isIn_Galbrena_BurstMode = (m_pPlayerStatus->Get_CurrentCharIndex() == CH_GALBRENA) ?
		m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE)) : false;
	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);
	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;


	static _uint iIndex_EBtn = 2;		static _uint iIndex_PrevEBtn;
	static _uint iIndex_RBtn = 0;		static _uint iIndex_PrevRBtn ;
	static _uint iIndex_LBBtn = 4;		static _uint iIndex_PrevLBBtn ;
	//_uint iIndex_TBtn = ..


	// 스킬 배치 변경
	switch (m_pPlayerStatus->Get_CurrentCharIndex())
	{
	case CH_AUGUSTA:
	{
		if (isIn_Augusta_AdvUlt)
		{
			iIndex_EBtn = 2;
			iIndex_RBtn = 0;
			iIndex_LBBtn = 1;
		}
	}break;
	case CH_GALBRENA:
	default:
		iIndex_EBtn = 2;
		iIndex_RBtn = 0;
		iIndex_LBBtn = 4;
		break;
	}


	CCustom_UI* pSkillReadyUI = m_pUI_Skill_ReadyFrame;

	auto& readyDesc = pSkillReadyUI->Get_UIDesc();
	auto& readyInstDesc = pSkillReadyUI->Get_UIDesc().vecInstanceDescs;
	for (auto& instDesc : readyInstDesc)
		instDesc.vClipTexcoordX = { 0.f, 0.f };
	
	vector<_float4x4> vecVariantMat = { }; vecVariantMat.resize(readyInstDesc.size());

	// - LB/E Button Indicator : 특수 공격이 준비 될 시에 불만 들어옴.
	 
	//if ("특수 공격 준비 시 함수 따로 만들어야 할 듯. 플레이어 종류마다 조건 제각각이라")
	//	readyInstDesc[iIndex_EBtn].vClipTexcoordX = { 0, 1 };
	//else
	//	readyInstDesc[iIndex_EBtn].vClipTexcoordX = { 0, 0 };

	if (isReady_Augusta_StrongATK)		readyInstDesc[iIndex_LBBtn].vClipTexcoordX = { 0, 1 };
	else								readyInstDesc[iIndex_LBBtn].vClipTexcoordX = { 0, 0 };
	if (isIn_Augusta_AdvUlt)			readyInstDesc[iIndex_LBBtn].vClipTexcoordX = { 0, 1 };
	else								readyInstDesc[iIndex_LBBtn].vClipTexcoordX = { 0, 0 };


	switch (m_iSelectedCHIndex)
	{
	case CH_AUGUSTA:	readyInstDesc[iIndex_LBBtn].vClipTexcoordX = (isIn_Augusta_AdvUlt) ?		_float2{ 0.f, 1.f } : _float2{ 0.f, 0.f };	break;
	case CH_GALBRENA:	readyInstDesc[iIndex_LBBtn].vClipTexcoordX = (isIn_Galbrena_BurstMode) ?	_float2{ 0.f, 1.f } : _float2{ 0.f, 0.f };	break;
	default:			readyInstDesc[iIndex_LBBtn].vClipTexcoordX = _float2{ 0.f, 0.f };														break;
	}
	
	

	;

	// - R Button Indicator : 원으로 게이지 차고 (COST5) , 다 차면 불 들어옴
	
	_float fUltGuage = pStatus->Get_CostRatio(m_iSelectedCHIndex, COST_TYPE::COST5);
	vector<_float4> vecCustomColor		= { };		vecCustomColor.resize(CH_END);
	vector<_float4> vecAdvCustomColor	= { };		vecAdvCustomColor.resize(CH_END);
	
	switch (m_iSelectedCHIndex)
	{
	case CH_ROVER:		vecCustomColor[CH_ROVER]		= m_arrPlayerSymbolicColors[CLR_ROVER];		
						vecAdvCustomColor[CH_ROVER]		= m_arrPlayerAdvSymbolicColors[CLR_ROVER];		break;
	case CH_AUGUSTA:	vecCustomColor[CH_AUGUSTA]		=
					(	isIn_Augusta_AdvUlt ||											// 궁 사용중이거나
						pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) == 1.f ) ?	// 궁 게이지 100%일 때 색 다르게
													      m_arrPlayerSymbolicColors[CLR_AUGUSTA_ULT] :
													      m_arrPlayerSymbolicColors[CLR_AUGUSTA];		
						vecAdvCustomColor[CH_AUGUSTA]	=
					(	isIn_Augusta_AdvUlt ||											// 궁 사용중이거나
						pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) == 1.f ) ?	// 궁 게이지 100%일 때 색 다르게
														  m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA_ULT] :
														  m_arrPlayerAdvSymbolicColors[CLR_AUGUSTA];
																										break;
	case CH_GALBRENA:	vecCustomColor[CH_GALBRENA]		= m_arrPlayerSymbolicColors[CLR_GALBRENA];		
						vecAdvCustomColor[CH_GALBRENA]	= m_arrPlayerAdvSymbolicColors[CLR_GALBRENA];	break;
	}

	// - 활성화 여부 지정
	// - R
	_bool is_RBtn_Active = true;

	switch (m_iSelectedCHIndex)
	{
	case CH_ROVER:		is_RBtn_Active				= true;		break;
	case CH_AUGUSTA:	is_RBtn_Active				= 
					(	isIn_Augusta_AdvUlt && pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST4) < 1.0f) ?
													  false : true;
																break;
	case CH_GALBRENA:	is_RBtn_Active				= true;		break;
	}

	readyInstDesc[iIndex_RBtn].vClipTexcoordX = (is_RBtn_Active) ? _float2{ 0.f, 1.f } : _float2{ 0.f, 0.f };


	// 이걸 이게 스킬별로 대응?
	
	for (_uint i = 0; i < vecVariantMat.size(); i++)
	{	// 기본값 설정
		*reinterpret_cast<_float*>(&vecVariantMat[i]._11) = 0.f;
		*reinterpret_cast<_float*>(&vecVariantMat[i]._12) = 0.f;								// ColorMul1
		*reinterpret_cast<_float*>(&vecVariantMat[i]._13) = 1.f;								// ColorMul2
		*reinterpret_cast<_float*>(&vecVariantMat[i]._14) = static_cast<_float>(true);		// Is Use CustomColor?
		*reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vecCustomColor[m_iSelectedCHIndex];	// CustomColor
		*reinterpret_cast<_float*>(&vecVariantMat[i]._31) = 0.f;							// CD Start Degree
		*reinterpret_cast<_float*>(&vecVariantMat[i]._32) = static_cast<_float>(fUltGuage == 1.f);	// isUseNoise
		*reinterpret_cast<_float*>(&vecVariantMat[i]._33) = m_fElapsedTime;							// Elapsed Time
		*reinterpret_cast<_float*>(&vecVariantMat[i]._34) = 0.2f;							// UV Scroll Speed
		*reinterpret_cast<_float4*>(&vecVariantMat[i]._41) = vecAdvCustomColor[m_iSelectedCHIndex];	// Mask Color
	}

	// R에 대한 예외 적용
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._11) = (1.f - fUltGuage / 1.f);		// CD or Resource Rate
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._12) = 0.f;							// ColorMul1
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._13) = (fUltGuage >= 1.f) ? 1.f : 0.85f ;	// ColorMul2
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._14) = static_cast<_float>(true);	// Is Use CustomColor?
	*reinterpret_cast<_float4*>(&vecVariantMat[iIndex_RBtn]._21)= vecCustomColor[m_iSelectedCHIndex];	// CustomColor
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._31) = 0.f;							// CD Start Degree

	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._32) = static_cast<_float>(fUltGuage == 1.f);	// isUseNoise
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._33) = m_fElapsedTime;							// Elapsed Time
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._34) = 0.2f;							// UV Scroll Speed
	*reinterpret_cast<_float4*>(&vecVariantMat[iIndex_RBtn]._41)= vecAdvCustomColor[m_iSelectedCHIndex];	// Mask Color

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
		true
	};

	// 만약 버튼 인덱스가 바뀐다면, 꼬임 방지를 위한 이전 버튼 인덱스의 비활성화.
	if (iIndex_PrevEBtn != iIndex_EBtn)		readyInstDesc[iIndex_EBtn].vClipTexcoordX = { 0, 0 };
	if (iIndex_PrevRBtn != iIndex_RBtn)		readyInstDesc[iIndex_RBtn].vClipTexcoordX = { 0, 0 };
	if (iIndex_PrevLBBtn != iIndex_LBBtn)	readyInstDesc[iIndex_LBBtn].vClipTexcoordX = { 0, 0 };

	readyDesc.vecInstanceDescs = readyInstDesc;
	pSkillReadyUI->Set_VariantUIDesc(tVariantDesc);

	iIndex_PrevEBtn = iIndex_EBtn;
	iIndex_PrevRBtn = iIndex_RBtn;
	iIndex_PrevLBBtn = iIndex_LBBtn;
}

void CUI_HUD::Update_UI_SkillSection_Wave(_float fTimeDelta)
{
	CCustom_UI* pTargetUI = m_pUI_Skill_ReadyWave;
	_bool isUltGuageFull = m_pPlayerStatus->Get_CostRatio(m_iSelectedCHIndex, COST_TYPE::COST5) == 1.f;

	if (!isUltGuageFull)
	{
		pTargetUI->SetActivate(false);
		return;
	}

	pTargetUI->SetActivate(true);


	auto& waveDesc = pTargetUI->Get_UIDesc();
	auto& waveInstDesc = waveDesc.vecInstanceDescs;
	waveInstDesc.resize(1);

	static vector<_float4x4> vecVariantMat = { _float4x4() };

	// ===== Variant Edit.. ===== 
	
	static _float fDistortStrength = 0.f;
	static _float fLateDistortStrength = 0.f;

	static _float fDistortMin = 0.25f;
	static _float fDistortMax = 0.8f;
	static _float fFollowStrength = 0.01f;
	if (m_pGameInstance->Rand_Normal() <= 0.1f)
	{
		fDistortStrength += 0.16f;
	}
	fDistortStrength = fDistortStrength - 0.02f;
	fDistortStrength = clamp(fDistortStrength, fDistortMin, fDistortMax);

	if (fDistortStrength > fLateDistortStrength)	fLateDistortStrength += fFollowStrength * 2.f;
	if (fDistortStrength < fLateDistortStrength)	fLateDistortStrength -= fFollowStrength;
	fLateDistortStrength = clamp(fLateDistortStrength, fDistortMin, fDistortMax);

	_float fUVRotateSpeed = -10.f / 60.f;


	_float4 vDestColor = m_arrPlayerColors[m_iSelectedCHIndex];
	*reinterpret_cast<_float4*>(&vecVariantMat[0]._11) = vDestColor;					// dest color
	*reinterpret_cast<_float*>(&vecVariantMat[0]._21) = static_cast<_float>(true);	// is Distort On?
	*reinterpret_cast<_float*>(&vecVariantMat[0]._22) = m_fElapsedTime;				// ElapsedTime. for transforming UV
	*reinterpret_cast<_float*>(&vecVariantMat[0]._23) = fLateDistortStrength;		// distort strength.
	*reinterpret_cast<_float*>(&vecVariantMat[0]._24) = fUVRotateSpeed;				// rotate speed (deg per sec).
	*reinterpret_cast<_float*>(&vecVariantMat[0]._31) = 1.5f;						// Alpha Multiplier.
	*reinterpret_cast<_float*>(&vecVariantMat[0]._32) = static_cast<_float>(false);	// isDisableNormalize (deg per sec).


	// ==============================

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_WAVECIRCLE),
		true
	};
	
	pTargetUI->Set_VariantUIDesc(tVariantDesc);
}

void CUI_HUD::Update_UI_SkillSection_Utility(_float fTimeDelta)
{
	CCustom_UI* pTargetUI = m_pUI_Skill_Utility;

	auto& utilDesc = pTargetUI->Get_UIDesc();
	auto& utilInstDesc = utilDesc.vecInstanceDescs[0];


	// ksta : m_iUtilityIndex_Tmp 나중에 플레이어가 들고있는 현재 유틸스킬 반드시 연결할 것.

	UI_TAB_UTILITY ePlayerUtility = m_pPlayerStatus->Get_UtilityType();


	utilInstDesc.vSInstCoordX = m_arrUtilCoordPresets[ENUM_CLASS(ePlayerUtility)][0];
	utilInstDesc.vSInstCoordY = m_arrUtilCoordPresets[ENUM_CLASS(ePlayerUtility)][1];
}

void CUI_HUD::Update_UI_SkillSection_BG(_float fTimeDelta)
{
	// ==============================
	// * Skill_BackgroundImage
	// =============================='

	
	static _uint iNumActiveBG = 4;

	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isReady_Augusta_StrongATK = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType) == UI_AUGUSTA_STATE::LB_STRONG_READY;
	_bool isIn_Galbrena_BurstMode = (m_pPlayerStatus->Get_CurrentCharIndex() == CH_GALBRENA) ?
		m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE)) : false;

	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;


	switch (m_pPlayerStatus->Get_CurrentCharIndex())
	{
	case CH_ROVER:
	{
		iNumActiveBG = 4;
	}break;
	case CH_AUGUSTA:
	{
		if (isIn_Augusta_AdvUlt)
			iNumActiveBG = 2;
		else if (isReady_Augusta_StrongATK) //
			iNumActiveBG = 5;
		else
			iNumActiveBG = 4;
	}break;
	case CH_GALBRENA:
	{	// ksta : 버스트 모드 시에 5로 늘려야 함
		if (isIn_Galbrena_BurstMode)
			iNumActiveBG = 5;
		else
			iNumActiveBG = 4;
	}break;
	default:
		break;
	}



	_float4 vBGColor = _float4{ .5f, .5f, .5f, .3f };

	CCustom_UI* pSkillBGUI = m_pUI_Skill_BG;    // �ν��Ͻ� 4����

	vector<_float4x4> vecBGVariantMat = {};
	vecBGVariantMat.resize(5);

	for (_uint i = 0; i < iNumActiveBG; i++)
	{
		*reinterpret_cast<_float4*>(&vecBGVariantMat[i]._11) = vBGColor;
		*reinterpret_cast<_float*>(&vecBGVariantMat[i]._21) = (i < iNumActiveBG) ? static_cast<_float>(true) : static_cast<_float>(false);
	}

	CCustom_UI::VARIANTREADY_UI_DESC tBGVariantDesc = {
		vecBGVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLEMASK),
		true
	};


	pSkillBGUI->Set_VariantUIDesc(tBGVariantDesc);
}

void CUI_HUD::Update_UI_SkillFeedback_Trigger(_float fTimeDelta)
{
	// ==============================
	// * SkillBtn_FeedBack
	// =============================='

	if (!m_pUI_SectorRB_SkillIcons->IsActivate())
		return;


	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isReady_Augusta_StrongATK = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType) == UI_AUGUSTA_STATE::LB_STRONG_READY;
	_bool isIn_Galbrena_BurstMode = false; /* 나중에 버스트 모드 조건 삽입 */

	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;


	_uint iIndex_EBtn =	 2;
	_uint iIndex_RBtn =  0;
	_uint iIndex_LBBtn = 4;
	//_uint iIndex_TBtn = ..

	switch (m_pPlayerStatus->Get_CurrentCharIndex())
	{
	case CH_AUGUSTA:
	{
		if (isIn_Augusta_AdvUlt)
		{
			iIndex_EBtn = 2;
			iIndex_RBtn = 0;
			iIndex_LBBtn = 1;
		}
	}break;
	case CH_GALBRENA:
	{	// ksta : 버스트 모드 시에 5로 늘려야 함
	}break;
	default:
		iIndex_EBtn = 2;
		iIndex_RBtn = 0;
		iIndex_LBBtn = 4;
		break;
	}

	

	// LB Btn
	_bool isChar_LBBtnFeedbackAble = false;

	switch (m_pPlayerStatus->Get_CurrentCharIndex())
	{
	case CH_ROVER:
	{
		isChar_LBBtnFeedbackAble = false;
	}break;
	case CH_AUGUSTA:
	{
		_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
		UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

		_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;
		if (isIn_Augusta_AdvUlt)
			isChar_LBBtnFeedbackAble = true;
		else
			isChar_LBBtnFeedbackAble = false;
	}break;
	case CH_GALBRENA:
	{
		_bool isIn_Galbrena_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));
		if (isIn_Galbrena_BurstMode)
			isChar_LBBtnFeedbackAble = true;
		else
			isChar_LBBtnFeedbackAble = false;
	}break;
	}


	// E Btn
	_bool isChar_EButtonFeedbackAble = true;

	switch (m_pPlayerStatus->Get_CurrentCharIndex())
	{
	case CH_ROVER:
	{
	}break;
	case CH_AUGUSTA:
	{
		_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
		UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

		_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;
		if (isIn_Augusta_AdvUlt)
			isChar_EButtonFeedbackAble = false;
		else
			isChar_EButtonFeedbackAble = true;
	}break;
	case CH_GALBRENA:
	{
	}break;
	}


	// R Btn
	_bool isChar_RuttonFeedbackAble = true;


	
	// 클릭마다 해당 위치에 피드백 생성
	if (m_pGameInstance->Get_DIKeyState(DIK_E) == KEYSTATE::DOWN &&
		isChar_EButtonFeedbackAble)
		Add_UI_SkillSection_OnFeedback(iIndex_EBtn);
	if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN &&
		isChar_RuttonFeedbackAble)
		Add_UI_SkillSection_OnFeedback(iIndex_RBtn);
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN &&
		isChar_LBBtnFeedbackAble)
		Add_UI_SkillSection_OnFeedback(iIndex_LBBtn);
}

void CUI_HUD::Update_UI_SkillSection_OnFeedback(_float fTimeDelta)
{
    CCustom_UI* pFeedbackUI = m_pUI_Feedback;
    const _float2 fDestScale = { 1.2f, 1.2f };
    const _float fStartAlpha = 0.f;     // 0�� ����, 1�� �Ⱥ������� ����.
    const _float fLifeTime = .5f;
    _float4 vColor = { 0.f, 0.f, 0.f, 1.f };

    auto& uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;


    static vector<_float> vecLifeTimeElapsed = {};
    
    if (uiInstDescs.size() > vecLifeTimeElapsed.size())
    {
		_uint iAddLoopTime = static_cast<_uint>(uiInstDescs.size() - vecLifeTimeElapsed.size());
        for (_uint i = 0; i < iAddLoopTime; i++)
            vecLifeTimeElapsed.push_back(0.f);
    }

    vector<_float4x4> vecVariantMat = {};
    vecVariantMat.resize(uiInstDescs.size());

    for (uint i = 0; i < vecVariantMat.size(); i++)
    {
        *reinterpret_cast<_float2*>(&vecVariantMat[i]._11) = fDestScale;
        *reinterpret_cast<_float*>(&vecVariantMat[i]._13) = fStartAlpha;
        *reinterpret_cast<_float*>(&vecVariantMat[i]._14) = vecLifeTimeElapsed[i] / fLifeTime;
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vColor;
    }

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ACTIVEFEEDBACK),
        true
    };

    pFeedbackUI->Set_VariantUIDesc(tVariantDesc);

    for (_uint i = 0; i < vecLifeTimeElapsed.size(); i++)
    {
        if (vecLifeTimeElapsed[i] >= fLifeTime)
        {
            vecLifeTimeElapsed.erase(vecLifeTimeElapsed.begin() + i);
            uiInstDescs.erase(uiInstDescs.begin() + i);

            uiDesc.vecInstanceDescs = uiInstDescs; // ����
            i--;
        }
        else
        {
            vecLifeTimeElapsed[i] += fTimeDelta;
        }
    }
}

void CUI_HUD::Add_UI_SkillSection_OnFeedback(_uint iSectionIndex)
{
    CCustom_UI* pFeedbackUI = m_pUI_Feedback;

    const _float4 vStartPos = { 850.f, -415.f, 0.f, 1.f };
    const _float fDeltaPosX = -100.f;

    vector<_float4> vecPosIndex = {};

    for (_uint i = 0; i < 5; i++)
    {
        _float4 vPos = vStartPos;
        vPos.x = vPos.x + fDeltaPosX * (i);
        vecPosIndex.push_back(vPos);
    }

    auto& uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;



    CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
    tDesc.vSInstRight   = { 80.f, 0.f, 0.f, 0.f };
    tDesc.vSInstUp      = { 0.f, 80.f, 0.f, 0.f };
    tDesc.vSInstLook    = { 0.f, 0.f, 1.f, 0.f };
    tDesc.vSInstTrans   = vecPosIndex[iSectionIndex];

    uiInstDescs.push_back(tDesc);
    uiDesc.vecInstanceDescs = uiInstDescs;
}

void CUI_HUD::Update_Text_PlayerHP()
{
	CUI_Text* pTargetText = dynamic_cast<CUI_Text*>(m_pTextUI_PlayerHP);
	if (!pTargetText)
		return;

	auto& playerHPDesc = pTargetText->Get_TextUIDesc();


	_uint iPlayerCurHP = static_cast<_uint>(m_pAbility->Get_Hp());
	_uint iPlayerMaxHP = static_cast<_uint>(m_pAbility->Get_MaxHp());

	playerHPDesc.strText = to_wstring(iPlayerCurHP) + L"/" + to_wstring(iPlayerMaxHP);

	pTargetText->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);
}

void CUI_HUD::Update_Text_PlayerCD()
{
	auto& UISlots = m_pPlayerStatus->Get_Ability(m_iSelectedCHIndex)->Get_UISkillSlots();

	// 현재 쿨타임 가져오기
	_float fCurCHCD_R = UISlots[CAbility::KEY_R].fCurrentCoolTime;
	_float fCurCHCD_E = UISlots[CAbility::KEY_E].fCurrentCoolTime;
	_float fCurCHCD_T = 0.f;		// 나중에 쓸 것 같으면 그떄가서 넣기

	// 0초면 안보이게.
	if (fCurCHCD_R == 0.f) m_pTextUI_SkillCD_R->SetActivate(false);			else m_pTextUI_SkillCD_R->SetActivate(true); 
	if (fCurCHCD_E == 0.f) m_pTextUI_SkillCD_E->SetActivate(false);			else m_pTextUI_SkillCD_E->SetActivate(true); 
	if (fCurCHCD_T == 0.f) m_pTextUI_SkillCD_T->SetActivate(false);			else m_pTextUI_SkillCD_T->SetActivate(true);

	// 쿨타임 소숫점 잘라내기
	_tchar wbuf_R[32];	swprintf_s(wbuf_R, L"%.1f", fCurCHCD_R);	_wstring str_R(wbuf_R);
	_tchar wbuf_E[32];	swprintf_s(wbuf_E, L"%.1f", fCurCHCD_E);	_wstring str_E(wbuf_E);
	_tchar wbuf_T[32];	swprintf_s(wbuf_T, L"%.1f", fCurCHCD_T);	_wstring str_T(wbuf_T);

	// 쿨타임 반영
	static_cast<CUI_Text*>(m_pTextUI_SkillCD_R)->Change_Text(str_R);
	static_cast<CUI_Text*>(m_pTextUI_SkillCD_E)->Change_Text(str_E);
	static_cast<CUI_Text*>(m_pTextUI_SkillCD_T)->Change_Text(str_T);

}

void CUI_HUD::Update_UI_PlayerHPBar(_float fTimeDelta)
{
	// - required info list..
	// fPlayerHPRatio
	

    //static _float fPlayerHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    //static _float fPlayerBackHP[CH_END] = { fPlayerHP[0], fPlayerHP[1], fPlayerHP[2] };
    //const _float fPlayerMaxHP[CH_END] = { 2000.f, 4000.f, 10000.f };
    static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;
    


    //_float fPlayerHPRatio = fPlayerHP[iSelectedCHIndex] / fPlayerMaxHP[iSelectedCHIndex];
	_float fPlayerHPRatio = m_pPlayerStatus->Get_HpRatio(m_iSelectedCHIndex);
	static _float fPlayerHPPrevRatio = 0.f;
	static _float fPlayerHPBackRatio = 0.f; //  = fPlayerHPRatio;

    _float4 vHPColor        = { 1.f, 1.f, 1.f, 1.f };
    _float4 vHPBackColor    = { 1.f, 0.f, 0.f, 1.f };

    const _float fHPReduceTime = 0.5f;          // �پ��� �ҿ�ð��� 0.5������?

    CCustom_UI* targetUI = m_pUI_HPBar;



    
    if (fHPReduceLeftTime > 0)
    {
        _float diff = fPlayerHPBackRatio - fPlayerHPRatio;              // ü�� ���� ����

        if (diff > 0.f)
        {
            _float delta = diff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� ü�� ����

            fPlayerHPBackRatio -= delta;                               
            if (fPlayerHPBackRatio < fPlayerHPRatio)
                fPlayerHPBackRatio = fPlayerHPRatio;
        }

        fHPReduceLeftTime -= fTimeDelta;
        if (fHPReduceLeftTime < 0)
            fHPReduceLeftTime = 0;
    }
    else
    {
        fPlayerHPBackRatio = fPlayerHPRatio;
    }


	// HP 변화를 감지하여 피격 여부 확인
	if (fPlayerHPPrevRatio > fPlayerHPRatio)
	{
		isHit = true;
	}
	// 피격 여부 확인 시 뒷 HP바가 따라가기 시작
	if (isHit == true)
	{
		fHPReduceLeftTime = fHPReduceTime;
	}



    // change
    vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._11)    = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._11)      = vHPBackColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_NORMAL]._21)    = vHPColor;
    *reinterpret_cast<_float4*>(&vecVariantMat[PLHP_BACK]._21)      = vHPBackColor;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_NORMAL]._31)     = fPlayerHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[PLHP_BACK]._31)       = fPlayerHPBackRatio;

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };
    
    targetUI->Set_VariantUIDesc(tVariantDesc);


    isHit = false;
	fPlayerHPPrevRatio = fPlayerHPRatio;


#ifdef KSTA_UI_HPBARTEST
    std::cout << "[UI_HUD][Update_UI_HPBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_HPBar] 1 fPlayerHP     : " << fPlayerHPRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_HPBar] 2 fPlayerHPBack : " << fPlayerHPBackRatio << std::endl;
#endif // KSTA_UI_HPBARTEST

}

void CUI_HUD::Update_UI_BossHPBar(_float fTimeDelta)
{
	// boss hitpoint & superarmor
	_float	fBossHP		= 0.f;
	_float	fBossBackHP = 0.f;
	_float	fBossMaxHP	= 1.f;
	
	_float	fBossSA		= 0.f;
    _float	fBossBackSA = 0.f;
    _float	fBossMaxSA	= 1.f;
    _bool	isSABreak	= false;

	static _float	fTmpBossHP = 0.f;
	static _float	fTmpBossSA = 0.f;

	if (m_isOn_BossStatus)
	{
		fBossHP		= *m_pCurBossHP;
		fBossMaxHP	= m_pGameSystem->Get_MonsterInfo(m_strMonsterKey.c_str())->fMaxHp;
		fBossBackHP = (fBossHP == fBossMaxHP)? fBossHP : m_fBackBossHP;
		
		isSABreak	= *m_pIsGroggy;
		
		if (!isSABreak)
		{
			fBossSA = *m_pCurBossSA;
			fBossMaxSA = m_pGameSystem->Get_MonsterInfo(m_strMonsterKey.c_str())->fMaxStamina;
			fBossBackSA = (fBossSA == fBossMaxSA) ? fBossSA : m_fBackBossSA;
		}
		else
		{
			fBossSA = *m_pGroggyLeftRatio;
			fBossMaxSA = 1.f;
			fBossBackSA = (fBossSA == fBossMaxSA) ? fBossSA : m_fBackBossSA;
		}
	}
	
	//static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;

    _float fBossHPRatio = fBossHP / fBossMaxHP;
    _float fBossSARatio = fBossSA / fBossMaxSA;
    static _float fBossHPBackRatio = fBossHPRatio;
    static _float fBossSABackRatio = fBossSARatio;


	// Colors
    const _float4 vHPColor1         = { 1.f, .7f, .1f, 1.f };
    const _float4 vHPColor2         = { 1.f, .2f, .0f, 1.f };
    const _float4 vHPBackColor1     = { .8f, .8f, .8f, 1.f };

    const _float4 vSAColor          = { 1.f, 1.f, 1.f, 1.f };   // before armor break
    const _float4 vSABreakColor     = { .9f, .8f, .3f, 1.f };
    const _float4 vSABackColor      = { 1.f, 1.f, 1.f, .3f };   // after armor break
    //const _float4 vSABreakBackColor = { .2f, .2f, .2f, 1.f };

    const _float fHPReduceTime = 0.5f;          // �پ��� �ҿ�ð��� 0.5������?

    const auto targetUI		= m_pUI_BossHPBar;
    const auto targetSAUI	= m_pUI_BossSABar;


	// Back Guage
    if (fHPReduceLeftTime > 0)
    {
        _float fHPDiff = fBossHPBackRatio - fBossHPRatio;              // ü�� ���� ����
        _float fSADiff = fBossSABackRatio - fBossSARatio;              // �Ƹ� ���� ����

        if (fHPDiff > 0.f)
        {
            _float fHPDelta = fHPDiff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� ü�� ����

            fBossHPBackRatio -= fHPDelta;
            if (fBossHPBackRatio < fBossHPRatio)
                fBossHPBackRatio = fBossHPRatio;
        }
        if (fSADiff > 0.f)
        {
            _float fSADelta = fSADiff * (fTimeDelta / fHPReduceLeftTime);     // �پ�� �Ƹ� ����

            fBossSABackRatio -= fSADelta;
            if (fBossSABackRatio < fBossSARatio)
                fBossSABackRatio = fBossSARatio;
        }

        fHPReduceLeftTime -= fTimeDelta;
        if (fHPReduceLeftTime < 0)
            fHPReduceLeftTime = 0;
    }
    else
    {
        fBossHPBackRatio = fBossHPRatio;
        fBossSABackRatio = fBossSARatio;
    }


	if (fTmpBossHP > fBossHP || fTmpBossSA > fBossSA)
	{
		//isHit = true;
		fHPReduceLeftTime = fHPReduceTime;
		//cout << "[UI_HUD::Update_UI_BossHPBar] Triggered!" << endl;
	}

	if (!(fHPReduceLeftTime <= 0.01f))
	{
		//cout << "[UI_HUD::Update_UI_BossHPBar] [LeftTime] : " << fHPReduceLeftTime << endl;
	}



    // change
    vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._11)    = vHPColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._11)      = vHPBackColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._21)    = vHPColor2;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._21)      = vHPBackColor1;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._31)     = fBossHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_BACK]._31)       = fBossHPBackRatio;

	*reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._32) = static_cast<_float>(true);	// isUseNoise
	*reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._33) = m_fElapsedTime;							// Elapsed Time
	*reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._34) = 0.2f;							// UV Scroll Speed
	*reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._41) = _float4(0.698f, 0.212f, 0.035f, 1.000f);	// Mask Color


    vector<_float4x4> vecVariantMatSA = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._11)  = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._11)    = vSABackColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_NORMAL]._21)  = (isSABreak) ? vSABreakColor : vSAColor;
    *reinterpret_cast<_float4*>(&vecVariantMatSA[BOSA_BACK]._21)    = vSABackColor;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_NORMAL]._31)   = fBossSARatio;
    *reinterpret_cast<_float*>(&vecVariantMatSA[BOSA_BACK]._31)     = fBossSABackRatio;


    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDescSA = {
        vecVariantMatSA,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
        true
    };

    targetUI->Set_VariantUIDesc(tVariantDesc);
    targetSAUI->Set_VariantUIDesc(tVariantDescSA);

    //isHit = false;
	fTmpBossHP = fBossHP;
	fTmpBossSA = fBossSA;


#ifdef KSTA_UI_HPBARBOSSTEST
    std::cout << "[UI_HUD][Update_UI_BossHPBar] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 1 fBossHP     : " << fBossHPRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 2 fBossHPBack : " << fBossHPBackRatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 1 fBossSA     : " << fBossSARatio << std::endl;
    std::cout << "[UI_HUD][Update_UI_BossHPBar] 2 fBossSABack : " << fBossSABackRatio << std::endl;
#endif // KSTA_UI_HPBARBOSSTEST



}

void CUI_HUD::Update_UI_KeyGuide(_float fTimeDelta)
{
	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();


    // ĳ���Ϳ� ���� Ű ���̵� ���̱� ���� �б�
    CCustom_UI* pKeyButtonUI = m_pUI_KeyButton;
    auto& keyButtonDesc = pKeyButtonUI->Get_UIDesc();

    switch (m_iSelectedCHIndex)
    {
    case Client::CUI_HUD::CH_ROVER:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 0.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 1.0f };
        break;
    case Client::CUI_HUD::CH_AUGUSTA:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 1.0f };        
        break;
    case Client::CUI_HUD::CH_GALBRENA:
        keyButtonDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
        keyButtonDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.0f, 0.0f }; 
        break;
    }
}

void CUI_HUD::Update_UI_PlayerEnergyFrame(_float fTimeDelta)
{
	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();

	// 공통 정보 받아옴
	auto& skillSlots = pStatus->Get_Ability(ENUM_CLASS(CH_AUGUSTA))->Get_UISkillSlots();


		_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;



	//tUISlot.



    switch (m_iSelectedCHIndex)
    {
    case Client::CUI_HUD::CH_ROVER:
	{
		m_pUI_Group_Rover->Set_Active(true);     // CH change visible
		m_pUI_Group_Augusta->Set_Active(false);
		m_pUI_Group_Galbrena->Set_Active(false);

		_bool isIn_Rover_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

		if (!isIn_Rover_BurstMode)
		{
			m_pUI_Frame_Rover_Dark->Set_Active(false);
			// Find_ChildObject(L"Frame_Rover")->Set_Active(true); // nullptr
		}
		else if (isIn_Rover_BurstMode)
		{
			m_pUI_Frame_Rover_Dark->Set_Active(true);
			// Find_ChildObject(L"Frame_Rover")->Set_Active(false); // nullptr
		}


	}
        break;
    case Client::CUI_HUD::CH_AUGUSTA:
	{
		m_pUI_Group_Rover->Set_Active(false);
		m_pUI_Group_Augusta->Set_Active(true);
		m_pUI_Group_Galbrena->Set_Active(false);

		if (!isIn_Augusta_AdvUlt)
		{
			m_pUI_Frame_Augusta->Set_Active(true);
			m_pUI_FrameGroup_Augusta_OtherEnergy->Set_Active(true);
			m_pUI_FrameGroup_Augusta_UltMode->Set_Active(false);
		}
		else if (isIn_Augusta_AdvUlt)
		{
			m_pUI_Frame_Augusta->Set_Active(false);
			m_pUI_FrameGroup_Augusta_OtherEnergy->Set_Active(false);
			m_pUI_FrameGroup_Augusta_UltMode->Set_Active(true);
		}
	}

        break;
    case Client::CUI_HUD::CH_GALBRENA:
	{
		m_pUI_Group_Rover->Set_Active(false);
		m_pUI_Group_Augusta->Set_Active(false);
		m_pUI_Group_Galbrena->Set_Active(true);

		_bool isIn_Galbrena_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

		if (!isIn_Galbrena_BurstMode)
		{
			m_pUI_Frame_Galbrena->Set_Active(true);
			m_pUI_Frame_Galbrena_Icon->Set_Active(true);
			m_pUI_FrameGroup_Galbrena_RageMode->Set_Active(false);
		}
		else if (isIn_Galbrena_BurstMode)
		{
			m_pUI_Frame_Galbrena->Set_Active(false);
			m_pUI_Frame_Galbrena_Icon->Set_Active(false);
			m_pUI_FrameGroup_Galbrena_RageMode->Set_Active(true);
		}
	}

        break;
    }


    



    // �Ӽ� ������ ���� ����
    vector<CCustom_UI*> pElementIcons = {
        m_pUI_Icon_ElementDark,
        m_pUI_Icon_ElementThunder,
        m_pUI_Icon_ElementFire 
    };
    CCustom_UI* pElementTargetUI = pElementIcons[m_iSelectedCHIndex];

    vector<_float4x4> vecElementVariantMat = { _float4x4() };
    *reinterpret_cast<_float4*>(&vecElementVariantMat[0]._11) = m_arrPlayerSymbolicColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementVariantMat[0]._21) = static_cast<_float>(true);

    CCustom_UI::VARIANTREADY_UI_DESC tElementVariantDesc = {
        vecElementVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLEMASK),
        true
    };

    pElementTargetUI->Set_VariantUIDesc(tElementVariantDesc);

    static _float fElementAmounts[CH_END] = { 0.f, 0.f ,0.f };
    static _float fMaxElementAmounts[CH_END] = {100.f, 100.f, 100.f};


	fElementAmounts[m_iSelectedCHIndex] = m_pAbility->Get_Harmony();
    //fElementAmounts[0] = (fElementAmounts[0] >= 100)? 0 : fElementAmounts[0] + 2.f  * 30.f * fTimeDelta;
    //fElementAmounts[1] = (fElementAmounts[1] >= 100)? 0 : fElementAmounts[1] + 1.5f * 30.f * fTimeDelta;
    //fElementAmounts[2] = (fElementAmounts[2] >= 100)? 0 : fElementAmounts[2] + 1.f  * 30.f * fTimeDelta;

    

    CCustom_UI* pElementGuageUI = m_pUI_Icon_ElementGuage;
    //auto elementGuageDesc = pElementGuageUI->Get_UIDesc();

    vector<_float4x4> vecElementGuageVariantMat = { _float4x4() };
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._11) = (1.f - fElementAmounts[m_iSelectedCHIndex] / fMaxElementAmounts[m_iSelectedCHIndex]);
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._12) = 0.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._13) = 1.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._14) = static_cast<_float>(true);
    *reinterpret_cast<_float4*>(&vecElementGuageVariantMat[0]._21) = m_arrPlayerSymbolicColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._31) = 90.f;

	*reinterpret_cast<_float*> (&vecElementGuageVariantMat[0]._32) = static_cast<_float>(fElementAmounts[m_iSelectedCHIndex] == 100.f);	// isUseNoise
	*reinterpret_cast<_float*> (&vecElementGuageVariantMat[0]._33) = m_fElapsedTime;							// Elapsed Time
	*reinterpret_cast<_float*> (&vecElementGuageVariantMat[0]._34) = 0.2f;							// UV Scroll Speed
	*reinterpret_cast<_float4*>(&vecElementGuageVariantMat[0]._41) = m_arrPlayerAdvSymbolicColors[m_iSelectedCHIndex];	// Mask Color



    CCustom_UI::VARIANTREADY_UI_DESC tElementAmountVariantDesc = {
        vecElementGuageVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
        true
    };

    pElementGuageUI->Set_VariantUIDesc(tElementAmountVariantDesc);

}

void CUI_HUD::Update_UI_Icon_HarmonyReady(_float fTimeDelta)
{
	CCustom_UI* pTargetUI	= m_pUI_Icon_HarmonyIndicator;
	CCustom_UI* pTargetBGUI = m_pUI_Icon_HarmonyIndicatorBG;

	_bool isHarmonyGuageFull = m_pAbility->Get_Harmony() == 100.f;

	if (!isHarmonyGuageFull)
	{
		pTargetUI->SetActivate(false);
		pTargetBGUI->SetActivate(false);
		return;
	}

	pTargetUI->SetActivate(true);
	pTargetBGUI->SetActivate(true);



	auto& targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;
	targetInstDesc.resize(3);
	auto& targetBGDesc = pTargetBGUI->Get_UIDesc();
	auto& targetBGInstDesc = targetBGDesc.vecInstanceDescs;
	targetBGInstDesc.resize(3);

	static vector<_float4x4> vecVariantMat = { };
	vecVariantMat.resize(3);

	
	// ===== Variant Edit.. ===== 

	static _float fDistortStrength = 0.f;
	static _float fLateDistortStrength = 0.f;

	static _float fDistortMin = 0.25f;
	static _float fDistortMax = 0.8f;
	static _float fFollowStrength = 0.01f;

	if (m_pGameInstance->Rand_Normal() <= 0.1f)
	{
		fDistortStrength += 0.16f;
	}
	fDistortStrength = fDistortStrength - 0.02f;
	fDistortStrength = clamp(fDistortStrength, fDistortMin, fDistortMax);

	if (fDistortStrength > fLateDistortStrength)	fLateDistortStrength += fFollowStrength * 2.f;
	if (fDistortStrength < fLateDistortStrength)	fLateDistortStrength -= fFollowStrength;
	fLateDistortStrength = 0.5f;// clamp(fLateDistortStrength, fDistortMin, fDistortMax);

	_float fUVRotateSpeed = -60.f / 60.f;

	for (auto& variantMat : vecVariantMat)		// Outline
	{
		_float4 vDestColor = _float4(1.f, 1.f, 1.f, 1.f);
		*reinterpret_cast<_float4*>(&variantMat._11) = vDestColor;					// dest color
		*reinterpret_cast<_float*>(&variantMat._21) = static_cast<_float>(true);	// is Distort On?
		*reinterpret_cast<_float*>(&variantMat._22) = m_fElapsedTime;				// ElapsedTime. for transforming UV
		*reinterpret_cast<_float*>(&variantMat._23) = fLateDistortStrength;			// distort strength.
		*reinterpret_cast<_float*>(&variantMat._24) = fUVRotateSpeed;				// rotate speed (deg per sec).
		*reinterpret_cast<_float*>(&variantMat._31) = 1.0f;							// fAlphaMultiplier.
		*reinterpret_cast<_float*>(&variantMat._32) = static_cast<_float>(false);	// isDisableNormalize (deg per sec).
	}

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_WAVECIRCLE),
		true
	};

	pTargetUI->Set_VariantUIDesc(tVariantDesc);

	for (auto& variantMat : vecVariantMat)		// Background Circle
	{
		_float4 vDestColor = _float4(1.f, 1.f, 1.f, 0.2f);
		*reinterpret_cast<_float4*>(&variantMat._11) = vDestColor;					// dest color
		*reinterpret_cast<_float*>(&variantMat._21) = static_cast<_float>(true);	// is Distort On?
		*reinterpret_cast<_float*>(&variantMat._22) = m_fElapsedTime;				// ElapsedTime. for transforming UV
		*reinterpret_cast<_float*>(&variantMat._23) = fLateDistortStrength;			// distort strength.
		*reinterpret_cast<_float*>(&variantMat._24) = fUVRotateSpeed;				// rotate speed (deg per sec).
		*reinterpret_cast<_float*>(&variantMat._31) = 1.0f;							// fAlphaMultiplier.
		*reinterpret_cast<_float*>(&variantMat._32) = static_cast<_float>(true);	// isDisableNormalize (deg per sec).
	}

	tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_WAVECIRCLE),
		true
	};

	pTargetBGUI->Set_VariantUIDesc(tVariantDesc);

	// ==============================

	// disable when current character.
	for (_uint i = 0; i < CH_END; i++)
	{
		targetInstDesc[i].vClipTexcoordX	= (m_iSelectedCHIndex == i)? _float2( 0.0f, 0.0f ) : _float2( 0.0f, 1.0f );
		targetBGInstDesc[i].vClipTexcoordX	= (m_iSelectedCHIndex == i)? _float2( 0.0f, 0.0f ) : _float2( 0.0f, 1.0f );		
	}





}

void CUI_HUD::Update_UI_PlayerEnergyBar(_float fTimeDelta)
{
	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();

	// Load Status..
	m_iSelectedCHIndex = pStatus->Get_CurrentCharIndex();
	//auto& tUISlot = pStatus->Get_Ability(iSelectedCHIndex)->Get_UISkillSlots();
	pStatus->Get_CostRatio(m_iSelectedCHIndex, COST_TYPE::COST1);

	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();
	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);


	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;

    //m_fPlayerEnergy;
    //m_fPlayerMaxEnergy;
	//
    //_float fCurPlayerEnergy         = m_fPlayerEnergy[m_iSelectedCHIndex];
    //_float fCurPlayerMaxEnergy      = m_fPlayerMaxEnergy[m_iSelectedCHIndex];
	//
	//_float fCurPlayerEnergy		=
	//_float fCurPlayerMaxEnergy	=
	    
    static _float fGalbEchoEnergy   = 0.f;
    const _float fGalbMaxEchoEnergy = 50.f;

    _float fCurPlayerEnergyRatio    = pStatus->Get_CostRatio(m_iSelectedCHIndex, COST_TYPE::COST1); //fCurPlayerEnergy / fCurPlayerMaxEnergy;
    

    _float4     vSingleColor[2] = {};   // for Gradiant
    _float4     vExtraColor[2] = {};    // for Galbrena. �Ӹ��� ȥ�� ����ȸ�ο� �÷� �¿��� �ΰ���
    _bool       isSingleVisible = {};   //
    //_float      fSingleHeight = {};   // vSpectrumHeights, vBackSpectrumHeights

    _float4     vBackColor[2] = {};
    _bool       isBackVisible = {};
    _float      fBackHeight = {};

    const auto targetUI = m_pUI_EnergyInstItems;


    //// [Galbrena]
    //if (m_pGameInstance->Get_DIKeyState(DIK_Y) == KEYSTATE::DOWN)
    //{
    //    if (fGalbEchoEnergy == fGalbMaxEchoEnergy) fGalbEchoEnergy = 0;
    //    fGalbEchoEnergy += 10;
    //    if (fGalbEchoEnergy >= fGalbMaxEchoEnergy) fGalbEchoEnergy = fGalbMaxEchoEnergy;
    //}



    enum HUD_PLAYER_ENERGYBAR { VALUE, TARGET, END };
    enum HUD_PLAYER_ENCOLOR {
        ENCL_ROVER_NORMAL, 
        ENCL_AUGUSTA_NORMAL,
        ENCL_AUGUSTA_ULT,
        ENCL_GALBRENA_NORMAL_L,
        ENCL_GALBRENA_NORMAL_R,
        ENCL_GALBRENA_ULT,

        ENCL_STATIC,
        ENCL_END
    };
    const _uint iNumSpectrums = 41;					// draws 41 instance at once.

    vector<array<_float4, 2>> vecColorPreset;       // { start color (bottom), end color (top) }
    vecColorPreset.resize(ENCL_END);

    vecColorPreset[ENCL_ROVER_NORMAL]       = { _float4{0.961f, 0.192f, 0.502f, 1.f}, _float4{0.961f, 0.192f, 0.502f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_NORMAL]     = { _float4{0.769f, 0.631f, 0.933f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_AUGUSTA_ULT]        = { _float4{1.000f, 0.953f, 0.722f, 1.f}, _float4{0.769f, 0.631f, 0.933f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_L]  = { _float4{0.894f, 0.573f, 0.525f, 1.f}, _float4{0.922f, 0.490f, 0.486f, .8f} };
    vecColorPreset[ENCL_GALBRENA_NORMAL_R]  = { _float4{0.682f, 0.769f, 0.980f, 1.f}, _float4{0.553f, 0.557f, 0.878f, .8f} };
    vecColorPreset[ENCL_GALBRENA_ULT]       = { _float4{0.482f, 0.412f, 0.878f, 1.f}, _float4{0.867f, 0.824f, 0.957f, .8f} };

    vecColorPreset[ENCL_STATIC]             = { _float4(1.f, .8f, .8f, .35f), _float4(1.f, .8f, .8f, .35f) }; // ��á�� �� ����



    static vector<_float> vSpectrumHeights[END] = {};		// front instances height
    vSpectrumHeights[VALUE].resize(iNumSpectrums);
    vSpectrumHeights[TARGET].resize(iNumSpectrums);
    static vector<_float> vBackSpectrumHeights[END] = {};	// back instances height
    vBackSpectrumHeights[VALUE].resize(iNumSpectrums);
    vBackSpectrumHeights[TARGET].resize(iNumSpectrums);

    array<_bool, iNumSpectrums> arrIsVisible = {};							// visible mask for front & back indices information
    array<_bool, iNumSpectrums> arrIsVisibleStatic = {};					// visible mask for static indices information





    switch (m_iSelectedCHIndex)
    {
    case CH_ROVER:     
        {
			_bool isIn_Rover_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

            if      (!isIn_Rover_BurstMode)     /* Normal */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1];

                //arrIsVisible.assign(arrIsVisible.size(), true);                    // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(arrIsVisible.begin(), arrIsVisible.end() - (41 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
            }
            else if (isIn_Rover_BurstMode)     /* Ult    */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1]; 

                //arrIsVisible.assign(arrIsVisible.size(), true);                     // isvisible
                fill(arrIsVisible.begin() + 16, arrIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 16.f);      // applying player energy
                fill(arrIsVisible.begin() + (16 - iVisibleBarRange), arrIsVisible.end() - 25, true);
                fill(arrIsVisible.end() - 16, arrIsVisible.end() - (16 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
                fill(arrIsVisibleStatic.begin() + 16, arrIsVisibleStatic.end() - 16, false);
            }
        }break;
    case CH_AUGUSTA:   
        {
            if  (!isIn_Augusta_AdvUlt &&
				 (fCurPlayerEnergyRatio != 1.f) )
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_NORMAL][0];       // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_NORMAL][1]; 

                //arrIsVisible.assign(arrIsVisible.size(), true);                     // isvisible
                fill(arrIsVisible.begin() + 16, arrIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(arrIsVisible.begin(), 
                    (iVisibleBarRange > 16)? arrIsVisible.begin() + 16 : arrIsVisible.begin() + iVisibleBarRange, true);
                fill(arrIsVisible.end() - 16,
                    arrIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
                fill(arrIsVisibleStatic.begin() + 16, arrIsVisibleStatic.end() - 16, false);
            }
            else if(!isIn_Augusta_AdvUlt &&
					(fCurPlayerEnergyRatio == 1.f))     /* Ult?   */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_ULT][0];          // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_ULT][1]; 

                //arrIsVisible.assign(arrIsVisible.size(), true);                     // isvisible
                fill(arrIsVisible.begin() + 16, arrIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(arrIsVisible.begin(),
                    (iVisibleBarRange > 16) ? arrIsVisible.begin() + 16 : arrIsVisible.begin() + iVisibleBarRange, true);
                fill(arrIsVisible.end() - 16,
                    arrIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
                fill(arrIsVisibleStatic.begin() + 16, arrIsVisibleStatic.end() - 16, false);
            }
            else if (isIn_Augusta_AdvUlt)
            {
                fill(arrIsVisibleStatic.begin(), arrIsVisibleStatic.end(), false);
            }
        }break;
    case CH_GALBRENA:  
        {
			_bool isIn_Galbrena_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

            if  (	!isIn_Galbrena_BurstMode	)     /* Normal */
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][0];   // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][1];  
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                fill(arrIsVisible.begin() + 11, arrIsVisible.end() - 26, false);    // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 26.f);      // applying player energy
                fill(arrIsVisible.end() - 26, arrIsVisible.end() - 26 + iVisibleBarRange, true);

                // [Galbrena] applying echo energy
                _uint iVisibleBarRange_Echo = static_cast<_uint>(fGalbEchoEnergy / fGalbMaxEchoEnergy * 11.f);
                fill(arrIsVisible.begin() + 11 - iVisibleBarRange_Echo, arrIsVisible.begin() + 11, true);


                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
                fill(arrIsVisibleStatic.begin() + 11, arrIsVisibleStatic.end() - 26, false);
            }
            else if(isIn_Galbrena_BurstMode		)     /* Burst   */
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_ULT][0];        // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_ULT][1];       
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                //arrIsVisible.assign(arrIsVisible.size(), true);                     // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(arrIsVisible.begin(), arrIsVisible.begin() + iVisibleBarRange, true);

                // applying player energy - not filled
                for (_uint i = 0; i < arrIsVisible.size(); i++)
                    arrIsVisibleStatic[i] = !arrIsVisible[i];
            }
        }break;
    }





    vBackColor[0] = vSingleColor[0];
    vBackColor[1] = vSingleColor[1];
    vBackColor[0].w *= 0.4f;
    vBackColor[1].w *= 0.4f;

    for (_uint i = 0; i < iNumSpectrums; ++i)
    {
        vSpectrumHeights[VALUE][i] += (vSpectrumHeights[TARGET][i] - vSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;
        vBackSpectrumHeights[VALUE][i] += (vBackSpectrumHeights[TARGET][i] - vBackSpectrumHeights[VALUE][i]) * fTimeDelta * 5.0f;

        if (m_pGameInstance->Rand(0.f, 100.f) < 3.f)
            vSpectrumHeights[TARGET][i] = m_pGameInstance->Rand(1.f, 2.f);
        if (m_pGameInstance->Rand(0.f, 100.f) < 3.f)
            vBackSpectrumHeights[TARGET][i] = m_pGameInstance->Rand(1.f, 4.f);
    }


    array<_float4x4, 41> arrVariantMat = {};
	array<_float4x4, 41> arrVariantBackMat = {};
	array<_float4x4, 41> arrVariantStaticMat = {};

    for (uint i = 0; i < arrVariantMat.size(); i++)         // front spectrum.
    {
        *reinterpret_cast<_float4*>(&arrVariantMat[i]._11) = vSingleColor[0];
        *reinterpret_cast<_float4*>(&arrVariantMat[i]._21) = vSingleColor[1];
        arrVariantMat[i]._31 = static_cast<_float>(arrIsVisible[i]);
        arrVariantMat[i]._32 = vSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < arrVariantBackMat.size(); i++)     // back spectrum. 
    {
        *reinterpret_cast<_float4*>(&arrVariantBackMat[i]._11) = vBackColor[0];
        *reinterpret_cast<_float4*>(&arrVariantBackMat[i]._21) = vBackColor[1];
		arrVariantBackMat[i]._31 = static_cast<_float>(arrIsVisible[i]);
        //vecVariantBackMat[i]._31 = false;
		arrVariantBackMat[i]._32 = vBackSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < arrVariantStaticMat.size(); i++)   // static spectrum.
    {
        *reinterpret_cast<_float4*>(&arrVariantStaticMat[i]._11) = vecColorPreset[ENCL_STATIC][0];
        *reinterpret_cast<_float4*>(&arrVariantStaticMat[i]._21) = vecColorPreset[ENCL_STATIC][1];
		arrVariantStaticMat[i]._31 = static_cast<_float>(arrIsVisibleStatic[i]);
        //vecVariantStaticMat[i]._31 = false;                                                
		arrVariantStaticMat[i]._32 = 1.f;
    }


	// galbrena energy bar
    if (m_iSelectedCHIndex == CH_GALBRENA)
    {
		_bool isIn_Galbrena_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

        if (!isIn_Galbrena_BurstMode)
        {
            for (uint i = 0; i < arrVariantMat.size(); i++)
                if (i >= 15)
                {
                    *reinterpret_cast<_float4*>(&arrVariantMat[i]._11) = vExtraColor[0];
                    *reinterpret_cast<_float4*>(&arrVariantMat[i]._21) = vExtraColor[1];
                }
            for (uint i = 0; i < arrVariantBackMat.size(); i++)
                if (i >= 15)
                {
                    _float4 vExtraBackColor[2];
                    vExtraBackColor[0] = vExtraColor[0];   vExtraBackColor[0].w = 0.5f;
                    vExtraBackColor[1] = vExtraColor[1];   vExtraBackColor[1].w = 0.5f;
                    *reinterpret_cast<_float4*>(&arrVariantMat[i]._11) = vExtraBackColor[0];
                    *reinterpret_cast<_float4*>(&arrVariantMat[i]._21) = vExtraBackColor[1];
                }   
        }
    }

	static vector<_float4x4> vecResultVariantMat = {};
	vecResultVariantMat.resize(123);

	for (_uint i = 0; i < 41; i++)
		vecResultVariantMat[i] = arrVariantBackMat[i];
	for (_uint i = 41; i < 82; i++)
		vecResultVariantMat[i] = arrVariantMat[i - 41];
	for (_uint i = 82; i < 123; i++)
		vecResultVariantMat[i] = arrVariantStaticMat[i - 82];

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecResultVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_TRANSMIT),
        true
    };
    
    targetUI->Set_VariantUIDesc(tVariantDesc);  // draws 123 instances in once.
}

void CUI_HUD::Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta)
{
	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();

	// 공통 정보 받아옴
	m_iSelectedCHIndex = pStatus->Get_CurrentCharIndex();
    if (m_iSelectedCHIndex != CH_AUGUSTA)
        return;

    _uint	iSwordEnergy	= static_cast<_uint>(pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) * 2.f);            // mAX = 2

    //static _float	fPointEnergy		= 0,f;
    //const _float	fMaxPointEnergy		= 100.f;
	//
    //static _float	fUltBladeEnergy		= 0.f;
    //const _float	fMaxUltBladeEnergy	= 100.;

	_float fPointEnergyRatio	= pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST2);
	_float fUltBladeEnergyRatio = pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST4);


    CCustom_UI* pBladeUI    = m_pUI_Frame_Augusta_Inst_SwordEnergy;
    CCustom_UI* pPointUI    = m_pUI_Frame_Augusta_Inst_CenterPointEnergy;
    CCustom_UI* pUltBladeUI = m_pUI_Frame_Augusta_Inst_UltModeEnergy;



	auto& UISkillSlot = pStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	


    //// Į �ڿ�
    //if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
    //{
    //    iSwordEnergy++;
    //    if (iSwordEnergy > 2) iSwordEnergy = 0;
    //}
	//
    //// ���� �ڿ�
    //if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
    //{
    //    if (fPointEnergy == 100) fPointEnergy = 0;
    //    else
    //    {
    //        fPointEnergy += m_pGameInstance->Rand(10.f, 40.f);
    //        if (fPointEnergy >= 100) fPointEnergy = 100;
    //    }
    //}
	//
    //// �ñر� �ڿ�
    //if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
    //{
    //    if (fUltBladeEnergy == 100) fUltBladeEnergy = 0;
    //    else
    //    {
    //        //fUltBladeEnergy += m_pGameInstance->Rand(10.f, 40.f);
    //        fUltBladeEnergy += 100.f / 7.f;
    //        if (fUltBladeEnergy >= 99.9f) fUltBladeEnergy = 100;
    //    }
    //}



	// Control Active
	auto& skillSlots = pStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();
	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;

    if	(!isIn_Augusta_AdvUlt)
    {
        pBladeUI    ->Set_Active(true);
        pPointUI    ->Set_Active(true);
        pUltBladeUI ->Set_Active(false);
    }
    else
    {
        pBladeUI    ->Set_Active(false);
        pPointUI    ->Set_Active(false);
        pUltBladeUI ->Set_Active(true);
    }




    auto& bladeDesc = pBladeUI->Get_UIDesc();

    auto& ultBladeDesc = pUltBladeUI->Get_UIDesc();
    
    // Į �ڿ�
    // �׳� ������ ���� ������ ���� �����ֱ�. alpha pass �̿�.
    switch (iSwordEnergy)
    {
    case 0:     
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 0.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
    break;
    case 1 :    
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 0.0f };
    break;
    case 2 :    
        bladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, 1.0f };
        bladeDesc.vecInstanceDescs[1].vClipTexcoordX = { 0.0f, 1.0f };
    break;
    }



    vector<_float4x4> vecPointVariantMat = { _float4x4() };

    vecPointVariantMat[0].m[0][0] = 1 - (fPointEnergyRatio);
    vecPointVariantMat[0].m[0][1] = 0.f;
    vecPointVariantMat[0].m[0][2] = 1.f;
    vecPointVariantMat[0].m[0][3] = static_cast<_float>(true);
    *reinterpret_cast<_float4*>(&vecPointVariantMat[0].m[1][0]) = _float4(1.0f, 0.941f, 0.729f, 1.f);

    CCustom_UI::VARIANTREADY_UI_DESC tPointVariantDesc = {
        vecPointVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
        true
    };
    pPointUI->Set_VariantUIDesc(tPointVariantDesc);




    // �ñر� �ڿ�
    // ���� ���̴��� �������� �� Ȱ��. alpha pass �̿�.
    _float ultRatio = fUltBladeEnergyRatio;
    static _float fPreUltRatio = 0.f;

    if (ultRatio > fPreUltRatio)	fPreUltRatio += fTimeDelta * 1.5f;
    if (ultRatio <= fPreUltRatio)	fPreUltRatio = ultRatio;

    ultBladeDesc.vecInstanceDescs[0].vClipTexcoordX = { 0.0f, fPreUltRatio };

}

void CUI_HUD::Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta)
{
    // ���� ���ڹٴ� Update_UI_PlayerEnergyBar ���� ó��
    // 
    // ��ȭ������ �� �ϴ� ������ �� ����


    // ��ȭ ���� ����
    //if (m_iSelectedCHIndex != CH_GALBRENA)
    //    return;
	//
    //if (m_iEnergyBarMode != 1)
    //    return;










}


void CUI_HUD::Update_Icon_Rover(const vector<UISKILL_SLOT>& skillSlots)
{

	auto& roverUIDesc = m_pUI_Skill[CH_ROVER]->Get_UIDesc();

	UI_ROVER_STATE eState_Rover_E = static_cast<UI_ROVER_STATE>(skillSlots[CAbility::KEY_E].iStateType);
	UI_ROVER_STATE eState_Rover_R = static_cast<UI_ROVER_STATE>(skillSlots[CAbility::KEY_R].iStateType);


	_bool isIn_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));


	// ==============================
	// [Get] [E] Button Slot State
	// ==============================

	switch (eState_Rover_E)
	{
	case Client::UI_ROVER_STATE::E_BURST_READY:
		// Augusta Normal - E Combo Start
		roverUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Rover_E_Burst"][0];
		roverUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Rover_E_Burst"][1];
		break;
	case Client::UI_ROVER_STATE::E_DEFAULT_READY:
	default:
		roverUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Rover_E"][0];
		roverUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Rover_E"][1];
		break;
	}


	// ==============================
	// [Get] [R] Button Slot State
	// ==============================

	switch (eState_Rover_R)
	{
	case Client::UI_ROVER_STATE::R_READY:
	default:
		roverUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Rover_R"][0];
		roverUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Rover_R"][1];
		break;
	}

	//m_pUI_Skill[CH_ROVER]->Set_UIDesc(roverUIDesc);

}



void CUI_HUD::Update_Icon_Augusta(const vector<UISKILL_SLOT>& skillSlots)
{
	auto& augustaUIDesc = m_pUI_Skill[CH_AUGUSTA]->Get_UIDesc();

	UI_AUGUSTA_STATE eState_Augusta_E = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_E].iStateType);
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);
	UI_AUGUSTA_STATE eState_Augusta_LB = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType);



	// if True, In Advanced Ult Mode
	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	
	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;
	// ㄴ 이걸 기점으로 아이콘 조건 주어야 할 듯. 지금 궁 사용 상태에서 안보임

	_bool isAvailable_Griffon = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));
	_bool isAvailable_Rise = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));



	// ==============================
	// [Get] [E] Button Slot State
	// ==============================

	switch (eState_Augusta_E)
	{


	case Client::UI_AUGUSTA_STATE::E_GRIFFON_READY:
		// Augusta Normal - E Combo Start
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_E_GriffonReady"][0];
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_E_GriffonReady"][1];
		break;
	case Client::UI_AUGUSTA_STATE::E_RISE_READY:
		// Augusta Normal - E Combo 2
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_E_RiseReady"][0];
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_E_RiseReady"][1];
		break;
	case Client::UI_AUGUSTA_STATE::E_DEFAULT_READY:
	default:
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_E"][0];
		augustaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_E"][1];
		break;
	}


	// ==============================
	// [Get] [R] Button Slot State
	// ==============================

	// R Button Ctrl..
	switch (eState_Augusta_R)
	{
	case Client::UI_AUGUSTA_STATE::R_SWORD_READY:
		// Augusta Normal - Ult Ready
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R_Ready"][0];
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R_Ready"][1];
		break;
	case Client::UI_AUGUSTA_STATE::R_SWORD_ULTI_READY:
		// Augusta in Ult, Useable Final Ult
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R_Enforce"][0];
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R_Enforce"][1];
		break;
	default:
		if (isIn_AdvUltMode)
		{	// Autusta in Ult, Not Useable Final Ult.
			augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R_Enforce"][0];
			augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R_Enforce"][1];
			break;
		}
		// Augusta Normal
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R"][0];
		augustaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R"][1];
		break;

	}

	// E Button Ctrl.. via R Button State
	switch (eState_Augusta_R)
	{
	case Client::UI_AUGUSTA_STATE::R_SWORD_READY: break;
	case Client::UI_AUGUSTA_STATE::R_SWORD_ULTI_READY:
		// Augusta in Ult, Useable Final Ult
		augustaUIDesc.vecInstanceDescs[BTN_E].vClipTexcoordX = { 0.f, 0.f };
		break;
	default:
		if (isIn_AdvUltMode)
		{	// Autusta in Ult, Not Useable Final Ult.
			augustaUIDesc.vecInstanceDescs[BTN_E].vClipTexcoordX = { 0.f, 0.f };
			break;
		}
		augustaUIDesc.vecInstanceDescs[BTN_E].vClipTexcoordX = { 0.f, 1.f };
		break;
	}


	// ==============================
	// [Get] [LB] Button Slot State
	// ==============================
	
	if (isIn_Augusta_AdvUlt)
	{
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_LB_Burst"][0];
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_LB_Burst"][1];
		augustaUIDesc.vecInstanceDescs[BTN_LB].vClipTexcoordX = { 0.0f, 1.0f };
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstTrans.x = 750.f;
	}
	else if (eState_Augusta_LB == UI_AUGUSTA_STATE::LB_STRONG_READY)
	{
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_LB_StrongATK"][0];
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_LB_StrongATK"][1];
		augustaUIDesc.vecInstanceDescs[BTN_LB].vClipTexcoordX = { 0.0f, 1.0f };
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstTrans.x = 450.f;
	}
	else
	{
		augustaUIDesc.vecInstanceDescs[BTN_LB].vClipTexcoordX = { 0.0f, 0.0f };
		augustaUIDesc.vecInstanceDescs[BTN_LB].vSInstTrans.x = 450.f;
	}



	//m_pUI_Skill[CH_AUGUSTA]->Set_UIDesc(augustaUIDesc);

}

void CUI_HUD::Update_Icon_Galbrena(const vector<UISKILL_SLOT>& skillSlots)
{
	auto& galbrenaUIDesc = m_pUI_Skill[CH_GALBRENA]->Get_UIDesc();

	UI_GALBRENA_STATE eState_Galbrena_E = static_cast<UI_GALBRENA_STATE>(skillSlots[CAbility::KEY_E].iStateType);
	UI_GALBRENA_STATE eState_Galbrena_R = static_cast<UI_GALBRENA_STATE>(skillSlots[CAbility::KEY_R].iStateType);
	UI_GALBRENA_STATE eState_Galbrena_LB = static_cast<UI_GALBRENA_STATE>(skillSlots[CAbility::KEY_LB].iStateType);

	_bool isIn_Galbrena_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));


	// ==============================
	// [Get] [E] Button Slot State
	// ==============================

	switch (eState_Galbrena_E)
	{
	case Client::UI_GALBRENA_STATE::E_DEFAULT_READY:
	case Client::UI_GALBRENA_STATE::DEFAULT:
	default:
		galbrenaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_E"][0];
		galbrenaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_E"][1];
		break;
	case Client::UI_GALBRENA_STATE::E_BURST_READY:
		galbrenaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_E_BurstOn"][0];
		galbrenaUIDesc.vecInstanceDescs[BTN_E].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_E_BurstOn"][1];
		break;
	}

	// ==============================
	// [Get] [R] Button Slot State
	// ==============================

	// R Button Ctrl..
	switch (eState_Galbrena_R)
	{
	case Client::UI_GALBRENA_STATE::DEFAULT:
	default:
		galbrenaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_R"][0];
		galbrenaUIDesc.vecInstanceDescs[BTN_R].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_R"][1];
		break;
	}

	// ==============================
	// [Get] [LB] Button Slot State
	// ==============================
	
	// 버스트모드인지를 확인 가능한 무언가가 있어야 할 듯
	
	if (isIn_Galbrena_BurstMode)
	{
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][0];
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][1];
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vClipTexcoordX = { 0.f, 1.f };
	}
	else
	{
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][0];
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][1];
		galbrenaUIDesc.vecInstanceDescs[BTN_LB].vClipTexcoordX = { 0.f, 0.f };
	}
	


	//m_pUI_Skill[CH_GALBRENA]->Set_UIDesc(galbrenaUIDesc);
}



array<_float2, 2> CUI_HUD::Calc_SpriteSpace(_uint iIndexX, _uint iIndexY, array<_uint, 2> iNumMax, _float2 vSpriteSize)
{
	_float2 vXSpace, vYSpace;
	_float fXNumSize, fYNumSize;

	fXNumSize = (_float)(vSpriteSize.x / iNumMax[0]);
	fYNumSize = (_float)(vSpriteSize.y / iNumMax[1]);

	vXSpace = { fXNumSize * iIndexX, fXNumSize * (iIndexX + 1) };
	vYSpace = { fYNumSize * iIndexY, fYNumSize * (iIndexY + 1) };

	array<_float2, 2> output = { vXSpace, vYSpace };

	return output; // x 범위, y 범위 반환
}


CUI_HUD* CUI_HUD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_HUD* pInstance = new CUI_HUD(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_HUD");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_HUD::Clone(void* pArg)
{
    CUI_HUD* pInstance = new CUI_HUD(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_HUD");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_HUD::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_HUD");

    for (auto& child : m_vecChildObjects)
        Safe_Release(child);

    __super::Free();
}
