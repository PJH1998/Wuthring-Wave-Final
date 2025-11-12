#include "ClientPch.h"
#include "Animator_UI.h"
#include "UI_HUD.h"

#include "Ability.h"
#include "GameSystem.h"
#include "Player.h"
#include "PlayerStatus.h"

//#define KSTA_UI_COOLDOWNTEST
//#define KSTA_UI_HPBARTEST
//#define KSTA_UI_HPBARBOSSTEST
//#define KSTA_UI_ENERGYBARTEST


CUI_HUD::CUI_HUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
	, m_pGameSystem ( CGameSystem::GetInstance() )
{
}

CUI_HUD::CUI_HUD(const CUI_HUD& Prototype)
    :CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
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

    // Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
    _wstring strFilePath = 
        L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json";
    Load_ChildObjects(strFilePath);

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
        L"../../Client/Bin/Resource/UI/FJson/UIAnim/BossStatus_FadeIn.json"
    };
    Load_Animations(vecAnimFilePaths);


	m_pPlayerStatus = m_pGameSystem->Get_PlayerStatus();
	

    return S_OK;
}

void CUI_HUD::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_HUD::Update(_float fTimeDelta)
{
	m_iSelectedCHIndex = m_pPlayerStatus->Get_CurrentCharIndex();
	if (m_iSelectedCHIndex == 2)
		m_iSelectedCHIndex = 0;


	m_pAbility = m_pPlayerStatus->Get_Ability(m_iSelectedCHIndex);

	Ready_Presets();

	Update_UI_SkillSection(fTimeDelta);
	Update_UI_SkillSection_BG(fTimeDelta);
	Update_UI_SkillFeedback_Trigger(fTimeDelta);
	Update_UI_SkillSection_OnFeedback(fTimeDelta);
	Update_UI_PlayerHPBar(fTimeDelta);
	Update_UI_BossHPBar(fTimeDelta);
	Update_UI_KeyGuide(fTimeDelta);

	Update_UI_PlayerEnergyFrame(fTimeDelta);
	Update_UI_PlayerEnergyBar(fTimeDelta);
	Update_UI_PlayerEnergyBar_Augusta(fTimeDelta);
	Update_UI_PlayerEnergyBar_Galbrena(fTimeDelta);

	Update_CombinedMatrix();
	Update_CombinedDesc();

    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_HUD::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_HUD::Render()
{
    //__super::Render();                      // Nothing. �����׷� �߰��� �� ���� ���������� �˾Ƽ� �ڽĵ���� Render ����
}

HRESULT CUI_HUD::Ready_Components(void* pArg)
{
    return S_OK;
}

HRESULT CUI_HUD::Ready_Presets()
{
	// ========== Image Sizes ==========

	// 재정립 필요

	array<_uint, 2> iImgSize_Rover = { 14, 1 };
	array<_uint, 2> iImgSize_Augusta = { 7, 2 };
	array<_uint, 2> iImgSize_Galbrena = { 14, 1 };

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

	return S_OK;
}

void CUI_HUD::Update_UI_SkillSection(_float fTimeDelta)
{
	// Temp assumed Value
	_float fMaxChangeCD[3] = { 2.f, 2.f, 2.f};

	// ==============================

	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();

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

	// UI Load
    CCustom_UI* pSkillUI[CH_END] = {                  // Skill Indicator UI per Character.
        Find_ChildObject(L"Skill_Rover"),
        Find_ChildObject(L"Skill_Augusta"),
        Find_ChildObject(L"Skill_Galbrena")
    };

    CCustom_UI* pChangeUI[CH_END] = {                 // PartyFrame UI per Character.
        Find_ChildObject(L"Icon_Rover"),
        Find_ChildObject(L"Icon_Augusta"),
        Find_ChildObject(L"Icon_Galbrena")
    };

	// ==============================





    switch (m_iSelectedCHIndex)
    {
    case CH_ROVER:
        pSkillUI[0]->Set_Active(true);
        pSkillUI[1]->Set_Active(false);
        pSkillUI[2]->Set_Active(false);
        break;
    case CH_AUGUSTA:
        pSkillUI[0]->Set_Active(false);
        pSkillUI[1]->Set_Active(true);
        pSkillUI[2]->Set_Active(false);
        break;
    case CH_GALBRENA:
        pSkillUI[0]->Set_Active(false);
        pSkillUI[1]->Set_Active(false);
        pSkillUI[2]->Set_Active(true);
        break;
    }

	
	for (_uint i = 0; i < CH_END; i++)                                  // Apply cooldown values
	{
		auto targetUI = pSkillUI[i];

		// 스킬 ui 인스턴스 갯수는 캐릭터마다 다름. 이에 따라 인스턴스 갯수만큼 리사이징 및 할당 
		vector<_float4x4> vecVariantMat = {/* _float4x4() , _float4x4() */};	

		_uint iTargetNumInstance = targetUI->Get_UIDesc().vecInstanceDescs.size();
		vecVariantMat.resize(iTargetNumInstance);

        vecVariantMat[0].m[0][0] = fBasicSkillCD[i][SK_E] / fBasicSkillMaxCD[i][SK_E];
        vecVariantMat[0].m[0][1] = 0.5f;
        vecVariantMat[0].m[0][2] = 0.95f;

        vecVariantMat[1].m[0][0] = fBasicSkillCD[i][SK_R] / fBasicSkillMaxCD[i][SK_R];
        vecVariantMat[1].m[0][1] = 0.5f;
        vecVariantMat[1].m[0][2] = 0.95f;

		if (vecVariantMat.size() >= 3)
		{
			vecVariantMat[2].m[0][0] = 0.0f;	// for LB Btn
			vecVariantMat[2].m[0][1] = 0.5f;
			vecVariantMat[2].m[0][2] = 0.95f;
		}

        CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
            vecVariantMat,
            ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
            true
        };

        targetUI->Set_VariantUIDesc(tVariantDesc);





		// ksta : must separate later..
		auto& UISlots = m_pAbility->Get_UISkillSlots();
		UI_CHARACTERTYPE eCharacterType = static_cast<UI_CHARACTERTYPE>(m_iSelectedCHIndex);
		switch (eCharacterType)
		{
			// ==============================
			// * [SK Icon Update] Rover
			// ==============================
		case UI_CHARACTERTYPE::ROVER:
			// Rover
			Update_Icon_Rover(UISlots);
			break;
			// ==============================
			// * [SK Icon Update] Augusta
			// ==============================
		case UI_CHARACTERTYPE::AUGUSTA:
			// Augusta
			Update_Icon_Augusta(UISlots);
			break;
			// ==============================
			// * [SK Icon Update] Galbrena
			// ==============================
		case UI_CHARACTERTYPE::GALBRENA:
			auto galbrenaUIDesc = pSkillUI[CH_GALBRENA]->Get_UIDesc();

			switch (m_iPlayerEnhancedMode)
			{
			case 0:			// Galbrena Normal
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_E"][0];
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_E"][1];

				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R"][0];
				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R"][1];

				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][0];
				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][1];
				galbrenaUIDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.f, 0.f };
				break;
			case 1:			// Galbrena Normal - Burst Ready
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_E_BurstOn"][0];
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_E_BurstOn"][1];

				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R"][0];
				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R"][1];

				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][0];
				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][1];
				galbrenaUIDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.f, 0.f };
				break;
			case 2:			// Galbrena Burst
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_E"][0];
				galbrenaUIDesc.vecInstanceDescs[0].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_E"][1];

				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordX = m_mapSkillTexIndices[L"Augusta_R"][0];
				galbrenaUIDesc.vecInstanceDescs[1].vSInstCoordY = m_mapSkillTexIndices[L"Augusta_R"][1];

				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordX = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][0];
				galbrenaUIDesc.vecInstanceDescs[2].vSInstCoordY = m_mapSkillTexIndices[L"Galbrena_LB_Burst"][1];
				galbrenaUIDesc.vecInstanceDescs[2].vClipTexcoordX = { 0.f, 1.f };
				break;
			}
			pSkillUI[CH_ROVER]->Set_UIDesc(galbrenaUIDesc);
			break;

		}

    }



    // ==============================
	// * [Change Update] Cooldown
	// ==============================
    for (_uint i = 0; i < CH_END; i++)
    {
        _float fCooldown = fChangeCD[i];
        auto targetUI = pChangeUI[i];

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
	// * [Skill Ready] Skill Ready Indicator
	// ==============================
	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isReady_Augusta_StrongATK = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType) == UI_AUGUSTA_STATE::LB_STRONG_READY;
	_bool isIn_Galbrena_BurstMode = false; /* 나중에 버스트 모드 조건 삽입 */
	_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);
	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;


	static _uint iIndex_EBtn = 2;		static _uint iIndex_PrevEBtn;
	static _uint iIndex_RBtn = 0;		static _uint iIndex_PrevRBtn ;
	static _uint iIndex_LBBtn = 4;		static _uint iIndex_PrevLBBtn ;
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

	

	CCustom_UI* pSkillReadyUI = Find_ChildObject(L"Skill_ReadyFrame");

	auto readyDesc = pSkillReadyUI->Get_UIDesc();
	auto readyInstDesc = pSkillReadyUI->Get_UIDesc().vecInstanceDescs;
	for (auto& instDesc : readyInstDesc)
		instDesc.vClipTexcoordX = { 0.f, 0.f };
	

	// - E Button Indicator : 특수 공격이 준비 될 시에 불만 들어옴.
	 
	//if ("특수 공격 준비 시 함수 따로 만들어야 할 듯. 플레이어 종류마다 조건 제각각이라")
	//	readyInstDesc[iIndex_EBtn].vClipTexcoordX = { 0, 1 };
	//else
	//	readyInstDesc[iIndex_EBtn].vClipTexcoordX = { 0, 0 };


	// - R Button Indicator : 원으로 게이지 차고 (COST5) , 다 차면 불 들어옴
	
	_float fUltGuage = pStatus->Get_CostRatio(m_iSelectedCHIndex, COST_TYPE::COST5);
	//cout << fUltGuage << endl;
	
	vector<_float4> matCustomColor = { }; matCustomColor.resize(CH_END);
	
	switch (m_iSelectedCHIndex)
	{
	case CH_ROVER:		matCustomColor[CH_ROVER]	= _float4{ 0.808f, 0.322f, 0.612f, 1.0f };		break;
	case CH_AUGUSTA:	matCustomColor[CH_AUGUSTA]	=
					(	isIn_Augusta_AdvUlt ||
						pStatus->Get_CostRatio(CH_AUGUSTA, COST_TYPE::COST3) == 1.f ) ?
													  _float4{ 0.992f, 0.749f, 0.341f, 1.0f } :
													  _float4{ 0.969f, 0.451f, 1.000f, 1.0f };		break;
	case CH_GALBRENA:	matCustomColor[CH_GALBRENA] = _float4{ 1.000f, 0.416f, 0.416f, 1.0f };		break;
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

	vector<_float4x4> vecVariantMat = { }; vecVariantMat.resize(readyInstDesc.size());

	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._11) = (1.f - fUltGuage / 1.f);		// CD or Resource Rate
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._12) = 0.f;							// ColorMul1
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._13) = (fUltGuage >= 1.f) ? 1.f : 0.85f ;							// ColorMul2
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._14) = static_cast<_float>(true);	// Is Use CustomColor?
	*reinterpret_cast<_float4*>(&vecVariantMat[iIndex_RBtn]._21) = matCustomColor[m_iSelectedCHIndex];	// CustomColor
	*reinterpret_cast<_float*>(&vecVariantMat[iIndex_RBtn]._31) = 0.f;							// CD Start Degree

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
	pSkillReadyUI->Set_UIDesc(readyDesc);
	pSkillReadyUI->Set_VariantUIDesc(tVariantDesc);

	iIndex_PrevEBtn = iIndex_EBtn;
	iIndex_PrevRBtn = iIndex_RBtn;
	iIndex_PrevLBBtn = iIndex_LBBtn;




#ifdef KSTA_UI_COOLDOWNTEST
    std::cout << "[UI_HUD][Update_UI_Cooldown] ============================== : " << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 1 fChangeCD : " << fChangeCD[CH_ROVER] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 2 fChangeCD : " << fChangeCD[CH_AUGUSTA] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] 3 fChangeCD : " << fChangeCD[CH_GALBRENA] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] E fSkillCD  : " << fSkillCD[iSelectedCHIndex][SK_E] << std::endl;
    std::cout << "[UI_HUD][Update_UI_Cooldown] R fSkillCD  : " << fSkillCD[iSelectedCHIndex][SK_R] << std::endl;
#endif // KSTA_UI_COOLDOWNTEST

}

void CUI_HUD::Update_UI_SkillSection_BG(_float fTimeDelta)
{
	// ==============================
	// * Skill_BackgroundImage
	// =============================='

	
	static _uint iNumActiveBG = 4;

	auto& skillSlots = m_pPlayerStatus->Get_Ability(CH_AUGUSTA)->Get_UISkillSlots();

	_bool isReady_Augusta_StrongATK = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_LB].iStateType) == UI_AUGUSTA_STATE::LB_STRONG_READY;
	_bool isIn_Galbrena_BurstMode = false; /* ksta : 나중에 버스트 모드 조건 삽입 */

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

	CCustom_UI* pSkillBGUI = Find_ChildObject(L"Skill_BackgroundImage");    // �ν��Ͻ� 4����

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

	if (!Find_ChildObject(L"SectorRB_SkillIcons")->IsActivate())
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
		_bool isIn_Galbrena_BurstMode = false; /* ksta : 나중에 버스트 모드 조건 삽입 */
		if (isIn_Galbrena_BurstMode)
			isChar_LBBtnFeedbackAble = true;
		else
			isChar_LBBtnFeedbackAble = false;
	}break;
	}



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


	
	// 클릭마다 해당 위치에 피드백 생성
	if (m_pGameInstance->Get_DIKeyState(DIK_E) == KEYSTATE::DOWN &&
		isChar_EButtonFeedbackAble)
		Add_UI_SkillSection_OnFeedback(iIndex_EBtn);
	if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
		Add_UI_SkillSection_OnFeedback(iIndex_RBtn);
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN &&
		isChar_LBBtnFeedbackAble)
		Add_UI_SkillSection_OnFeedback(iIndex_LBBtn);
	
}

void CUI_HUD::Update_UI_SkillSection_OnFeedback(_float fTimeDelta)
{
    CCustom_UI* pFeedbackUI = Find_ChildObject(L"Skill_OnFeedback");    
    const _float2 fDestScale = { 1.2f, 1.2f };
    const _float fStartAlpha = 0.f;     // 0�� ����, 1�� �Ⱥ������� ����.
    const _float fLifeTime = .5f;
    _float4 vColor = { 0.f, 0.f, 0.f, 1.f };

    auto uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;


    static vector<_float> vecLifeTimeElapsed = {};
    
    if (uiInstDescs.size() > vecLifeTimeElapsed.size())
    {
        _uint iAddLoopTime = uiInstDescs.size() - vecLifeTimeElapsed.size();
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
            pFeedbackUI->Set_UIDesc(uiDesc);
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
    CCustom_UI* pFeedbackUI = Find_ChildObject(L"Skill_OnFeedback");

    const _float4 vStartPos = { 850.f, -415.f, 0.f, 1.f };
    const _float fDeltaPosX = -100.f;

    vector<_float4> vecPosIndex = {};

    for (_uint i = 0; i < 5; i++)
    {
        _float4 vPos = vStartPos;
        vPos.x = vPos.x + fDeltaPosX * (i);
        vecPosIndex.push_back(vPos);
    }

    auto uiDesc = pFeedbackUI->Get_UIDesc();
    auto& uiInstDescs = uiDesc.vecInstanceDescs;



    CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
    tDesc.vSInstRight   = { 80.f, 0.f, 0.f, 0.f };
    tDesc.vSInstUp      = { 0.f, 80.f, 0.f, 0.f };
    tDesc.vSInstLook    = { 0.f, 0.f, 1.f, 0.f };
    tDesc.vSInstTrans   = vecPosIndex[iSectionIndex];

    uiInstDescs.push_back(tDesc);
    uiDesc.vecInstanceDescs = uiInstDescs;
    pFeedbackUI->Set_UIDesc(uiDesc);
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

    CCustom_UI* targetUI = Find_ChildObject(L"Inst_HPBar");



    
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


    //if (m_pGameInstance->Get_DIKeyState(DIK_P) == KEYSTATE::DOWN)       // [Test]
    //{
    //    if (fPlayerHP[m_iSelectedCHIndex] == 0) fPlayerHP[m_iSelectedCHIndex] = fPlayerMaxHP[m_iSelectedCHIndex];
    //    isHit = true;
    //}
	//
    //if (isHit == true)
    //{
	//
    //    _float fRandDamage = m_pGameInstance->Rand(100.f, 500.f);       // [Test] External Value
	//
    //    // HP�� ��� ����
    //    fPlayerHP[m_iSelectedCHIndex] -= fRandDamage;
    //    if (fPlayerHP[m_iSelectedCHIndex] < 0) fPlayerHP[m_iSelectedCHIndex] = 0;
	//
    //    fHPReduceLeftTime = fHPReduceTime;
    //}
	

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
    //if (pBoss == nullptr)
    //    return;
    


    // ksta : ���߿� ���� ���� ���յǸ� �ű��κ��� �޾ƿ� ����
    static _float fBossHP = { 10000.f };            // boss hitpoint
    static _float fBossBackHP = fBossBackHP;
    const _float fBossMaxHP = { 10000.f };
    
    static _float fBossSA = { 4000.f };             // boss superarmor
    static _float fBossBackSA = fBossSA;
    const _float fBossMaxSA = { 4000.f };
    static _bool isSABreak = false;




    static _bool isHit = false;
    static _float fHPReduceLeftTime = 0.f;

    _float fBossHPRatio = fBossHP / fBossMaxHP;
    _float fBossSARatio = fBossSA / fBossMaxSA;
    static _float fBossHPBackRatio = fBossHPRatio;
    static _float fBossSABackRatio = fBossSARatio;

    const _float4 vHPColor1         = { 1.f, .7f, .1f, 1.f };
    const _float4 vHPColor2         = { 1.f, .2f, .0f, 1.f };
    const _float4 vHPBackColor1     = { .8f, .8f, .8f, 1.f };

    const _float4 vSAColor          = { 1.f, 1.f, 1.f, 1.f };   // before armor break
    const _float4 vSABreakColor     = { .9f, .8f, .3f, 1.f };
    const _float4 vSABackColor      = { 1.f, 1.f, 1.f, .3f };   // after armor break
    //const _float4 vSABreakBackColor = { .2f, .2f, .2f, 1.f };

    const _float fHPReduceTime = 0.5f;          // �پ��� �ҿ�ð��� 0.5������?

    const auto targetUI = Find_ChildObject(L"Inst_BossHPBar");
    const auto targetSAUI = Find_ChildObject(L"Inst_BossSABar");


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


    if (m_pGameInstance->Get_DIKeyState(DIK_O) == KEYSTATE::DOWN)       // [Test]
    {
        if (fBossHP == 0) fBossHP = fBossMaxHP;
        if (fBossSA == 0) fBossSA = fBossMaxSA;
        isHit = true;
    }

    if (isHit == true)
    {
        _float fRandDamage = m_pGameInstance->Rand(100.f, 500.f);       // [Test] External Value
        _float fRandSADamage = fRandDamage * 0.8f;

        // HP�� ��� ����
        fBossHP -= fRandDamage;
        fBossSA -= fRandSADamage;

        if (fBossHP < 0) fBossHP = 0;
        if (fBossSA < 0) fBossSA = 0;

        fHPReduceLeftTime = fHPReduceTime;
    }

    // change
    vector<_float4x4> vecVariantMat = { _float4x4() , _float4x4() };
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._11)    = vHPColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._11)      = vHPBackColor1;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_NORMAL]._21)    = vHPColor2;
    *reinterpret_cast<_float4*>(&vecVariantMat[BOHP_BACK]._21)      = vHPBackColor1;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_NORMAL]._31)     = fBossHPRatio;
    *reinterpret_cast<_float*>(&vecVariantMat[BOHP_BACK]._31)       = fBossHPBackRatio;

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

    isHit = false;



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
    CCustom_UI* pKeyButtonUI = Find_ChildObject(L"Inst_KeyButton");
    auto keyButtonDesc = pKeyButtonUI->Get_UIDesc();

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
    pKeyButtonUI->Set_UIDesc(keyButtonDesc);
}

void CUI_HUD::Update_UI_PlayerEnergyFrame(_float fTimeDelta)
{
	CPlayerStatus* pStatus = m_pGameSystem->Get_PlayerStatus();

	// 공통 정보 받아옴
	auto& skillSlots = pStatus->Get_Ability(ENUM_CLASS(CH_AUGUSTA))->Get_UISkillSlots();


		_bool isIn_AdvUltMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	UI_AUGUSTA_STATE eState_Augusta_R = static_cast<UI_AUGUSTA_STATE>(skillSlots[CAbility::KEY_R].iStateType);

	_bool isIn_Augusta_AdvUlt = (eState_Augusta_R == UI_AUGUSTA_STATE::R_SWORD_ULTI_READY) || isIn_AdvUltMode;

	_bool isIn_Rover_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));


	//tUISlot.



    switch (m_iSelectedCHIndex)
    {
    case Client::CUI_HUD::CH_ROVER:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(true);     // CH change visible
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(false);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(false);

        if	(	!isIn_Rover_BurstMode	)
        {
            Find_ChildObject(L"Frame_Rover_Dark")->Set_Active(false);
            // Find_ChildObject(L"Frame_Rover")->Set_Active(true); // nullptr
        } 
        else if(isIn_Rover_BurstMode)
        {
            Find_ChildObject(L"Frame_Rover_Dark")->Set_Active(true);
            // Find_ChildObject(L"Frame_Rover")->Set_Active(false); // nullptr
        }
                                    
                                    
        break;
    case Client::CUI_HUD::CH_AUGUSTA:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(false);
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(true);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(false);

        if  (	!isIn_Augusta_AdvUlt	)
        {
            Find_ChildObject(L"Frame_Augusta")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Augusta_OtherEnergy")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Augusta_UltMode")->Set_Active(false);
        }
		else if(isIn_Augusta_AdvUlt)
        {
            Find_ChildObject(L"Frame_Augusta")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Augusta_OtherEnergy")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Augusta_UltMode")->Set_Active(true);
        }

        break;
    case Client::CUI_HUD::CH_GALBRENA:
        Find_ChildObject(L"Group_Rover")    ->Set_Active(false);
        Find_ChildObject(L"Group_Augusta")  ->Set_Active(false);
        Find_ChildObject(L"Group_Galbrena") ->Set_Active(true);

        if  (	m_iPlayerEnhancedMode == 0 ||
				m_iPlayerEnhancedMode == 2	)
        {
            Find_ChildObject(L"Frame_Galbrena")->Set_Active(true);
            Find_ChildObject(L"Frame_Galbrena_Icon")->Set_Active(true);
            Find_ChildObject(L"FrameGroup_Galbrena_RageMode")->Set_Active(false);
        }
        else if(m_iPlayerEnhancedMode == 1)
        {
            Find_ChildObject(L"Frame_Galbrena")->Set_Active(false);
            Find_ChildObject(L"Frame_Galbrena_Icon")->Set_Active(false);
            Find_ChildObject(L"FrameGroup_Galbrena_RageMode")->Set_Active(true);
        }

        break;
    }


    



    // �Ӽ� ������ ���� ����
    vector<CCustom_UI*> pElementIcons = {
        Find_ChildObject(L"Icon_ElementDark"),
        Find_ChildObject(L"Icon_ElementThunder"),
        Find_ChildObject(L"Icon_ElementFire")
    };
    CCustom_UI* pElementTargetUI = pElementIcons[m_iSelectedCHIndex];
    const vector<_float4> vecElemColors = {
        {0.808f, 0.322f, 0.612f, 1.0f},			// Dark
        {0.969f, 0.451f, 1.0f, 1.0f},			// Elec
        {1.0f, 0.416f, 0.416f, 1.0f}			// Fusi
    };

    vector<_float4x4> vecElementVariantMat = { _float4x4() };
    *reinterpret_cast<_float4*>(&vecElementVariantMat[0]._11) = vecElemColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementVariantMat[0]._21) = static_cast<_float>(true);

    CCustom_UI::VARIANTREADY_UI_DESC tElementVariantDesc = {
        vecElementVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLEMASK),
        true
    };

    pElementTargetUI->Set_VariantUIDesc(tElementVariantDesc);

    
    
    // �Ӽ� ������ �ֺ� ���� �� ����
    const vector<_float4> vecElemCircleColors = {
        {0.808f, 0.322f, 0.612f, 1.0f},         // Dark
        {0.969f, 0.451f, 1.0f, 1.0f},         // Thunder
        {1.0f, 0.416f, 0.416f, 1.0f}          // Fire
    };
    static _float fElementAmounts[CH_END] = { 0.f, 0.f ,0.f };
    static _float fMaxElementAmounts[CH_END] = {100.f, 100.f, 100.f};

    // ksta : test 

	fElementAmounts[m_iSelectedCHIndex] = m_pAbility->Get_Harmony();
    //fElementAmounts[0] = (fElementAmounts[0] >= 100)? 0 : fElementAmounts[0] + 2.f  * 30.f * fTimeDelta;
    //fElementAmounts[1] = (fElementAmounts[1] >= 100)? 0 : fElementAmounts[1] + 1.5f * 30.f * fTimeDelta;
    //fElementAmounts[2] = (fElementAmounts[2] >= 100)? 0 : fElementAmounts[2] + 1.f  * 30.f * fTimeDelta;

    

    CCustom_UI* pElementGuageUI = Find_ChildObject(L"Icon_ElementGuage");
    //auto elementGuageDesc = pElementGuageUI->Get_UIDesc();

    vector<_float4x4> vecElementGuageVariantMat = { _float4x4() };
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._11) = (1.f - fElementAmounts[m_iSelectedCHIndex] / fMaxElementAmounts[m_iSelectedCHIndex]);
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._12) = 0.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._13) = 1.0f;
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._14) = static_cast<_float>(true);
    *reinterpret_cast<_float4*>(&vecElementGuageVariantMat[0]._21) = vecElemCircleColors[m_iSelectedCHIndex];
    *reinterpret_cast<_float*>(&vecElementGuageVariantMat[0]._31) = 90.f;



    CCustom_UI::VARIANTREADY_UI_DESC tElementAmountVariantDesc = {
        vecElementGuageVariantMat,
        ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
        true
    };

    pElementGuageUI->Set_VariantUIDesc(tElementAmountVariantDesc);

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

	_bool isIn_Rover_BurstMode = m_pAbility->Check_AnyCondition(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

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

    const auto targetUI = Find_ChildObject(L"Inst_EnergyItems");


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

    vector<_bool> vIsVisible = {};							// visible mask for front & back indices information
    vIsVisible.resize(iNumSpectrums);
    vector<_bool> vIsVisibleStatic = {};					// visible mask for static indices information
    vIsVisibleStatic.resize(iNumSpectrums);




    switch (m_iSelectedCHIndex)
    {
    case CH_ROVER:     
        {
            if      (!isIn_Rover_BurstMode)     /* Normal */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1];

                //vIsVisible.assign(vIsVisible.size(), true);                    // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(vIsVisible.begin(), vIsVisible.end() - (41 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
            }
            else if (isIn_Rover_BurstMode)     /* Ult    */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_ROVER_NORMAL][0];         // color
                vSingleColor[1] = vecColorPreset[ENCL_ROVER_NORMAL][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 16.f);      // applying player energy
                fill(vIsVisible.begin() + (16 - iVisibleBarRange), vIsVisible.end() - 25, true);
                fill(vIsVisible.end() - 16, vIsVisible.end() - (16 - iVisibleBarRange), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
        }break;
    case CH_AUGUSTA:   
        {
            if  (!isIn_Augusta_AdvUlt &&
				 (fCurPlayerEnergyRatio != 1.f) )
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_NORMAL][0];       // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_NORMAL][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(vIsVisible.begin(), 
                    (iVisibleBarRange > 16)? vIsVisible.begin() + 16 : vIsVisible.begin() + iVisibleBarRange, true);
                fill(vIsVisible.end() - 16,
                    vIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
            else if(!isIn_Augusta_AdvUlt &&
					(fCurPlayerEnergyRatio == 1.f))     /* Ult?   */
            { 
                vSingleColor[0] = vecColorPreset[ENCL_AUGUSTA_ULT][0];          // color
                vSingleColor[1] = vecColorPreset[ENCL_AUGUSTA_ULT][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible
                fill(vIsVisible.begin() + 16, vIsVisible.end() - 16, false);

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 32.f);      // applying player energy
                fill(vIsVisible.begin(),
                    (iVisibleBarRange > 16) ? vIsVisible.begin() + 16 : vIsVisible.begin() + iVisibleBarRange, true);
                fill(vIsVisible.end() - 16,
                    vIsVisible.end() - 16 + ((iVisibleBarRange > 16) ? (iVisibleBarRange - 16) : 0), true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 16, vIsVisibleStatic.end() - 16, false);
            }
            else if (isIn_Augusta_AdvUlt)
            {
                fill(vIsVisibleStatic.begin(), vIsVisibleStatic.end(), false);
            }
        }break;
    case CH_GALBRENA:  
        {
            if  (	m_iPlayerEnhancedMode == 0 ||
					m_iPlayerEnhancedMode == 1)     /* Normal */  
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][0];   // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_L][1];  
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                fill(vIsVisible.begin() + 11, vIsVisible.end() - 26, false);    // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 26.f);      // applying player energy
                fill(vIsVisible.end() - 26, vIsVisible.end() - 26 + iVisibleBarRange, true);

                // [Galbrena] applying echo energy
                _uint iVisibleBarRange_Echo = static_cast<_uint>(fGalbEchoEnergy / fGalbMaxEchoEnergy * 11.f);
                fill(vIsVisible.begin() + 11 - iVisibleBarRange_Echo, vIsVisible.begin() + 11, true);


                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
                fill(vIsVisibleStatic.begin() + 11, vIsVisibleStatic.end() - 26, false);
            }
            else if(m_iPlayerEnhancedMode == 2)     /* Burst   */
            { 
                vSingleColor [0] = vecColorPreset[ENCL_GALBRENA_ULT][0];        // color
                vSingleColor [1] = vecColorPreset[ENCL_GALBRENA_ULT][1];       
                vExtraColor  [0] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][0]; 
                vExtraColor  [1] = vecColorPreset[ENCL_GALBRENA_NORMAL_R][1]; 

                //vIsVisible.assign(vIsVisible.size(), true);                     // isvisible

                _uint iVisibleBarRange = static_cast<_uint>(fCurPlayerEnergyRatio * 41.f);      // applying player energy
                fill(vIsVisible.begin(), vIsVisible.begin() + iVisibleBarRange, true);

                // applying player energy - not filled
                for (_uint i = 0; i < vIsVisible.size(); i++)
                    vIsVisibleStatic[i] = !vIsVisible[i];
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


    vector<_float4x4> vecVariantMat = {};
    vecVariantMat.resize(41);
    vector<_float4x4> vecVariantBackMat = {};
    vecVariantBackMat.resize(41);
    vector<_float4x4> vecVariantStaticMat = {};
    vecVariantStaticMat.resize(41);


    for (uint i = 0; i < vecVariantMat.size(); i++)         // front spectrum.
    {
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSingleColor[0];
        *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vSingleColor[1];
        vecVariantMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        vecVariantMat[i]._32 = vSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantBackMat.size(); i++)     // back spectrum. 
    {
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._11) = vBackColor[0];
        *reinterpret_cast<_float4*>(&vecVariantBackMat[i]._21) = vBackColor[1];
        vecVariantBackMat[i]._31 = static_cast<_float>(vIsVisible[i]);
        //vecVariantBackMat[i]._31 = false;
        vecVariantBackMat[i]._32 = vBackSpectrumHeights[VALUE][i];
    }
    for (uint i = 0; i < vecVariantStaticMat.size(); i++)   // static spectrum.
    {
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._11) = vecColorPreset[ENCL_STATIC][0];
        *reinterpret_cast<_float4*>(&vecVariantStaticMat[i]._21) = vecColorPreset[ENCL_STATIC][1];
        vecVariantStaticMat[i]._31 = static_cast<_float>(vIsVisibleStatic[i]);               
        //vecVariantStaticMat[i]._31 = false;                                                
        vecVariantStaticMat[i]._32 = 1.f;                                                    
    }


	// galbrena energy bar
    if (m_iSelectedCHIndex == CH_GALBRENA)
    {
        if (m_iPlayerEnhancedMode == 0 ||
			m_iPlayerEnhancedMode == 1)
        {
            for (uint i = 0; i < vecVariantMat.size(); i++)
                if (i >= 15)
                {
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vExtraColor[0];
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vExtraColor[1];
                }
            for (uint i = 0; i < vecVariantBackMat.size(); i++)
                if (i >= 15)
                {
                    _float4 vExtraBackColor[2];
                    vExtraBackColor[0] = vExtraColor[0];   vExtraBackColor[0].w = 0.5f;
                    vExtraBackColor[1] = vExtraColor[1];   vExtraBackColor[1].w = 0.5f;
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vExtraBackColor[0];
                    *reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vExtraBackColor[1];
                }   
        }
    }

    vecVariantBackMat.insert(vecVariantBackMat.end(),           // combine two vector. -> size = 41 + 41 = 82
        make_move_iterator(vecVariantMat.begin()),
        make_move_iterator(vecVariantMat.end()));
    vecVariantBackMat.insert(vecVariantBackMat.end(),           // combine two vector. -> size = 82 + 41 = 123
        make_move_iterator(vecVariantStaticMat.begin()),
        make_move_iterator(vecVariantStaticMat.end()));

    CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
        vecVariantBackMat,
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


    CCustom_UI* pBladeUI    = Find_ChildObject(L"Frame_Augusta_Inst_SwordEnergy");         
    CCustom_UI* pPointUI    = Find_ChildObject(L"Frame_Augusta_Inst_CenterPointEnergy");   

    CCustom_UI* pUltBladeUI = Find_ChildObject(L"Frame_Augusta_Inst_UltModeEnergy");       


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




    auto bladeDesc = pBladeUI->Get_UIDesc();

    auto ultBladeDesc = pUltBladeUI->Get_UIDesc();
    
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

    pBladeUI->Set_UIDesc(bladeDesc);
    


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

    pUltBladeUI->Set_UIDesc(ultBladeDesc);

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

	auto roverUIDesc = Find_ChildObject(L"Skill_Rover")->Get_UIDesc();

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

	Find_ChildObject(L"Skill_Rover")->Set_UIDesc(roverUIDesc);

}



void CUI_HUD::Update_Icon_Augusta(const vector<UISKILL_SLOT>& skillSlots)
{

	auto augustaUIDesc = Find_ChildObject(L"Skill_Augusta")->Get_UIDesc();

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



	Find_ChildObject(L"Skill_Augusta")->Set_UIDesc(augustaUIDesc);

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
    for (auto& child : m_vecChildObjects)
        Safe_Release(child);

    __super::Free();
}
