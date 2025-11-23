#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_TabUtility.h"

#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만
#define	 FLOAT2_LENGTH(x)								(XMVectorGetX(XMVector2Length(XMLoadFloat2(x))))


// 생각할 것 : 탭중에는 커서락 해제되어야 함

CUI_TabUtility::CUI_TabUtility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_TabUtility::CUI_TabUtility(const CUI_TabUtility& Prototype)
	: CCustom_UI(Prototype)
{
}

HRESULT CUI_TabUtility::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_TabUtility::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_TabUtility.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hide.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Hide.json",
	};
	Load_Animations(vecAnimFilePaths);


	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Initialize");
	static_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Initialize");

	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_TabUtility", this);

	return S_OK;
}

void CUI_TabUtility::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_TabUtility::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_MouseSelection();
	Update_GoinDisable(fTimeDelta);


	__super::Update(fTimeDelta);
}

void CUI_TabUtility::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_TabUtility::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_TabUtility::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	static_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Initialize", true);
	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Show", true);

	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;
	m_isActivate = true;
	m_isFirstCheckedIndex = false;
}

void CUI_TabUtility::Update_MouseSelection()
{
	POINT PointMousePos = m_pGameInstance->Get_MousePoint();
	_float2 vMousePos = {							// Center Aligned.
		PointMousePos.x - g_iWinSizeX * 0.5f,
		PointMousePos.y - g_iWinSizeY * 0.5f
	};


	
	// 1. 커서 위치에 따라 Arrow 도 같이 돌아가야 함. 이는 중앙 대비 커서 위치에 따라 단순히 m_pUI_Arrow 를 돌리기만 하면 될 것.
	// 2. 커서가 중앙으로부터 일정 거리 벗어나면, SelectedIndex 가 갱신되어야 함. (기본값 UINT_MAX)
	// 3. SelectedIndex 가 갱신될 떄에, Hover 또한 애니메이션이 재생되어야 하며, 해당 인덱스에 맞는 인스턴스만이 보여야 함
	
	//	 근데 다시 중앙에 돌아오면, SelectedIndex 는 다시 기본값으로 돌아가며, 이 상태로 끄면 선택한 장비는 현행유지여야 함.
	

	// [1] 커서 위치에 따른 Arrow 회전
	// Z 값이 0이면 12시. 90이면 9시와 같이 반시계방향으로 돌아감.
	_float fArrowAngle = atan2f(vMousePos.x, vMousePos.y) + DegreesToRadians(180.f);	// counterclockwise, rad
	m_pTransformCom_UIArrow->Rotation(XMVectorSet(0.f, 0.f, 1.f, 1.f), fArrowAngle);


	// [2] 커서 방향지정에 따른 선택 Util 갱신
	_float fAngleDeg = RadiansToDegrees(fArrowAngle);
	if		(fAngleDeg > 360.f)			fAngleDeg -= 360.f;
	else if (fAngleDeg <= 0.f)			fAngleDeg += 360.f;

	// 만약 중앙에서 일정 벗어나있다면 인덱스 지정
	_uint iPrevSelectedIndex = m_iSelectedIndex;
	_bool isIndexChanged = false;
	_float fDistFromCenter = FLOAT2_LENGTH(&vMousePos);
	if (fDistFromCenter >= 200.f)
	{
		if		(IS_BETWEEN(fAngleDeg, 0.f, 45.f) ||
				 IS_BETWEEN(fAngleDeg, 315.f, 360.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::FLIGHT);		// 2
		else if (IS_BETWEEN(fAngleDeg, 45.f, 135.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::SENSOR);		// 1
		else if (IS_BETWEEN(fAngleDeg, 135.f, 225.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::GRAPPLE);		// 0
		else if (IS_BETWEEN(fAngleDeg, 225.f, 315.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::LEVITATOR);	// 3
	}
	else
		m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::NOTHING);

	if (iPrevSelectedIndex != m_iSelectedIndex)
		isIndexChanged = true;

	// [3] Util 갱신에 따른 애니메이션 재생3 및 인스턴스 선택
	if ((!m_isFirstCheckedIndex && (fDistFromCenter >= 200.f)) ||							// 최초 1회 커서위치에 따른 확인
		isIndexChanged && !(m_iSelectedIndex == ENUM_CLASS(UI_TAB_UTILITY::NOTHING)))		// 다른 무언가로 선택이 바뀜.
	{
		m_isFirstCheckedIndex = true;

		dynamic_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Show", true);
		
		auto targetDesc = m_pUI_InstHover->Get_UIDesc();
		auto& targetInstDesc = targetDesc.vecInstanceDescs;

		for (_uint i = 0; i < targetInstDesc.size(); i++)		// 현재 인덱스에 해당하는 인스턴스만 보이게 하고, 나머지는 가림.
		{
			targetInstDesc[i].vClipTexcoordX = (m_iSelectedIndex == i)? 
				_float2{ 0.f, 1.f } : 
				_float2{ 0.f, 0.f };
		}

		m_pUI_InstHover->Set_UIDesc(targetDesc);
	}
	else if (isIndexChanged && (m_iSelectedIndex == ENUM_CLASS(UI_TAB_UTILITY::NOTHING)))	// 선택 해제함 (커서가 화면 중앙으로 감)
		dynamic_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Hide");
}

void CUI_TabUtility::Update_GoinDisable(_float fTimeDelta)
{
	if (!m_IsGoinDisabled)
		return;

	const _float fMaxDisableTimer = 0.5f;
	
	// 시간 충분히 경과시 inactive.
	m_fDisableTimer += fTimeDelta;
	if (m_fDisableTimer >= fMaxDisableTimer)
	{
		m_isActivate = false;
		m_iAnimOrder = 0;
		m_fDisableTimer = 0.f;

		return;
	}

	// 시간 경과 중 애니메이션 재생 (disable 시작 시 1회)
	if (m_iAnimOrder == 0)
	{
		static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hide");
		m_iAnimOrder++;
	}
}

HRESULT CUI_TabUtility::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_TabUtility::PreAssign_ChildUIs()
{
	m_pRUI_All			= Find_ChildObject(L"Sub_All");
	m_pUI_Back			= Find_ChildObject(L"SectorA_Back");
	m_pUI_Hover			= Find_ChildObject(L"SectorA_Hover");
	m_pUI_Arrow			= Find_ChildObject(L"SectorA_Arrow");

	m_pUI_InstHover		= Find_ChildObject(L"Switch_Hover");

	m_pTransformCom_UIArrow = dynamic_cast<CTransform*>(m_pUI_Arrow->Get_Component(L"Com_Transform"));
}

void CUI_TabUtility::Create_ChildText()
{
	// 나중에 현재 선택된 게 뭔지 텍스트도 추가? 
}

CUI_TabUtility* CUI_TabUtility::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_TabUtility* pInstance = new CUI_TabUtility(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_TabUtility");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_TabUtility::Clone(void* pArg)
{
	CUI_TabUtility* pInstance = new CUI_TabUtility(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_TabUtility");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_TabUtility::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_TabUtility");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
