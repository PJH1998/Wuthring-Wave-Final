#include "ClientPch.h"
#include "UI_CurveTrace.h"
#include "VIBuffer_CurveTrace.h"
#include "GameSystem.h"
#include "PlayerStatus.h"

// 플레이어 0, 0, 0 기준 테스트 필요시, 아래 매크로 해제 
//#define KSTA_UITEST_BASEDONPLAYER


CUI_CurveTrace::CUI_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_CurveTrace::CUI_CurveTrace(const CUI_CurveTrace& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem (CGameSystem::GetInstance())
{
}

HRESULT CUI_CurveTrace::Initialize_Prototype()
{
	CGameObject::Initialize_Prototype();

	return S_OK;
}

HRESULT CUI_CurveTrace::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	this->Ready_Components(pArg);
	PreAssign_Presets();


	m_isActivate = false;
	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Custom_CurveTrace", this);

	return S_OK;
}

void CUI_CurveTrace::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

}

void CUI_CurveTrace::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	//Update_CurveVB();
	Update_CurrentColor();
	

	m_fTimeElapsed += fTimeDelta;
}


void CUI_CurveTrace::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CheckReq();

	m_pGameInstance->Add_Render_Object(RENDERGROUP::BLEND, this);
}

void CUI_CurveTrace::Render()
{
	if (!m_isActivate)
		return;

	Render_Curve();
	Render_Sphere();
}

void CUI_CurveTrace::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// 월드 행렬 세팅
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	// pArg 로부터 새 파라미터를 받아 궤적 갱신
	if (pArg)
	{
		UI_CURVETRACE_DESC* pDesc = reinterpret_cast<UI_CURVETRACE_DESC*>(pArg);
		m_tDesc = *pDesc;
	}

	m_isActivate = true;
	m_isModified = true;
}

void CUI_CurveTrace::Req_Render_CurveTrace(_float3& vStartPos,
										   _float3& vStartVelocity,
										   _float3& vAcceleration,
										   _float3* pCustomSpherePos,
										   _float fMaxTime,
										   _uint iSegmentCount,
										   _float fRibbonWidth,
										   _bool isUseCustomColor,
										   _float4 vBaseColor,
										   _float4 vHeadColor,
										   _float4 vTailColor)
{
	m_tDesc.vStartPos = vStartPos;
	m_tDesc.vStartVel = vStartVelocity;
	m_tDesc.vAcceleration = vAcceleration;
	m_tDesc.pCustomSpherePos = pCustomSpherePos;
	m_tDesc.fMaxTime = fMaxTime;
	m_tDesc.iSegmentCount = iSegmentCount;
	m_tDesc.fWidth = fRibbonWidth;
	m_tDesc.isUseCustomColor = isUseCustomColor;
	m_tDesc.vBaseColor = vBaseColor;
	m_tDesc.vHeadColor = vHeadColor;
	m_tDesc.vTailColor = vTailColor;
	

	Update_CurveVB();
	m_isReqedCurFrame = true;
}

HRESULT CUI_CurveTrace::Ready_Components(void* pArg)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();

	if (FAILED(CCustom_UI::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxCurveTrace"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pCurveShaderCom), nullptr)))
		return E_FAIL;
	if (FAILED(CCustom_UI::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_CurveTrace"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pCurveVIBufferCom), nullptr)))
		return E_FAIL;
	



	m_pSphereTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (FAILED(m_pSphereTransformCom->Initialize_Clone(pArg)))
		return E_FAIL;
	m_Components.emplace(TEXT("Com_TargetTransform"), m_pSphereTransformCom);
	Safe_AddRef(m_pSphereTransformCom);
	m_pSphereTransformCom->Scale(_float3(5.f, 5.f, 5.f));

	if (FAILED(CCustom_UI::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxCurveTrace_Sphere"),
		TEXT("Com_TargetShader"), reinterpret_cast<CComponent**>(&m_pSphereShaderCom), nullptr)))
		return E_FAIL;
	if (FAILED(CCustom_UI::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Sphere"),
		TEXT("Com_TargetVIBuffer"), reinterpret_cast<CComponent**>(&m_pSphereVIBufferCom), nullptr)))
		return E_FAIL;

	return S_OK;
}

void CUI_CurveTrace::PreAssign_Presets()
{
	enum CHAR_INDEX { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };

	m_arrColorPreset[CH_ROVER]			= _float4(0.808f, 0.322f, 0.612f, 1.0f);
	m_arrColorPreset[CH_AUGUSTA]		= _float4(0.969f, 0.451f, 1.000f, 1.0f);
	m_arrColorPreset[CH_GALBRENA]		= _float4(1.000f, 0.416f, 0.416f, 1.0f);

	m_arrAdvColorPreset[CH_ROVER]		= _float4(0.485f, 0.193f, 0.367f, 1.0f);
	m_arrAdvColorPreset[CH_AUGUSTA]		= _float4(0.581f, 0.271f, 0.600f, 1.0f);
	m_arrAdvColorPreset[CH_GALBRENA]	= _float4(0.600f, 0.250f, 0.250f, 1.0f);
}

_float3 CUI_CurveTrace::EvalProjectilePos(const _float3& vPosition, const _float3& vVelocity, const _float3& vAcceleration, _float fTime)		// Start Position, Start Velocity, Acceleration
{
	// iSeg + 1개의 기준점 계산
	// 나중에 계산식 다르면 이거 수정하면 됨
	_float3 out;

	out.x = vPosition.x + vVelocity.x * fTime + 0.5f * vAcceleration.x * powf(fTime, 2);
	out.y = vPosition.y + vVelocity.y * fTime + 0.5f * vAcceleration.y * powf(fTime, 2);
	out.z = vPosition.z + vVelocity.z * fTime + 0.5f * vAcceleration.z * powf(fTime, 2);

	return out;
};

_float3 CUI_CurveTrace::CalcTangent(_float3* pPts, _uint count, _uint idx)
{
	_uint prev = (idx == 0) ? 0 : idx - 1;
	_uint next = (idx + 1 >= count) ? count - 1 : idx + 1;

	_float3 vDir;
	vDir.x = pPts[next].x - pPts[prev].x;
	vDir.y = pPts[next].y - pPts[prev].y;
	vDir.z = pPts[next].z - pPts[prev].z;

	_vector v = XMVector3Normalize(XMLoadFloat3(&vDir));
	_float3 out;
	XMStoreFloat3(&out, v);
	return out;
};


_float3 CUI_CurveTrace::CalcSide(_float3& vTan, _float3& vPos)
{
	_vector vTangent = XMVector3Normalize(XMLoadFloat3(&vTan));	// 정규화
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// [예외 처리] 만약 선이 수직으로 솟구쳐서(위/아래) Up과 평행하다면?
	// 그때만 X축을 임시 기준으로 잡음 (거의 일어날 일 없지만 안전장치)
	if (abs(XMVectorGetY(vTangent)) > 0.99f)
		vWorldUp = XMVectorSet(1.f, 0.f, 0.f, 0.f);

	// 외적 (Tangent x Up -> Right Side)
	_vector vSide = XMVector3Cross(vWorldUp, vTangent);
	vSide = XMVector3Normalize(vSide);

	_float3 out;
	XMStoreFloat3(&out, vSide);
	return out;
}

void CUI_CurveTrace::Update_CheckReq()
{
	if (m_isReqedPreFrame && !m_isReqedCurFrame)
		m_isActivate = false;

	m_isReqedPreFrame = m_isReqedCurFrame;
	m_isReqedCurFrame = false;
}

void CUI_CurveTrace::Update_CurveVB()					// 지금 시작점의 위치가, 플레이어 위치를 고려않고, 플레이어 위치를 중점삼아 쏘는 중.
{
	// [1] 파라미터 준비
	const _uint iSegmentIndex = m_tDesc.iSegmentCount;
	if (iSegmentIndex == 0) return;

	// ksta : 나중에 타겟 변경 시, 좌표 삽입. 바라보는 방향도 적용된 행렬이 필요함.
	//		 이는 위치는 해당 오브젝트 위치 기준으로, 방향 등을 비롯한 정보는 카메라 방향을 바라보던가 하는 식으로 따로 하면 될 듯

	//_float4 vRawTargetPos = {};	XMStoreFloat4(&vRawTargetPos, m_pGameSystem->Get_PlayerPosition());			.
#ifdef KSTA_UITEST_BASEDONPLAYER
	_matrix matTargetTransform = XMLoadFloat4x4(m_pGameSystem->Get_PlayerMatrixPtr());
#endif // KSTA_UITEST_BASEDONPLAYER

	//_float3	vTargetPos = *reinterpret_cast<_float3*>(&vRawTargetPos);

	// 유효한 점들을 담을 컨테이너 (시작점 포함)
	vector<_float3> vecValidPoints;
	vecValidPoints.reserve(iSegmentIndex + 1);
	vecValidPoints.push_back(m_tDesc.vStartPos);

	// 타겟 표시 초기화
	m_isShowTarget = false;

	// [2] 시뮬레이션 루프. 각 선분 단위로, 가까운 선 부터,레이 검사하며 확인
	for (_uint i = 0; i < iSegmentIndex; ++i)
	{
		_float3 vCurrentPos = vecValidPoints.back();		// 현재 점

		_float fNextRatio = (_float)(i + 1) / (_float)iSegmentIndex;	// 다음 점 계산
		_float fNextTime = fNextRatio * m_tDesc.fMaxTime;

		_float3 vNextPos = EvalProjectilePos(m_tDesc.vStartPos, m_tDesc.vStartVel, m_tDesc.vAcceleration, fNextTime);	// 다음 점

		// 레이캐스트 진행
		_float4 vHitPos4;
		// 레이는 월드 좌표를 기준으로 비교하여야 함. 플레이어 위치 고려안하면 그냥 플레이어 위치를 중점삼아 이상하게 계산함.
		// 또한 계산될 위치는 플레이어가 바라보는 방향으로 적용되어야 함.

#ifdef KSTA_UITEST_BASEDONPLAYER
		_vector vCalcedCurrentWorldPos	= XMVector3TransformCoord(XMLoadFloat3(&vCurrentPos), matTargetTransform);
		_vector vCalcedNextWorldPos		= XMVector3TransformCoord(XMLoadFloat3(&vNextPos), matTargetTransform);
#endif // KSTA_UITEST_BASEDONPLAYER
#ifndef KSTA_UITEST_BASEDONPLAYER
		_vector vCalcedCurrentWorldPos	= XMVectorSetW(XMLoadFloat3(&vCurrentPos), 1.f);
		_vector vCalcedNextWorldPos		= XMVectorSetW(XMLoadFloat3(&vNextPos), 1.f);
#endif // !KSTA_UITEST_BASEDONPLAYER

		_float3 vCurrentWorldPos = {};	XMStoreFloat3(&vCurrentWorldPos, vCalcedCurrentWorldPos);

		_float3 vNextWorldPos = {};		XMStoreFloat3(&vNextWorldPos, vCalcedNextWorldPos);

		_bool isRayDetected = m_pGameInstance->Ray_Cast(XMLoadFloat3(&vCurrentWorldPos), XMLoadFloat3(&vNextWorldPos), &vHitPos4);
		vHitPos4.w = 1.f;



		// ksta : 임의 원 설정 좌표 있으면 그것 사용.
		if (m_tDesc.pCustomSpherePos)
		{
			m_pSphereTransformCom->Set_State(STATE::POSITION, XMLoadFloat3(m_tDesc.pCustomSpherePos));
			m_isShowTarget = true;
		}
		else
		{
			if (isRayDetected)	// 충돌O
			{
				_float3 vHitPos = _float3(vHitPos4.x, vHitPos4.y, vHitPos4.z);				// 충돌 지점. world
				// 이거는 다시 플레이어 기준 로컬 좌표로 변환하여 넣어야 함.
	#ifdef KSTA_UITEST_BASEDONPLAYER
				_vector vCalcedLocalHitPos = XMVector3TransformCoord(XMLoadFloat3(&vHitPos), XMMatrixInverse(nullptr, matTargetTransform));
	#endif // KSTA_UITEST_BASEDONPLAYER
	#ifndef KSTA_UITEST_BASEDONPLAYER
				_vector vCalcedLocalHitPos = XMVectorSetW(XMLoadFloat3(&vHitPos), 1.f);
	#endif // !KSTA_UITEST_BASEDONPLAYER
				_float3 vLocalHitPos = {};	XMStoreFloat3(&vLocalHitPos, vCalcedLocalHitPos);

				vecValidPoints.push_back(vLocalHitPos);										// 궤적 리스트에 충돌 지점까지의 점 추가. (선 종료)		
				m_pSphereTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vHitPos4));	// 구 위치 지정
				m_isShowTarget = true;														// 구 활성화
				break;
			}
			else				// 충돌X
			{
				vecValidPoints.push_back(vNextPos);											// 궤적 리스트에 점 추가.
				if (i == iSegmentIndex - 1)													// 만약 끝까지 날아갔다면, 마지막 지점에 타겟 표시
				{
					m_pSphereTransformCom->Set_State(STATE::POSITION, XMLoadFloat3(&vNextPos));
					m_isShowTarget = true;
				}
			}
		}
	}


	// [3] 리본 메쉬(VIBuffer) 갱신
	const _uint iValidCount = (_uint)vecValidPoints.size();
	if (iValidCount < 2) return; // 점이 최소 2개는 있어야 선을 그리므로..

	// 정점 배열 (점 개수 * 2) -> 좌측점, 우측점
	vector<VTXUICURVE> vecVerts(iValidCount * 2);
	const _float fHalfWidth = m_tDesc.fWidth * 0.5f;

	_float3 prevSide = {};
	_bool   hasPrevSide = false;

	for (_uint i = 0; i < iValidCount; ++i)
	{
		_float3 vPos = vecValidPoints[i];

		// 탄젠트(진행방향) 및 사이드(우측방향) 계산
		_float3 vTan = CalcTangent(vecValidPoints.data(), iValidCount, i);
		_float3 vSide = CalcSide(vTan, vPos);

		// 연속성 보정 (갑자기 방향 튐 방지)
		if (hasPrevSide)
		{
			_vector s = XMLoadFloat3(&vSide);
			_vector ps = XMLoadFloat3(&prevSide);
			if (XMVectorGetX(XMVector3Dot(s, ps)) < 0.0f) // 90도 이상 꺾이면 뒤집기
			{
				vSide.x *= -1.f; vSide.y *= -1.f; vSide.z *= -1.f;
			}
		}
		prevSide = vSide;
		hasPrevSide = true;

		// 좌/우 정점 생성
		_float3 vLeft, vRight;
		vLeft.x = vPos.x - vSide.x * fHalfWidth;
		vLeft.y = vPos.y - vSide.y * fHalfWidth;
		vLeft.z = vPos.z - vSide.z * fHalfWidth;

		vRight.x = vPos.x + vSide.x * fHalfWidth;
		vRight.y = vPos.y + vSide.y * fHalfWidth;
		vRight.z = vPos.z + vSide.z * fHalfWidth;

		// UV 좌표 계산 (0~1)
		const _float fCurveU = (_float)i / (_float)(m_tDesc.iSegmentCount); // 전체 대비, 현재 인덱스의 진행 비율
		const _uint  idx = i * 2;

		vecVerts[idx + 0].vPosition = vLeft;
		vecVerts[idx + 0].fCurve = fCurveU;
		vecVerts[idx + 0].fWidth = 0; // Shader에서 Edge 처리용

		vecVerts[idx + 1].vPosition = vRight;
		vecVerts[idx + 1].fCurve = fCurveU;
		vecVerts[idx + 1].fWidth = 1;
	}

	// 버퍼에 데이터 삽입. UpdateVertices 함수는 선분(segment) 개수를 인자로 받으므로, 점의 개수인 iValidCount에서 1을 뺍니다.
	static_cast<CVIBuffer_CurveTrace*>(m_pCurveVIBufferCom)->UpdateVertices(vecVerts.data(), iValidCount - 1);

	m_isModified = true;
}

void CUI_CurveTrace::Update_CurrentColor()
{
	if (m_tDesc.isUseCustomColor)
	{
		m_arrSelectedColor[0] =	m_tDesc.vBaseColor;
		m_arrSelectedColor[1] =	m_tDesc.vHeadColor;
		m_arrSelectedColor[2] = m_tDesc.vTailColor;
	}
	else
	{
		_uint iSelectedChar = m_pGameSystem->Get_PlayerStatus()->Get_CurrentCharIndex();

		m_arrSelectedColor[0] = _float4(1.f, 1.f, 1.f, 1.f);
		m_arrSelectedColor[1] = m_arrColorPreset[iSelectedChar];
		m_arrSelectedColor[2] = m_arrAdvColorPreset[iSelectedChar];
	}
}
void CUI_CurveTrace::Render_Curve()
{
	_float4x4 IdentityMatrix;
	XMStoreFloat4x4(&IdentityMatrix, XMMatrixIdentity());

	_float4x4 thisTransformMatrix = *m_pTransformCom->Get_WorldMatrixPtr();

	// ksta : 임시로 플레이어 좌표 가져옴. 나중에 시작 좌표는 픽업한 오브젝트,
	//		 방향은 카메라가 바라보는 방향으로 픽스 필요
#ifdef KSTA_UITEST_BASEDONPLAYER
	_float4x4 playerTransformMatrix = *m_pGameSystem->Get_PlayerMatrixPtr();	// 나중에 
	if (FAILED(m_pCurveShaderCom->Bind_Matrix("g_WorldMatrix", &playerTransformMatrix)))
		CRASH("Binding_Matrix_Failed");
#endif // KSTA_UITEST_BASEDONPLAYER

#ifndef KSTA_UITEST_BASEDONPLAYER
	_float4x4 matCurveStart = {};
	XMStoreFloat4x4(&matCurveStart, XMMatrixTranslationFromVector(XMLoadFloat3(&m_tDesc.vStartPos)));
	if (FAILED(m_pCurveShaderCom->Bind_Matrix("g_WorldMatrix", &matCurveStart)))
		CRASH("Binding_Matrix_Failed");
#endif // !KSTA_UITEST_BASEDONPLAYER


	if (FAILED(m_pCurveShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pCurveShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Binding_Matrix_Failed");

	if (FAILED(m_pCurveShaderCom->Bind_Value("g_BaseColor", &m_arrSelectedColor[0], sizeof(m_arrSelectedColor[0]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pCurveShaderCom->Bind_Value("g_HeadColor", &m_arrSelectedColor[1], sizeof(m_arrSelectedColor[1]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pCurveShaderCom->Bind_Value("g_TailColor", &m_arrSelectedColor[2], sizeof(m_arrSelectedColor[2]))))
		CRASH("Binding_Value_Failed");

	if (FAILED(m_pCurveShaderCom->Bind_Value("g_TimeElapsed", &m_fTimeElapsed, sizeof(m_fTimeElapsed))))
		CRASH("Binding_Value_Failed");


	m_pCurveShaderCom->Begin(0);
	m_pCurveVIBufferCom->Bind_Resources();
	m_pCurveVIBufferCom->Render();
}

void CUI_CurveTrace::Render_Sphere()
{
	if (!m_isShowTarget)
		return;

	_float4x4 TargetWorld;
	XMStoreFloat4x4(&TargetWorld, m_pSphereTransformCom->Get_WorldMatrix());
	if (FAILED(m_pSphereShaderCom->Bind_Matrix("g_WorldMatrix", &TargetWorld)))
		CRASH("Binding_Matrix_Failed");
	_float4 vCamPos = *m_pGameInstance->Get_CamPos();
	if (FAILED(m_pSphereShaderCom->Bind_Value("g_CamPosition", &vCamPos, sizeof(vCamPos))))
		CRASH("Binding_Value_Failed");

	if (FAILED(m_pSphereShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pSphereShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Binding_Matrix_Failed");

	if (FAILED(m_pSphereShaderCom->Bind_Value("g_BaseColor", &m_arrSelectedColor[0], sizeof(m_arrSelectedColor[0]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pSphereShaderCom->Bind_Value("g_HeadColor", &m_arrSelectedColor[1], sizeof(m_arrSelectedColor[1]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pSphereShaderCom->Bind_Value("g_TailColor", &m_arrSelectedColor[2], sizeof(m_arrSelectedColor[2]))))
		CRASH("Binding_Value_Failed");

	if (FAILED(m_pCurveShaderCom->Bind_Value("g_TimeElapsed", &m_fTimeElapsed, sizeof(m_fTimeElapsed))))
		CRASH("Binding_Value_Failed");


	m_pSphereShaderCom->Begin(0);			// Sphere 용 패스 제작
	m_pSphereVIBufferCom->Bind_Resources();
	m_pSphereVIBufferCom->Render();
}
;
CUI_CurveTrace* CUI_CurveTrace::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_CurveTrace* pInstance = new CUI_CurveTrace(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_CurveTrace");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CCustom_UI* CUI_CurveTrace::Clone(void* pArg)
{
	CUI_CurveTrace* pInstance = new CUI_CurveTrace(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_CurveTrace");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_CurveTrace::Free()
{
	__super::Free();

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Custom_CurveTrace");

	Safe_Release(m_pCurveShaderCom);
	Safe_Release(m_pCurveVIBufferCom);

	Safe_Release(m_pSphereTransformCom);
	Safe_Release(m_pSphereShaderCom);
	Safe_Release(m_pSphereVIBufferCom);
}
