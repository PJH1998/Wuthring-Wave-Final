#include "ClientPch.h"
#include "UI_Dialog.h"
#include "GameSystem.h"
#include "UI_Text.h"
#include "Animator_UI.h"

CUI_Dialog::CUI_Dialog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI( pDevice, pContext )
{
}

CUI_Dialog::CUI_Dialog(const CUI_Dialog& Prototype)
	: CCustom_UI( Prototype )
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_Dialog::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Dialog::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Dialog.json"; //확인
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	Create_ChildText_Speaker();
	Create_ChildText_Dialog();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Dialog_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Dialog_Mask_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Dialog_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Dialog_FadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);


	static_cast<CAnimator_UI*>(m_pUI_SectorA_Images->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Dialog_Initialize");
	static_cast<CAnimator_UI*>(Find_ChildObject(L"Bottom_Mask")->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Dialog_Mask_Initialize");

	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Dialog", this);

    return S_OK;
}

void CUI_Dialog::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_Dialog::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_DialogOrder(fTimeDelta);
	Update_DialogInstance(fTimeDelta);
	Update_GoinDisable(fTimeDelta);

	__super::Update(fTimeDelta);
}

void CUI_Dialog::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	m_fTickElapsedTime += fTimeDelta;
	__super::Late_Update(fTimeDelta);
}

void CUI_Dialog::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_Dialog::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	UI_DIALOG_DESC* pDesc = static_cast<UI_DIALOG_DESC*>(pArg);

	m_iDialogOrder = 0.f;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0.f;
	m_fTickElapsedTime = 0.f;

	m_vecDialogs.clear();
	m_isCurDialogFinished = false;

	if (pArg)
		Load_Dialog(pDesc->strFilePath.c_str());

	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Dialog_FadeIn"); //확인
	m_isActivate = true;
	m_isGoinDisable = false;
}

void CUI_Dialog::Load_Dialog(const _char* pFilePath)
{
	vector<vector<_string>> vecParsedData = m_pGameSystem->Load_CSV(pFilePath);

	for (auto& datas : vecParsedData)
	{
		for (auto& data : datas)
		{
			_string out;
			out.reserve(data.size());

			for (size_t i = 0; i < data.size(); ++i)
			{
				const char c = data[i];

				if (c == '\r')
					continue;

				if (c == '\\' && i + 1 < data.size())
				{
					const char n = data[i + 1];

					if (n == 'n') { out.push_back('\n'); ++i; continue; }
					else if (n == 't') { out.push_back('\t'); ++i; continue; }
					else if (n == '\\') { out.push_back('\\'); ++i; continue; }
					else if (n == '"') { out.push_back('"');  ++i; continue; }
					// else if (n == 'r') { out.push_back('\r'); ++i; continue; }
				}
				out.push_back(c);
			}
			data.swap(out);
		}
	}

	m_vecDialogs.clear();
	m_vecDialogs.reserve(vecParsedData.size());

	for (auto& row : vecParsedData)
	{
		DIALOG_DESC tDesc = { StringToWString(row[0]),	StringToWString(row[1]) };
		m_vecDialogs.push_back(tDesc);
	}
}

//void CUI_Dialog::Load_Dialog(const _char* pFilePath)
//{
//	vector<vector<_string>> vecParsedData = m_pGameSystem->Load_CSV(pFilePath);
//
//	m_vecDialogs.clear();
//	m_vecDialogs.reserve(vecParsedData.size());
//
//	for (auto& row : vecParsedData)
//	{
//		DIALOG_DESC tDesc = { StringToWString(row[0]),	StringToWString(row[1]) };
//		m_vecDialogs.push_back(tDesc);
//	}
//}


void CUI_Dialog::Change_Dialog(_uint iDialogIndex)
{
	if (m_vecDialogs.empty())
		CRASH("로드 된 대화가 없는디?");
	if (m_vecDialogs.size() <= iDialogIndex)
		CRASH("대화 갯수보다 더 큰 걸 불러오려고 시도하는디?");

	static_cast<CUI_Text*>(m_pTextUI_Dialog)->Change_Text(m_vecDialogs[iDialogIndex].strDialog);
	static_cast<CUI_Text*>(m_pTextUI_Speaker)->Change_Text(m_vecDialogs[iDialogIndex].strSpeaker);

	CUI_Text* pTargetText = dynamic_cast<CUI_Text*>(m_pTextUI_Dialog);
	
	auto& pDialogInst = pTargetText->Get_UIDesc().vecInstanceDescs;	// dialog 최초 전체 투명.
	for (_uint i = 0; i < pDialogInst.size(); i++)
		pDialogInst[i].matExtraData._11 = 1.f;

	m_fTickElapsedTime = 0.f;
}

HRESULT CUI_Dialog::Ready_Components(void* pArg)
{
    return S_OK;
}

void CUI_Dialog::PreAssign_ChildUIs()
{
	m_pRUI_All				= Find_ChildObject(L"Sub_All");
	m_pUI_SectorA_Images	= Find_ChildObject(L"SectorA_Images");
}

void CUI_Dialog::PreAssign_Presets()
{

}

HRESULT CUI_Dialog::Create_ChildText_Speaker()
{
	// 생성
	_float2 vTextPos = { 0.f, 150.f };
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"이친구가말을해요",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.5f,
		L"UI_Text_DialogSpeaker"
	);

	// 연결 및 중앙정렬
	CCustom_UI* pAttacher = m_pUI_SectorA_Images;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_Speaker = pFont;
	return S_OK;
}

HRESULT CUI_Dialog::Create_ChildText_Dialog()
{
	// 생성
	_float2 vTextPos = { 0.f, 250.f };
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"이친구가한말이에요",
		TEXT_COLOR_TYPE::TT_NORMAL,
		0.4f,
		L"UI_Text_DialogDialog"
	);

	// 연결 및 중앙정렬
	CCustom_UI* pAttacher = m_pUI_SectorA_Images;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	// 캐싱
	m_pTextUI_Dialog = pFont;
	return S_OK;
}

void CUI_Dialog::Update_DialogInstance(_float fTimeDelta)
{
	if (m_isCurDialogFinished)
		return;

	// 각각의 글자 인스턴스들을 서서히 나오도록 조정한다.

	CUI_Text* pTargetText = dynamic_cast<CUI_Text*>(m_pTextUI_Dialog);
	
	auto& pDialogInsts = pTargetText->Get_UIDesc().vecInstanceDescs;
	//for (_uint i = 0; i < pTargetText->Get_TextUIDesc().strText.length(); i++)
	//	pDialogInsts[i].matExtraData._11 = 0.f;


	// 1. 전역 elapsedtime 하나만 두고..
	// 2. 글자 갯수에 맞춰서 인터벌타임 및 글자별 변화시간 기준으로 완전히 대화 나올시간 계산하고
	// 3. 각 인스턴스마다의 투명도는 인덱스와 인터벌타임만으로도 계산이 가능하니 그렇게.
	//	 굳이 인스턴스마다 felapsedtime 같은거 들 필요 없음

	const _float fDialogFinishTime = m_fInstIntervalTime * pDialogInsts.size() + m_fInstFadeInTime;
	
	for (_uint i = 0; i < pDialogInsts.size(); i++)
	{
		const _float fInstFadeStartTime = m_fInstIntervalTime * i;
		_float fInstAlpha = 1.f - Clamp(SmoothStep(fInstFadeStartTime, fInstFadeStartTime + m_fInstFadeInTime, m_fTickElapsedTime), 0.f, 1.f);
		

		pDialogInsts[i].matExtraData._11 = (pDialogInsts[i].matExtraData._11 >= fInstAlpha)?		// 계산값보다 지금이 더 투명함?
			pDialogInsts[i].matExtraData._11 : fInstAlpha;
	}

	if (fDialogFinishTime <= m_fTickElapsedTime &&
		!m_isCurDialogFinished)
		m_isCurDialogFinished = true;
}

void CUI_Dialog::Update_DialogOrder(_float fTimeDelta)
{
	// 글자가 전부 보이는 상태에서, 클릭이나 엔터, F 등 상호작용 시 다음으로 넘어가도록.
	// 만약 아직 전부 보이지 않는 상태에서 상호작용 시도 시 바로 다 보이게.
	
	CUI_Text* pTargetText = dynamic_cast<CUI_Text*>(m_pTextUI_Dialog);

	if (m_fTickElapsedTime == 0)
		Change_Dialog(m_iDialogOrder);

	// Interacted when dialog On.
	if (m_pGameInstance->Get_DIKeyState(DIK_SPACE)			 == KEYSTATE::DOWN ||
		m_pGameInstance->Get_DIKeyState(DIK_F)				 == KEYSTATE::DOWN ||
		m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
	{
		if (m_isCurDialogFinished &&
			m_iDialogOrder == m_vecDialogs.size() - 1)	// 마지막 대화 순서. 이러면 종료해야.
		{
			Req_Close_Dialog();
		}
		else if (m_isCurDialogFinished)					// 현재 대화까지 끝. 다음 대화로..
		{
			m_iDialogOrder++;
			m_isCurDialogFinished = false;
			Change_Dialog(m_iDialogOrder);
		}
		else											// 현재 대화 진행중. 현재 대화부터 마치기.
		{
			auto& pDialogInst = pTargetText->Get_UIDesc().vecInstanceDescs;
			for (_uint i = 0; i < pDialogInst.size(); i++)
				pDialogInst[i].matExtraData._11 = 0.f;

			m_isCurDialogFinished = true;
		}
	}
}

void CUI_Dialog::Update_GoinDisable(_float fTimeDelta)
{
	if (!m_isGoinDisable)
		return;

	// 나중에 서서히 사라지던가 할 때

	if (m_fDisableTimer == 0.f)	// Fade Out 시작.
		static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Dialog_FadeOut");

	if (m_fDisableTimer >= m_fDisableTime)
		m_isActivate = false;

	m_fDisableTimer += fTimeDelta;
}

CUI_Dialog* CUI_Dialog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Dialog* pInstance = new CUI_Dialog(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Dialog");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Dialog::Clone(void* pArg)
{
	CUI_Dialog* pInstance = new CUI_Dialog(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Dialog");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Dialog::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Dialog");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}