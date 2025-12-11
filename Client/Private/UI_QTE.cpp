
#include "ClientPch.h"
#include "UI_QTE.h"

#include "Animator_UI.h"
#include "GameSystem.h"

#include "Event_Level.h"
#include "Event_Leviatan.h"

#define KSTA_UITEST_TEMPTRIGGER

CUI_QTE::CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_QTE::CUI_QTE(const CUI_QTE& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}


HRESULT CUI_QTE::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_QTE::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);
	
	Ready_Components(pArg);
	//__super::Ready_Events();

	Ready_Events();
	
	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_QTE1.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_Initialize.json",					//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeIn.json",						
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeOut.json",						
																							
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Initialize.json",				//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Start.json",					
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_Assemble_AlphaStrength.json",	
																							
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Initialize.json",				//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Start.json",					
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Succeed.json",					
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE2A_FG_Initialize.json",				//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE2A_FG_Triggered.json",				
																							
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_Initialize.json",		//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_FadeIn.json",			
																							
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FG_Arrow_Initialize.json",			//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FG_Arrow_TickLoop.json",			
																							
																							
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_FeedbackRing_Initialize.json",	//	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_FeedbackRing_TickLoop.json",    

	};
	Load_Animations(vecAnimFilePaths);
	
	Reset(_fmatrix(), pArg);
	m_isActivate = false;

	m_pAnim_RUI_All->Change_Animation(L"QTE1_Initialize");
	m_pAnim_UI_SectorA_KeyGuide->Change_Animation(L"QTE1A_KeyGuide_Initialize");
	m_pAnim_UI_SectorA_BG->Change_Animation(L"QTE1A_BG_Initialize");
	m_pAnim_UI_SectorA_FG_Fillguage->Change_Animation(L"QTE1A_FG_Initialize");
	m_pAnim_UI_SectorA_FG_Trigger->Change_Animation(L"QTE2A_FG_Initialize");

	m_pAnim_UI_FG_QTEArrow->Change_Animation(L"QTE1_FG_Arrow_Initialize");
	m_pAnim_UI_FG_QTEFeedbackRing->Change_Animation(L"QTE1_BG_FeedbackRing_Initialize");


	//m_pAnim_RUI_All->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//m_pAnim_UI_SectorA_KeyGuide->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//m_pAnim_UI_SectorA_BG->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//m_pAnim_UI_SectorA_FG->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));

	//Create_ChildText();
	//m_pGameInstance->Add_RootUI(L"UI_QTE", this);

	m_isClone = true;

	return S_OK;
}

void CUI_QTE::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_QTE::Update(_float fTimeDelta)
{
	Update_FinishEvent(fTimeDelta);
	Update_GoinDisabled(fTimeDelta);
	Update_AnimOrder(fTimeDelta);
	
	switch (m_eQTEType)		// m_isQTEMode 일 시 진행
	{
	case Client::UI_QTE_TYPE::FILLGUAGE:		Update_QTE_Fillguage(fTimeDelta);		break;
	case Client::UI_QTE_TYPE::TRIGGER_ROPE:	
	case Client::UI_QTE_TYPE::TRIGGER_EXECUTE:	Update_QTE_Trigger(fTimeDelta);			break;
	}

	__super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_QTE::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	Update_Instances(fTimeDelta);

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_QTE::Render()
{
	if (!m_isActivate)
		return;

	__super::Render();
}

void CUI_QTE::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	UI_QTE_DESC* pDesc = static_cast<UI_QTE_DESC*>(pArg);

	m_eIconIndex		= pDesc->eIconIndex;
	m_eQTEType			= pDesc->eQTEType;
	_float2 vSpawnPos	= pDesc->vSpawnPos;

	_float4 vPosition = { vSpawnPos.x, vSpawnPos.y, 0.f, 1.f };
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vPosition));
	_float3 vScale		= _float3(pDesc->vSpawnScale.x, pDesc->vSpawnScale.y, 1.f);
	static_cast<CTransform*>(m_pRRUI_TransformCtrl->Get_Component(L"Com_Transform"))->Scale(vScale);

	
	// m_eIconIndex 에 따른, 중앙에 나올 이미지 변경
	auto& keyDesc = m_pUI_KeyButtons->Get_UIDesc();
	auto& keyInstDesc = keyDesc.vecInstanceDescs;

	keyInstDesc[0].vSInstCoordX = m_arrBtnPresets[ENUM_CLASS(m_eIconIndex)][0];
	keyInstDesc[0].vSInstCoordY = m_arrBtnPresets[ENUM_CLASS(m_eIconIndex)][1];	



	// m_eQTEType에 따른, 타입 별 이미지 다르게. 이는 Update단에서 on/off 로 하도록.
	switch (m_eQTEType)
	{
	case Client::UI_QTE_TYPE::FILLGUAGE:
	{
		m_pUI_KeyButtons->SetActivate(true);				// on
		m_pUI_AbilityIconBG->SetActivate(false);			// off
		m_pUI_AbilityIcons->SetActivate(false);				// off
		m_pUI_ExtraIcons->SetActivate(false);				// off

		m_pUI_SectorA_FG_Fillguage->SetActivate(true);		// on
		m_pUI_SectorA_FG_Trigger->SetActivate(false);		// off

		m_fQTEDropRate		= 0.25f;	// 초당 떨어지는 정도.
		m_fQTEFillAmount	= 0.15f;		// 조작 1회 당 차는 정도
		m_fQTEMaxTime		= 5.f;		// QTE 제한시간.
	}break;
	case Client::UI_QTE_TYPE::TRIGGER_ROPE:
	{
		m_pUI_KeyButtons->SetActivate(!true);				// !on
		m_pUI_AbilityIconBG->SetActivate(!false);			// !off
		m_pUI_AbilityIcons->SetActivate(!false);			// !off
		m_pUI_ExtraIcons->SetActivate(false);				// off

		m_pUI_SectorA_FG_Fillguage->SetActivate(!true);		// !on
		m_pUI_SectorA_FG_Trigger->SetActivate(!false);		// !off

		m_fQTEDropRate		= 0.0f;
		m_fQTEFillAmount	= 1.0f;		// 사실상 한번만 누르면 바로 차게끔.	
		m_fQTEMaxTime		= 5.f;		// 필요 시 변경
	}break;
	case Client::UI_QTE_TYPE::TRIGGER_EXECUTE:
	{
		m_pUI_KeyButtons->SetActivate(!true);				// !on
		m_pUI_AbilityIconBG->SetActivate(!false);			// !off
		m_pUI_AbilityIcons->SetActivate(false);				// !off
		m_pUI_ExtraIcons->SetActivate(!false);				// !off

		m_pUI_SectorA_FG_Fillguage->SetActivate(!true);		// !on
		m_pUI_SectorA_FG_Trigger->SetActivate(!false);		// !off

		m_fQTEDropRate		= 0.0f;
		m_fQTEFillAmount	= 1.0f;		// 사실상 한번만 누르면 바로 차게끔.	
		m_fQTEMaxTime		= FLT_MAX;		// 필요 시 변경
	}break;
	}



	m_fQTEGuage = 0.f;
	m_fQTEElapsedTime = 0.f;
	m_fElapsedTime = 0.f;
	m_isQTEMode = false;

	m_iAnimOrder = 0;
	m_fElapsedTime = 0.f;

	m_pAnim_RUI_All->Change_Animation(L"QTE1_Initialize");
	m_pAnim_UI_SectorA_KeyGuide->Change_Animation(L"QTE1A_KeyGuide_Initialize");
	m_pAnim_UI_SectorA_BG->Change_Animation(L"QTE1A_BG_Initialize");
	m_pAnim_UI_SectorA_FG_Fillguage->Change_Animation(L"QTE1A_FG_Initialize");
	m_pAnim_UI_SectorA_FG_Trigger->Change_Animation(L"QTE2A_FG_Initialize");

	m_pAnim_UI_FG_QTEArrow->Change_Animation(L"QTE1_FG_Arrow_Initialize");
	m_pAnim_UI_FG_QTEFeedbackRing->Change_Animation(L"QTE1_BG_FeedbackRing_Initialize");

	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iDisableAnimOrder = 0;

	m_isGoinSuccess = false;
	m_isGoinFail = false;

	m_isActivate = true;

	if (m_isClone)
		m_pGameSystem->HUD_FadeOut();
}

void CUI_QTE::PreAssign_ChildUIs()
{
	m_pRRUI_TransformCtrl			= Find_ChildObject(L"Sub_TransformCtrl");

	m_pRUI_All						= Find_ChildObject(L"Sub_All");
	m_pUI_SectorA_KeyGuide			= Find_ChildObject(L"SectorA_KeyGuide");
	m_pUI_SectorA_BG				= Find_ChildObject(L"SectorA_BG");
	m_pUI_SectorA_FG_Fillguage		= Find_ChildObject(L"SectorA_FG_Fillguage");
	m_pUI_SectorA_FG_Trigger		= Find_ChildObject(L"SectorA_FG_Trigger");

	m_pAnim_RUI_All					= dynamic_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_KeyGuide		= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_KeyGuide->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_BG			= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_BG->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_FG_Fillguage	= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_FG_Fillguage->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_FG_Trigger	= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_FG_Trigger->Get_Component(L"Com_Animator_UI"));
	
	m_pUI_KeyButtons				= Find_ChildObject(L"KeyButtons");
	m_pUI_BG_QTEFrame				= Find_ChildObject(L"QTE_Frame");
	m_pUI_AbilityIconBG				= Find_ChildObject(L"AbilityIconBG");
	m_pUI_AbilityIcons				= Find_ChildObject(L"AbilityIcons");
	m_pUI_ExtraIcons				= Find_ChildObject(L"ExtraIcons");
	m_pUI_BG_QTEAssemble			= Find_ChildObject(L"QTE_Assemble");
	m_pUI_FG_QTEFeedbackRing		= Find_ChildObject(L"QTE_FeedbackRing");
	m_pUI_FG_QTEGuageFrame			= Find_ChildObject(L"QTE_GuageFrame");
	m_pUI_FG_QTEGuage				= Find_ChildObject(L"QTE_Guage");
	m_pUI_FG_QTEArrow				= Find_ChildObject(L"QTE_Arrow");
	m_pUI_FG_Trigger				= Find_ChildObject(L"QTE_TriggerGuage");

	m_pAnim_UI_BG_QTEAssemble		= dynamic_cast<CAnimator_UI*>(m_pUI_BG_QTEAssemble->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_FG_QTEArrow			= dynamic_cast<CAnimator_UI*>(m_pUI_FG_QTEArrow->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_FG_QTEFeedbackRing	= dynamic_cast<CAnimator_UI*>(m_pUI_FG_QTEFeedbackRing->Get_Component(L"Com_Animator_UI"));

	array<_uint, 2> arrNumMax = {4, 4};
	m_arrBtnPresets[ENUM_CLASS(UI_QTE_BTN::F)] = Calc_SpriteSpace(2, 0, arrNumMax); // 3
	m_arrBtnPresets[ENUM_CLASS(UI_QTE_BTN::E)] = Calc_SpriteSpace(3, 0, arrNumMax); // 4
	m_arrBtnPresets[ENUM_CLASS(UI_QTE_BTN::Q)] = Calc_SpriteSpace(3, 1, arrNumMax); // 8
	m_arrBtnPresets[ENUM_CLASS(UI_QTE_BTN::R)] = Calc_SpriteSpace(3, 2, arrNumMax); // 12
	m_arrBtnPresets[ENUM_CLASS(UI_QTE_BTN::T)] = Calc_SpriteSpace(2, 3, arrNumMax); // 15
}

array<_float2, 2> CUI_QTE::Calc_SpriteSpace(_uint iIndexX, _uint iIndexY, array<_uint, 2> iNumMax, _float2 vSpriteSize)
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

void CUI_QTE::Update_AnimOrder(_float fTimeDelta)
{
	const _uint iNumAnimOrder = 3;

	if (m_iAnimOrder >= iNumAnimOrder)
		return;


	const array<_float, 3> arrAnimOrderTimings = { 0.0f, 0.25f, 0.5f };

	if (	m_iAnimOrder == 0 &&
			m_fElapsedTime >= arrAnimOrderTimings[0])	// 0.0f
	{
		m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeIn") ;
		m_pAnim_UI_SectorA_BG->Change_Animation(L"QTE1A_BG_Start") ;
		m_pAnim_UI_BG_QTEAssemble->Change_Animation(L"QTE1_BG_Assemble_AlphaStrength");
		m_pAnim_UI_FG_QTEArrow->Change_Animation(L"QTE1_FG_Arrow_TickLoop");
		m_pAnim_UI_FG_QTEFeedbackRing->Change_Animation(L"QTE1_BG_FeedbackRing_TickLoop");

		m_iAnimOrder++;
	}
	else if(m_iAnimOrder == 1 &&
			m_fElapsedTime >= arrAnimOrderTimings[1])	// 0.25f
	{
		m_pAnim_UI_SectorA_FG_Fillguage->Change_Animation(L"QTE1A_FG_Start") ;

		m_iAnimOrder++;
	}
	else if(m_iAnimOrder == 2 &&
			m_fElapsedTime >= arrAnimOrderTimings[2])	// 0.5f
	{
		m_pAnim_UI_SectorA_KeyGuide->Change_Animation(L"QTE1A_KeyGuide_FadeIn") ;
		m_isQTEMode = true;
		m_iAnimOrder++;
	}

	m_fElapsedTime += fTimeDelta;

	// 이후 m_fElapsedTime, m_iAnimOrder 초기화 필요
}

void CUI_QTE::Update_Instances(_float fTImeDelta)
{
	// 내부 게이지 (m_pUI_FG_QTEGuage) 에 한해, m_fQTEGuage  값을 따라 원형 변화 필요.
	// 6시 방향부터 반시계로 참.

	auto& guageDesc = m_pUI_FG_QTEGuage->Get_UIDesc();
	auto& guageInstDesc = guageDesc.vecInstanceDescs;

	vector<_float4x4> vecGuageVariantMat = { _float4x4()};
	*reinterpret_cast<_float*>(&vecGuageVariantMat[0]._11) = (1.f - m_fQTEGuage);
	*reinterpret_cast<_float*>(&vecGuageVariantMat[0]._12) = 0.0f;				// 세부조절 필요
	*reinterpret_cast<_float*>(&vecGuageVariantMat[0]._13) = 1.0f;
	*reinterpret_cast<_float*>(&vecGuageVariantMat[0]._14) = static_cast<_float>(false);
	*reinterpret_cast<_float*>(&vecGuageVariantMat[0]._31) = 180.f;

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecGuageVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_COOLDOWN_CIRCLE),
		true
	};

	m_pUI_FG_QTEGuage->Set_VariantUIDesc(tVariantDesc);
}

void CUI_QTE::Update_QTE_Fillguage(_float fTimeDelta)
{
	if (!m_isQTEMode)					// QTE가 켜질 시 진행.
		return;

	// 1. 일정 시간마다 떨어진다.
	
	_float fDropAmount = fTimeDelta * m_fQTEDropRate;
	m_fQTEGuage = ((m_fQTEGuage - fDropAmount) >= 0.f) ? m_fQTEGuage - fDropAmount : 0.f;

	// 2. 지정된 버튼을 누를 시, 해당 버튼이 눌렸을 때에
	// 피드백 효과, 게이지 상승, 다 찼는지의 판별 여부 등을 진행한다. 앨단 F.
	
	if (m_pGameInstance->Get_DIKeyState(m_arrBtnMapping[ENUM_CLASS(m_eIconIndex)]) == KEYSTATE::DOWN)
	{
		m_fQTEGuage = ((m_fQTEGuage + m_fQTEFillAmount) <= 1.f) ? m_fQTEGuage + m_fQTEFillAmount : 1.f;			// 1.f 되면 Success
		if (m_fQTEGuage == 1.f)
			m_isGoinSuccess = true;
		
	}
		
	//std::cout << "[UI_QTE::Update_QTE] Current QTE Guage : " << m_fQTEGuage << " / 1.0" << std::endl;

	if (!m_isGoinSuccess &&
		m_fQTEElapsedTime >= m_fQTEMaxTime)
		m_isGoinFail = true;

	
	if (m_isGoinSuccess || m_isGoinFail)		// 끝나는 조건 시 QTE 종료
	{
		m_isQTEMode = false;
		if (m_isGoinSuccess) m_pAnim_UI_SectorA_FG_Fillguage->Change_Animation(L"QTE1A_FG_Succeed");
		//m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeOut");
		return;
	}

	m_fQTEElapsedTime += fTimeDelta;
}

void CUI_QTE::Update_QTE_Trigger(_float fTimeDelta)
{
	if (!m_isQTEMode)					// QTE가 켜질 시 진행.
		return;

	_bool isTriggered = false;

#ifdef KSTA_UITEST_TEMPTRIGGER

	if (m_pGameInstance->Get_DIKeyState(m_arrBtnMapping[ENUM_CLASS(m_eIconIndex)]) == KEYSTATE::DOWN)		// ksta : 나중에 외부로부터 성공 여부 받아오기.
		isTriggered = true;
#endif // KSTA_UITEST_TEMPTRIGGER


	

	if (isTriggered)
		m_isGoinSuccess = true;

	if (!m_isGoinSuccess &&
		m_fQTEElapsedTime >= m_fQTEMaxTime)
		m_isGoinFail = true;

	if (m_isGoinSuccess || m_isGoinFail)		// 끝나는 조건 시 QTE 종료
	{
		m_isQTEMode = false;
		if (m_isGoinSuccess) m_pAnim_UI_SectorA_FG_Trigger->Change_Animation(L"QTE2A_FG_Triggered");
		//m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeOut");
		return;
	}

	m_fQTEElapsedTime += fTimeDelta;
}

void CUI_QTE::Update_FinishEvent(_float fTimeDelta)
{
	if (!(m_isGoinSuccess || m_isGoinFail))
		return;
	if (m_IsGoinDisabled)
		return;

	if		(m_isGoinSuccess)
	{
		//std::cout << "[UI_QTE::Update_FinishEvent] QTE Success Triggered!" << std::endl;
		if (m_eQTEType == UI_QTE_TYPE::FILLGUAGE)
			m_pGameSystem->Bind_Condition_ToPlayer("LeviatanQTESuccess");
		else if (m_eQTEType == UI_QTE_TYPE::TRIGGER_EXECUTE)
		{
			LEVI_EXECUTE Desc{ true };
			m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Execute"), Desc);
		}
		
		//m_isGoinSuccess = false;
		//m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), L"Event_QTESuccess", QTE_SUCCESS_UI_EVENT(m_isGoinSuccess));
	}
	else if (m_isGoinFail)
	{
		//std::cout << "[UI_QTE::Update_FinishEvent] QTE Fail Triggered!" << std::endl;

		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), L"Event_QTEFail", QTE_FAIL_UI_EVENT(m_isGoinFail));
	}

	m_IsGoinDisabled = true;
	m_pGameSystem->HUD_FadeIn();
}

void CUI_QTE::Update_GoinDisabled(_float fTimeDelta)
{
	if (!m_IsGoinDisabled)
		return;

	// 그냥 사라지는 이벤트 해도 되고, anim order 따라 순차저긍로 애니메이션 켜면서 진행하는 방법도 있음
	// 즉시 변해야 하는 애나메이션일 시 인자로 true붙이는 것 잊지 말 것

	array<_float, 2>	arrAnimTimings;

	switch (m_eQTEType)
	{
	case Client::UI_QTE_TYPE::FILLGUAGE:			arrAnimTimings = { 0.25f, 0.75f };		break;
	case Client::UI_QTE_TYPE::TRIGGER_ROPE:			
	case Client::UI_QTE_TYPE::TRIGGER_EXECUTE:		arrAnimTimings = { 1.f, 1.5f};			break;
	}


	if (	m_iDisableAnimOrder == 0 &&
			m_fDisableTimer >= arrAnimTimings[0])
	{
		m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeOut", true);
		m_iDisableAnimOrder++;
	}
	else if(m_iDisableAnimOrder == 1 &&
			m_fDisableTimer >= arrAnimTimings[1])
	{
		this->SetActivate(false);

		m_iDisableAnimOrder++;
	}

	m_fDisableTimer += fTimeDelta;
}


void CUI_QTE::Ready_Events()
{

}

CUI_QTE* CUI_QTE::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_QTE* pInstance = new CUI_QTE(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_QTE");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_QTE::Clone(void* pArg)
{
	CUI_QTE* pInstance = new CUI_QTE(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_QTE");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_QTE::Free()
{
	Safe_Release(m_pGameSystem);
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Interact");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
