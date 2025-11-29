
#include "ClientPch.h"
#include "UI_QTE.h"

#include "Animator_UI.h"
#include "GameSystem.h"

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
	__super::Ready_Events();
	
	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_QTE1.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeIn.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeOut.json",			
		

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Start.json",			

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_Assemble_AlphaStrength.json",			


		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Start.json",			


		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_FadeIn.json",	


		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FG_Arrow_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FG_Arrow_LickLoop.json",	


		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_FeedbackRing_Initialize.json",	
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_BG_FeedbackRing_TickLoop.json",	
	};
	Load_Animations(vecAnimFilePaths);
	
	Reset(_fmatrix(), pArg);
	m_isActivate = false;

	m_pAnim_RUI_All->Change_Animation(L"QTE1_Initialize");
	m_pAnim_UI_SectorA_KeyGuide->Change_Animation(L"QTE1A_KeyGuide_Initialize");
	m_pAnim_UI_SectorA_BG->Change_Animation(L"QTE1A_BG_Initialize");
	m_pAnim_UI_SectorA_FG->Change_Animation(L"QTE1A_FG_Initialize");

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
	Update_QTE(fTimeDelta);					// m_isQTEMode

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

	_float4 vPosition = { pDesc->vSpawnPos.x, pDesc->vSpawnPos.y, 0.f, 1.f };
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vPosition));

	m_fQTEGuage = 0.f;
	m_fQTEElapsedTime = 0.f;
	m_fElapsedTime = 0.f;
	m_isQTEMode = false;

	m_iAnimOrder = 0;
	m_fElapsedTime = 0.f;

	m_pAnim_RUI_All->Change_Animation(L"QTE1_Initialize");
	m_pAnim_UI_SectorA_KeyGuide->Change_Animation(L"QTE1A_KeyGuide_Initialize");
	m_pAnim_UI_SectorA_BG->Change_Animation(L"QTE1A_BG_Initialize");
	m_pAnim_UI_SectorA_FG->Change_Animation(L"QTE1A_FG_Initialize");
	m_pAnim_UI_FG_QTEArrow->Change_Animation(L"QTE1_FG_Arrow_Initialize");
	m_pAnim_UI_FG_QTEFeedbackRing->Change_Animation(L"QTE1_BG_FeedbackRing_Initialize");
	

	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iDisableAnimOrder = 0;

	m_isGoinSuccess = false;
	m_isGoinFail = false;

	m_isActivate = true;
}

void CUI_QTE::PreAssign_ChildUIs()
{
	m_pRUI_All						= Find_ChildObject(L"Sub_All");
	m_pUI_SectorA_KeyGuide			= Find_ChildObject(L"SectorA_KeyGuide");
	m_pUI_SectorA_BG				= Find_ChildObject(L"SectorA_BG");
	m_pUI_SectorA_FG				= Find_ChildObject(L"SectorA_FG");

	m_pAnim_RUI_All					= dynamic_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_KeyGuide		= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_KeyGuide->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_BG			= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_BG->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_SectorA_FG			= dynamic_cast<CAnimator_UI*>(m_pUI_SectorA_FG->Get_Component(L"Com_Animator_UI"));
	
	m_pUI_KeyButtons				= Find_ChildObject(L"KeyButtons");
	m_pUI_BG_QTEFrame				= Find_ChildObject(L"QTE_Frame");
	m_pUI_BG_QTEAssemble			= Find_ChildObject(L"QTE_Assemble");
	m_pUI_FG_QTEGuageFrame			= Find_ChildObject(L"QTE_GuageFrame");
	m_pUI_FG_QTEGuage				= Find_ChildObject(L"QTE_Guage");
	m_pUI_FG_QTEArrow				= Find_ChildObject(L"QTE_Arrow");
	m_pUI_FG_QTEFeedbackRing		= Find_ChildObject(L"QTE_FeedbackRing");

	m_pAnim_UI_BG_QTEAssemble		= dynamic_cast<CAnimator_UI*>(m_pUI_BG_QTEAssemble->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_FG_QTEArrow			= dynamic_cast<CAnimator_UI*>(m_pUI_FG_QTEArrow->Get_Component(L"Com_Animator_UI"));
	m_pAnim_UI_FG_QTEFeedbackRing	= dynamic_cast<CAnimator_UI*>(m_pUI_FG_QTEFeedbackRing->Get_Component(L"Com_Animator_UI"));
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
		m_pAnim_UI_FG_QTEArrow->Change_Animation(L"QTE1_FG_Arrow_LickLoop");
		m_pAnim_UI_FG_QTEFeedbackRing->Change_Animation(L"QTE1_BG_FeedbackRing_TickLoop");

		m_iAnimOrder++;
	}
	else if(m_iAnimOrder == 1 &&
			m_fElapsedTime >= arrAnimOrderTimings[1])	// 0.25f
	{
		m_pAnim_UI_SectorA_FG->Change_Animation(L"QTE1A_FG_Start") ;

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

	auto guageDesc = m_pUI_FG_QTEGuage->Get_UIDesc();
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

void CUI_QTE::Update_QTE(_float fTimeDelta)
{
	if (!m_isQTEMode)					// QTE가 켜질 시 00진행.
		return;

	// 1. 일정 시간마다 떨어진다.
	
	_float fDropAmount = fTimeDelta * m_fQTEDropRate;
	m_fQTEGuage = ((m_fQTEGuage - fDropAmount) >= 0.f) ? m_fQTEGuage - fDropAmount : 0.f;

	// 2. 지정된 버튼을 누를 시, 해당 버튼이 눌렸을 때에
	// 피드백 효과, 게이지 상승, 다 찼는지의 판별 여부 등을 진행한다. 앨단 F.
	
	if (m_pGameInstance->Get_DIKeyState(DIK_F) == KEYSTATE::DOWN)
	{
		m_fQTEGuage = ((m_fQTEGuage + m_fQTEFillAmount) <= 1.f) ? m_fQTEGuage + m_fQTEFillAmount : 1.f;			// 1.f 되면 Success
		if (m_fQTEGuage == 1.f)
			m_isGoinSuccess = true;
		
	}
		
	std::cout << "[UI_QTE::Update_QTE] Current QTE Guage : " << m_fQTEGuage << " / 1.0" << std::endl;

	if (!m_isGoinSuccess &&
		m_fQTEElapsedTime >= m_fQTEMaxTime)
		m_isGoinFail = true;

	
	if (m_isGoinSuccess || m_isGoinFail)		// 끝나는 조건 시 QTE 종료
	{
		m_isQTEMode = false;
		m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeOut");
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
		std::cout << "[UI_QTE::Update_FinishEvent] QTE Success Triggered!" << std::endl;
	}
	else if (m_isGoinFail)
	{
		std::cout << "[UI_QTE::Update_FinishEvent] QTE Fail Triggered!" << std::endl;
	}

	m_IsGoinDisabled = true;
}

void CUI_QTE::Update_GoinDisabled(_float fTimeDelta)
{
	if (!m_IsGoinDisabled)
		return;

	// 그냥 사라지는 이벤트 해도 되고, anim order 따라 순차저긍로 애니메이션 켜면서 진행하는 방법도 있음
	// 즉시 변해야 하는 애나메이션일 시 인자로 true붙이는 것 잊지 말 것
	
	if (	m_iDisableAnimOrder == 0 &&
			m_fDisableTimer >= 0.f)
	{
		m_pAnim_RUI_All->Change_Animation(L"QTE1_FadeOut", true);
		m_iDisableAnimOrder++;
	}
	else if(m_iDisableAnimOrder == 1 &&
			m_fDisableTimer >= 0.5f)
	{
		this->SetActivate(false);

		m_iDisableAnimOrder++;
	}

	m_fDisableTimer += fTimeDelta;
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
