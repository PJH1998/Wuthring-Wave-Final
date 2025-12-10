// Single GrapplePoint

#include "ClientPch.h"
#include "UI_GrapplePoint.h"

#include "GameSystem.h"
#include "Animator_UI.h"

#define KSTA_UITEST_GRAPPLE_TOZERO  
#define	IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만


CUI_GrapplePoint::CUI_GrapplePoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_GrapplePoint::CUI_GrapplePoint(const CUI_GrapplePoint& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem (CGameSystem::GetInstance())
{
	//Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_GrapplePoint::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_GrapplePoint::Initialize_Clone(void* pArg)
{
	UI_GRAPPLEPOINT_DESC* pDesc = static_cast<UI_GRAPPLEPOINT_DESC*>(pArg);
	
	m_vTargetPos = pDesc->vTargetPos;
	m_eGrappleType = pDesc->eType;


	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_GrapplePoint.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Static_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Static_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Static_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Dynamic_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Dynamic_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Dynamic_FadeOut.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Grapple_Dynamic_TickLoop.json",
	};
	Load_Animations(vecAnimFilePaths);


	m_pSubAnimUI	->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	m_pStaticAnimUI	->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	m_pDynamicAnimUI->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));

	//static_cast<CAnimator_UI*>(m_pDynamicUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Grapple_Dynamic_Initialize");

	Reset(_fmatrix(), nullptr);
	m_isActivate = true;

	m_isClone = true;
	//m_pGameInstance->Add_RootUI(L"UI_GrapplePoint", this);


	return S_OK;
}

void CUI_GrapplePoint::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	m_pWorldTransformCom->Save_PreviousPosition();

	__super::Priority_Update(fTimeDelta);
}

void CUI_GrapplePoint::Update(_float fTimeDelta)
{
	Update_ToggleReqedEvent(fTimeDelta);

	if (!m_isActivate)
		return;

#ifdef KSTA_UITEST_GRAPPLE_TOZERO
	if (!m_DEBUG_isAssignedPosition)
	{
		m_DEBUG_isAssignedPosition = true;
		
		const _float fDEBUG_randRadius = 30.f;//30.f;
		const _float3 vDEBUG_offset = { 0.f, -10.f, 0.f };
		
		//_float3* pDEBUG_vTargetPos = new _float3();
		//
		//pDEBUG_vTargetPos->x = vDEBUG_offset.x + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		//pDEBUG_vTargetPos->y = vDEBUG_offset.y + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		//pDEBUG_vTargetPos->z = vDEBUG_offset.z + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		
		//_float3 vDEBUG_TargetPos = {};
		//vDEBUG_TargetPos.x = vDEBUG_offset.x + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		//vDEBUG_TargetPos.y = vDEBUG_offset.y + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		//vDEBUG_TargetPos.z = vDEBUG_offset.z + m_pGameInstance->Rand(-fDEBUG_randRadius, fDEBUG_randRadius);
		//
		//m_vTargetPos = vDEBUG_TargetPos;
	}
#endif // KSTA_UITEST_GRAPPLE_TOZERO



	Update_TargetColor();
	Update_AnimOrder(fTimeDelta);

	Update_ApplyTargetPos(m_pStaticUI, m_vTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.
	Update_ApplyTargetPos(m_pDynamicUI, m_vTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.

	__super::Update(fTimeDelta);

	// from ropeanchor..
	// 1. 플레이어 카메라 범위 안에 들어가 있으면서 거리도 적절하다면?
	if (nullptr != m_pTargetTransformCom)
	{
		_vector vMyPos = m_pWorldTransformCom->Get_State(STATE::POSITION);
		_vector vTargetPos = m_pTargetTransformCom->Get_State(STATE::POSITION);
		m_fTargetDistance = XMVectorGetX(XMVector3Length(vMyPos - vTargetPos));
	}
	// 2. RigidBodyCom 업데이트
	m_pRigidbodyCom->Update_Rigidbody(m_pWorldTransformCom->Get_WorldMatrix(), fTimeDelta);
	// Last => TargetTransform 비우기?
	m_pTargetTransformCom = nullptr;
}

void CUI_GrapplePoint::Late_Update(_float fTimeDelta)
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

void CUI_GrapplePoint::Render()
{
	if (!m_isActivate)
		return;

#ifdef _DEBUG
	m_pRigidbodyCom->Render();
#endif // DEBUG

}

void CUI_GrapplePoint::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 1. Detect 감지되면?
	if (ENUM_CLASS(COLLISIONLAYER::PLAYER) != iLayer)
		return;

	// 2. CallBack 정보 가져오기
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
	if (nullptr == pTargetTransform)
		return;
	{
		lock_guard<mutex> lock(m_Mutex);
		m_pTargetTransformCom = pTargetTransform;
	}
}

HRESULT CUI_GrapplePoint::Ready_Components(void* pArg)
{
	__super::Ready_Components(pArg);

	UI_GRAPPLEPOINT_DESC* pDesc = static_cast<UI_GRAPPLEPOINT_DESC*>(pArg);

	// Additional Transform (Based On World)
	m_pWorldTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (FAILED(m_pWorldTransformCom->Initialize_Clone(pArg)))
		return E_FAIL;
	m_Components.emplace(TEXT("Com_WorldTransform"), m_pWorldTransformCom);
	Safe_AddRef(m_pWorldTransformCom);

	m_pWorldTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));


	// Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::GRAPPLE);
	RigidbodyDesc.vExtent = _float3(1.5f, 1.5f, 1.5f); // 탐지 범위 안에 들어가있다면?
	RigidbodyDesc.vPos = m_vTargetPos;

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
	});

	// Transform과 Rope_Anchor 타입임을 알립니다.
	m_CallBack.pTransform = m_pWorldTransformCom;
	if		(pDesc->eType == UI_GRAPPLE_TYPE::ANCHOR)	m_CallBack.eObjectType = OBJECTTYPE::ROPE_ANCHOR;
	else if (pDesc->eType == UI_GRAPPLE_TYPE::PULL)		m_CallBack.eObjectType = OBJECTTYPE::ROPE_PULL;
	m_CallBack.pCondition = &m_iCondition;

	m_pRigidbodyCom->Set_Desc(&m_CallBack);
	return S_OK;
}

//void CUI_GrapplePoint::Reset(const _fmatrix& WorldMatrix, void* pArg)
//{
//	static_cast<CAnimator_UI*>(m_pStaticUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Grapple_Initialize", true);
//
//	if (pArg != nullptr)
//	{
//		UI_GRAPPLEPOINT_DESC* pDesc = static_cast<UI_GRAPPLEPOINT_DESC*>(pArg);
//
//		m_vTargetPos = pDesc->vTargetPos;
//		m_eGrappleType = pDesc->eType;
//	}
//
//
//#ifndef KSTA_UITEST_GRAPPLE_TOZERO
//	else
//		MSG_BOX("GrapplePoint doesn't receive position information.");
//#endif // !KSTA_UITEST_GRAPPLE_TOZERO
//
//
//	m_isActivate = true;
//}

void CUI_GrapplePoint::PreAssign_ChildUIs()
{
	m_pRUI_All		= Find_ChildObject(L"Sub_All");
	m_pStaticUI		= Find_ChildObject(L"SectorA_Static");
	m_pDynamicUI	= Find_ChildObject(L"SectorA_Dynamic");

	m_pSubAnimUI	= dynamic_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"));
	m_pStaticAnimUI = dynamic_cast<CAnimator_UI*>(m_pStaticUI->Get_Component(L"Com_Animator_UI"));
	m_pDynamicAnimUI= dynamic_cast<CAnimator_UI*>(m_pDynamicUI->Get_Component(L"Com_Animator_UI"));

	m_pUIGrapplePoint	= Find_ChildObject(L"GrapplePoint");
	m_pUIGrappleOutline	= Find_ChildObject(L"GrappleOutline");
}

void CUI_GrapplePoint::Ready_Presets()
{
	arrTypeColors[ENUM_CLASS(UI_GRAPPLE_TYPE::ANCHOR)]		= _float4(1.000f, 0.957f, 0.631f, 1.0f);
	arrTypeColors[ENUM_CLASS(UI_GRAPPLE_TYPE::PULL)]		= _float4(0.631f, 1.000f, 0.914f, 1.0f);
	arrTypeColors[ENUM_CLASS(UI_GRAPPLE_TYPE::END)]			= _float4(1.000f, 0.000f, 1.000f, 1.0f);
}

void CUI_GrapplePoint::Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos)
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

void CUI_GrapplePoint::Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"));

	_float3 vScale =/* (pTargetUI == this)? m_vOriginSca :*/ pTargetTransform->Get_Scaled();

	_float3 vTargetPos = m_vTargetPos;				// ksta : 테스트용, 나중에 수정. 받아온 타겟 좌표로.
	_float fDist = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - XMLoadFloat3(&vTargetPos)));
	_float fScaleMultiple = fPivotDistance / fDist;

	_float3 vFinalScale = _float3{
		vScale.x * fScaleMultiple,
		vScale.y * fScaleMultiple,
		vScale.z * fScaleMultiple
	};

	pTargetTransform->Scale(vFinalScale);
}

void CUI_GrapplePoint::Update_ToggleReqedEvent(_float fTimeDelta)
{
	if (!(m_iReqedDisabled || m_iReqedEnabled))
		return;
	

	const _float fMaxReqTime = 0.5f;	// 애니메이션 적용 시간..
	

	if (m_iReqedEnabled)
	{
		//if (m_isActivate)		// 이미 해당 상태면 무시
		//{	m_iReqedEnabled = false;	return;	}

		if (m_fReqedTimer == 0.f)
		{
			m_pSubAnimUI->Change_Animation(L"Grapple_FadeIn", true);						
			m_pStaticAnimUI->Change_Animation(L"Grapple_Static_FadeIn", true);				
		}
		m_fReqedTimer += fTimeDelta;
		if (m_fReqedTimer >= fMaxReqTime)
		{
			m_isActivate = true;
			m_iReqedEnabled = false;
			return;
		}
	}


	if (m_iReqedDisabled)
	{
		if (!m_isActivate)		// 이미 해당 상태면 무시
		{	m_iReqedEnabled = true;		return;	}

		if (m_fReqedTimer == 0.f)
		{
			m_pSubAnimUI->Change_Animation(L"Grapple_FadeOut", true);
			m_pStaticAnimUI->Change_Animation(L"Grapple_Static_FadeOut", true);
		}
		m_fReqedTimer += fTimeDelta;
		if (m_fReqedTimer >= fMaxReqTime)
		{
			m_isActivate = false;
			m_iReqedDisabled = false;
			return;
		}
	}
}

void CUI_GrapplePoint::Update_TargetColor()
{
	static vector<_float4x4> vecVariantMat = { _float4x4() };
	*reinterpret_cast<_float4*>(&vecVariantMat[0]) = arrTypeColors[ENUM_CLASS(m_eGrappleType)];	// Ready_Presets 에서 정의해 둔 색상으로.

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLE_COLORIZE),
		true
	};

	m_pUIGrapplePoint	->Set_VariantUIDesc(tVariantDesc);
	m_pUIGrappleOutline	->Set_VariantUIDesc(tVariantDesc);
}

void CUI_GrapplePoint::Update_AnimOrder(_float fTimeDelta)
{
	//_float4 vCamPos = *m_pGameInstance->Get_CamPos();	// ksta : 나중에 플레이어 좌표로..
	_vector vPlayerPos = m_pGameSystem->Get_PlayerPosition();
	_float fDistance = XMVectorGetX(XMVector3Length((XMLoadFloat3(&m_vTargetPos) - vPlayerPos)));	// 카메라와 타겟 간 거리

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
	case CUI_GrapplePoint::ENTER:		m_pSubAnimUI->Change_Animation(L"Grapple_FadeIn", true);						
										m_pStaticAnimUI->Change_Animation(L"Grapple_Static_FadeIn", true);				break;
	case CUI_GrapplePoint::EXIT:		m_pSubAnimUI->Change_Animation(L"Grapple_FadeOut", true);						
										m_pStaticAnimUI->Change_Animation(L"Grapple_Static_FadeOut", true);				break;
	case CUI_GrapplePoint::SEMIENTER:	m_pDynamicAnimUI->Change_Animation(L"Grapple_Dynamic_TickLoop", true);			break;
	case CUI_GrapplePoint::SEMIEXIT:	m_pDynamicAnimUI->Change_Animation(L"Grapple_Dynamic_FadeOut", true);			break;
	//case CUI_GrapplePoint::NONE:		
	//default:							m_pSubAnimUI->Change_Animation(L"Grapple_Initialize", true);
	//									m_pDynamicAnimUI->Change_Animation(L"Grapple_Dynamic_Initialize", true);		break;
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

CUI_GrapplePoint* CUI_GrapplePoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_GrapplePoint* pInstance = new CUI_GrapplePoint(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_GrapplePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_GrapplePoint::Clone(void* pArg)
{
	CUI_GrapplePoint* pInstance = new CUI_GrapplePoint(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_GrapplePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_GrapplePoint::Free()
{
#ifdef KSTA_UITEST_GRAPPLE_TOZERO
	//delete m_pTargetPos;
#endif // !KSTA_UITEST_GRAPPLE_TOZERO

	//if (m_isClone)
	//	m_pGameInstance->Remove_RootUI(L"UI_GrapplePoint");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pWorldTransformCom);
}
