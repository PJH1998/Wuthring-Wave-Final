// Single LockOn 

#include "ClientPch.h"
#include "UI_GrafflePoint.h"

#include "GameSystem.h"
#include "Animator_UI.h"

#define KSTA_UITEST_GRAFFLE_TOZERO
#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만


CUI_GrafflePoint::CUI_GrafflePoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_GrafflePoint::CUI_GrafflePoint(const CUI_GrafflePoint& Prototype)
	: CUI_Image(Prototype)
	//, m_pGameSystem (CGameSystem::GetInstance())
{
	//Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_GrafflePoint::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_GrafflePoint::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_GrafflePoint.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Static_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Static_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Static_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Dynamic_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Dynamic_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Dynamic_FadeOut.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Graffle_Dynamic_TickLoop.json",
	};
	Load_Animations(vecAnimFilePaths);


	m_pSubAnimUI	->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	m_pStaticAnimUI	->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	m_pDynamicAnimUI->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));

	//static_cast<CAnimator_UI*>(m_pDynamicUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Graffle_Dynamic_Initialize");

	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	//m_pGameInstance->Add_RootUI(L"UI_GrafflePoint", this);


	return S_OK;
}

void CUI_GrafflePoint::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	__super::Priority_Update(fTimeDelta);
}

void CUI_GrafflePoint::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

#ifdef KSTA_UITEST_GRAFFLE_TOZERO
	if (!m_DEBUG_isAssignedPosition)
	{
		m_DEBUG_isAssignedPosition = true;
		
		const _float fDEBUG_randRadius = 30.f;//30.f;
		const _float3 vDEBUG_offset = { 0.f, -10.f, 0.f };
		
		_float3* pDEBUG_vTargetPos = new _float3();
		
		pDEBUG_vTargetPos->x = vDEBUG_offset.x + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		pDEBUG_vTargetPos->y = vDEBUG_offset.y + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		pDEBUG_vTargetPos->z = vDEBUG_offset.z + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);

		m_pTargetPos = pDEBUG_vTargetPos;
	}
#endif // KSTA_UITEST_GRAFFLE_TOZERO




	Update_AnimOrder(fTimeDelta);

	Update_ApplyTargetPos(m_pStaticUI, *m_pTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.
	Update_ApplyTargetPos(m_pDynamicUI, *m_pTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.

	__super::Update(fTimeDelta);
}

void CUI_GrafflePoint::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	if (m_isUnvisible)
		return;

	Update_CamDistScale(m_pStaticUI, m_fPivotDistance);
	Update_CamDistScale(m_pDynamicUI, m_fPivotDistance);

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_GrafflePoint::Render()
{
	if (!m_isActivate)
		return;


}

void CUI_GrafflePoint::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	static_cast<CAnimator_UI*>(m_pStaticUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Graffle_Initialize", true);

	if (pArg != nullptr)
		m_pTargetPos = static_cast<UI_GRAFFLEPOINT_DESC*>(pArg)->pTargetPos;
#ifndef KSTA_UITEST_GRAFFLE_TOZERO
	else
		MSG_BOX("GrafflePoint doesn't receive position information.");
#endif // !KSTA_UITEST_GRAFFLE_TOZERO


	m_isActivate = true;
}

void CUI_GrafflePoint::PreAssign_ChildUIs()
{
	m_pRUI_All		= Find_ChildObject(L"Sub_All");
	m_pStaticUI		= Find_ChildObject(L"SectorA_Static");
	m_pDynamicUI	= Find_ChildObject(L"SectorA_Dynamic");

	m_pSubAnimUI	= dynamic_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"));
	m_pStaticAnimUI = dynamic_cast<CAnimator_UI*>(m_pStaticUI->Get_Component(L"Com_Animator_UI"));
	m_pDynamicAnimUI = dynamic_cast<CAnimator_UI*>(m_pDynamicUI->Get_Component(L"Com_Animator_UI"));
}

void CUI_GrafflePoint::Ready_Presets()
{

}

void CUI_GrafflePoint::Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos)
{
	const _matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	const _matrix matCamProj = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);

	const _float2 vScreenSize = { g_iWinSizeX, g_iWinSizeY };
	_vector vTargetWorldPos = XMVectorSetW(XMLoadFloat3(&vTargetPos), 1.0f);

	_matrix matViewProj = matCamView * matCamProj;
	_vector vTargetClipRaw = XMVector3Transform(vTargetWorldPos, matViewProj);

	_float fTargetW = XMVectorGetW(vTargetClipRaw);
	_bool isBehindCamera = (fTargetW <= 0.0f);

	_float2 vScreenPos = {};

	if (!isBehindCamera)
	{
		_vector vTargetNDC = XMVector3TransformCoord(vTargetWorldPos, matViewProj);

		vScreenPos.x = (XMVectorGetX(vTargetNDC) + 1.0f) * 0.5f * vScreenSize.x - vScreenSize.x * 0.5f;
		vScreenPos.y = (1.0f - XMVectorGetY(vTargetNDC)) * 0.5f * vScreenSize.y - vScreenSize.y * 0.5f;
	}
	else
		vScreenPos = { -2000.f, -2000.f }; // 카메라 뒤면 밖으로 쫒아냄

	_vector vPos = XMVectorSet(vScreenPos.x, -vScreenPos.y, 0.f, 1.f);
	static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"))->Set_State(STATE::POSITION, vPos);
}

void CUI_GrafflePoint::Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"));

	_float3 vScale =/* (pTargetUI == this)? m_vOriginSca :*/ pTargetTransform->Get_Scaled();

	_float3 vTargetPos = *m_pTargetPos;				// ksta : 테스트용, 나중에 수정. 받아온 타겟 좌표로.
	_float fDist = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - XMLoadFloat3(&vTargetPos)));
	_float fScaleMultiple = fPivotDistance / fDist;

	_float3 vFinalScale = _float3{
		vScale.x * fScaleMultiple,
		vScale.y * fScaleMultiple,
		vScale.z * fScaleMultiple
	};

	pTargetTransform->Scale(vFinalScale);
}

void CUI_GrafflePoint::Update_AnimOrder(_float fTimeDelta)
{
	_float4 vCamPos = *m_pGameInstance->Get_CamPos();
	_float fDistance = XMVectorGetX(XMVector3Length((XMLoadFloat3(m_pTargetPos) - XMLoadFloat4(&vCamPos))));	// 카메라와 타겟 간 거리


	// 이전 상태 확인 후 트리거 분기 및 사용, 이후 상태 갱신
	
	// 1. 현재 상태 확인
	if		(IS_BETWEEN(fDistance, 0.f, m_fTriggerDistance))				
		m_eCurDistState = INNER;
	else if (IS_BETWEEN(fDistance, m_fTriggerDistance, m_fVisibleDistance))
		m_eCurDistState = OUTER;
	else
		m_eCurDistState = UNVISIBLE;
	
	// 2. 전후 상태전환 비교에 따른 분기
	if		(m_ePrevDistState == UNVISIBLE	&& m_eCurDistState == OUTER)		{ m_eTriggerState = SEMIENTER; }		// 보이는 범위로 진입
	else if (m_ePrevDistState == OUTER		&& m_eCurDistState == INNER)		{ m_eTriggerState = ENTER; }			// 상호작용 가능한 범위
	else if (m_ePrevDistState == INNER		&& m_eCurDistState == OUTER)		{ m_eTriggerState = EXIT; }
	else if (m_ePrevDistState == OUTER		&& m_eCurDistState == UNVISIBLE)	{ m_eTriggerState = SEMIEXIT; }
	else																		{ m_eTriggerState = NONE; }
	
	// 3. 일정 거리를 조건으로 애니메이션 분기 진행
	switch (m_eTriggerState)
	{
	case CUI_GrafflePoint::ENTER:		m_pSubAnimUI->Change_Animation(L"Graffle_FadeIn", true);						
										m_pStaticAnimUI->Change_Animation(L"Graffle_Static_FadeIn", true);				break;
	case CUI_GrafflePoint::EXIT:		m_pSubAnimUI->Change_Animation(L"Graffle_FadeOut", true);						
										m_pStaticAnimUI->Change_Animation(L"Graffle_Static_FadeOut", true);				break;
	case CUI_GrafflePoint::SEMIENTER:	m_pDynamicAnimUI->Change_Animation(L"Graffle_Dynamic_TickLoop", true);			break;
	case CUI_GrafflePoint::SEMIEXIT:	m_pDynamicAnimUI->Change_Animation(L"Graffle_Dynamic_FadeOut", true);			break;
	//case CUI_GrafflePoint::NONE:		
	//default:							m_pSubAnimUI->Change_Animation(L"Graffle_Initialize", true);
	//									m_pDynamicAnimUI->Change_Animation(L"Graffle_Dynamic_Initialize", true);		break;
	}

	// 4. 비교용 이전상태 갱신
	m_ePrevDistState = m_eCurDistState;
	
	// 5. 완전히 투명해지면 렌더X
	if (m_eCurDistState == UNVISIBLE)
	{
		if (!m_isUnvisibleStandby && !m_isUnvisible)	// 최초 진입 시도
		{
			m_isUnvisibleStandby = true;
			m_fGoinUnvisibleTime = 0.f;
		}
		if (m_isUnvisibleStandby)						// 진입중. 타이머 적용
		{
			m_fGoinUnvisibleTime += fTimeDelta;
			if (m_fGoinUnvisibleTime >= m_fUnvisibledTime)	// 진입 완료. 렌더 끔
			{
				m_isUnvisible = true;
				m_isUnvisibleStandby = false;
				m_fGoinUnvisibleTime = 0.f;
			}
		}
	}
	else												// 진입 X
	{
		m_isUnvisible = false;
		m_isUnvisibleStandby = false;
		m_fGoinUnvisibleTime = 0.f;
	}

}

CUI_GrafflePoint* CUI_GrafflePoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_GrafflePoint* pInstance = new CUI_GrafflePoint(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_GrafflePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_GrafflePoint::Clone(void* pArg)
{
	CUI_GrafflePoint* pInstance = new CUI_GrafflePoint(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_GrafflePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_GrafflePoint::Free()
{
#ifdef KSTA_UITEST_GRAFFLE_TOZERO
	delete m_pTargetPos;
#endif // !KSTA_UITEST_GRAFFLE_TOZERO

	//if (m_isClone)
	//	m_pGameInstance->Remove_RootUI(L"UI_GrafflePoint");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
