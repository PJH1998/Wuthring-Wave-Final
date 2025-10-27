#include "EditorPch.h"
#include "Sequencer.h"

CSequencer::CSequencer()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CSequencer::Get(_int index, _int** start, _int** end, _int* type, _uint* color)
{
	if (0 == m_Items.size())
		return;

	SEQUENCE_ITEM& item = m_Items[index];
	if (nullptr != color)
		*color = 0xFFAA8080;
	if (nullptr != start)
		*start = &item.iFrameStart;
	if(nullptr != end)
		*end = &item.iFrameEnd;
	if (nullptr != type)
		*type = ENUM_CLASS(item.eType);
}

HRESULT CSequencer::Initialize()
{
	m_iFrameMin = 0;
	m_iFrameMax = 100;

	return S_OK;
}

void CSequencer::Update(_float fTimeDelta)
{
	ImGui::Begin("Sequence");

	ImGui::PushItemWidth(130);
	ImGui::Text("Frame : "); ImGui::SameLine(); ImGui::InputInt("##", &m_iCurrentFrame); ImGui::SameLine();
	ImGui::Text("/ Frame Min : ");  ImGui::SameLine(); ImGui::InputInt("##", &m_iFrameMin); ImGui::SameLine();
	ImGui::Text("/ Frame Max : ");  ImGui::SameLine(); ImGui::InputInt("##", &m_iFrameMax);
	ImGui::PopItemWidth();

	Drawing();

	ImGui::End();
}

void CSequencer::Drawing()
{
	// Get DrawList
	m_pDrawList = ImGui::GetWindowDrawList();
	ImVec2 vCanvasPos = ImGui::GetCursorScreenPos();			// ImDrawList는 Screen 좌표계 사용
	ImVec2 vCanvasSize = ImGui::GetContentRegionAvail();		// Canvas Size

	_int iControlHeight = m_Items.size() * m_iItemHeight;
	m_iFrameCnt = max(m_iFrameMax - m_iFrameMin, 1);

	ImGui::BeginGroup();

	const _int iVisibleFrameCnt = static_cast<_int>(floorf((vCanvasSize.x - m_iLegendWidth) / m_fFramePixelWidth));
	const _float fBarWidthRatio = min(iVisibleFrameCnt / static_cast<_float>(m_iFrameCnt), 1.f);
	const _float fBarWidthInPixels = fBarWidthRatio * (vCanvasSize.x - m_iLegendWidth);

	Panning(iVisibleFrameCnt);

	ImGui::EndGroup();
}

void CSequencer::Panning(const _int iVisibleFrameCnt)
{
	ImGuiIO& io = ImGui::GetIO();
	// Panning (Alt + Wheel Click -> Drag => 화면 좌우 이동)
	if (ImGui::IsWindowFocused() && m_pGameInstance->Get_DIKeyState(DIK_LALT) == KEYSTATE::PRESS && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::WB) == KEYSTATE::PRESS)
	{
		if (false == m_isPanningView)
		{
			m_vPanningViewSource = io.MousePos;
			m_isPanningView = true;
			m_iPanningViewFrame = m_iFirstFrame;
		}
		m_iFirstFrame = m_iPanningViewFrame - static_cast<_int>((io.MousePos.x - m_vPanningViewSource.x) / m_fFramePixelWidth);
		m_iFirstFrame = clamp(m_iFirstFrame, m_iFrameMin, m_iFrameMax - iVisibleFrameCnt);
	}
	// Panning Off
	if (true == m_isPanningView && m_pGameInstance->Get_DIKeyState(DIK_LALT) == KEYSTATE::UP)
		m_isPanningView = false;

	m_fFramePixelWidthTarget = clamp(m_fFramePixelWidthTarget, 0.1f, 50.f);
	m_fFramePixelWidth = ImLerp(m_fFramePixelWidth, m_fFramePixelWidthTarget, 0.33f);
	m_iFrameCnt = m_iFrameMax - m_iFrameMin;

	// 보여야 되는 Frame 개수가 총 Frame 개수보다 많으면 FirstFrame Min으로 고정
	if (iVisibleFrameCnt >= m_iFrameCnt)
		m_iFirstFrame = m_iFrameMin;
}

CSequencer* CSequencer::Create()
{
	CSequencer* pInstance = new CSequencer();

	if (FAILED(pInstance->Initialize()))
		CRASH("Sequence Create");

	return pInstance;
}

void CSequencer::Free()
{
	__super::Free();

	m_pDrawList = nullptr;

	Safe_Release(m_pGameInstance);
}
