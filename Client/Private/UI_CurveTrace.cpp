#include "ClientPch.h"
#include "UI_CurveTrace.h"
#include "VIBuffer_CurveTrace.h"
#include "GameSystem.h"
#include "PlayerStatus.h"


CUI_CurveTrace::CUI_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CUI_CurveTrace::CUI_CurveTrace(const CUI_CurveTrace& Prototype)
	: CGameObject(Prototype)
	, m_pGameSystem (CGameSystem::GetInstance())
{
}

HRESULT CUI_CurveTrace::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUI_CurveTrace::Initialize_Clone(void* pArg)
{
	__super::Initialize_Clone(pArg);

	this->Ready_Components(pArg);
	PreAssign_Presets();


	m_isActivate = false;
	m_isClone = true;

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

	Update_CurveVB();
	Update_CurrentColor();
}


void CUI_CurveTrace::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	m_pGameInstance->Add_Render_Object(RENDERGROUP::BLEND, this);
}

void CUI_CurveTrace::Render()
{
	if (!m_isActivate)
		return;

	_float4x4 IdentityMatrix;
	XMStoreFloat4x4(&IdentityMatrix, XMMatrixIdentity());
	
	_float4x4 thisTransformMatrix = *m_pTransformCom->Get_WorldMatrixPtr();

	_float4x4 playerTransformMatrix = *m_pGameSystem->Get_PlayerMatrixPtr();


	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &playerTransformMatrix)))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Binding_Matrix_Failed");

	if (FAILED(m_pShaderCom->Bind_Value("g_BaseColor", &m_arrSelectedColor[0], sizeof(m_arrSelectedColor[0]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_HeadColor", &m_arrSelectedColor[1], sizeof(m_arrSelectedColor[1]))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_TailColor", &m_arrSelectedColor[2], sizeof(m_arrSelectedColor[2]))))
		CRASH("Binding_Value_Failed");

	m_pShaderCom->Begin(0);
	m_pVIBufferCom->Bind_Resources();
	m_pVIBufferCom->Render();
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

HRESULT CUI_CurveTrace::Ready_Components(void* pArg)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();

	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxCurveTrace"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_CurveTrace"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		return E_FAIL;
}

void CUI_CurveTrace::PreAssign_Presets()
{
	enum CHAR_INDEX { CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };

	m_arrColorPreset[CH_ROVER]		= _float4(0.808f, 0.322f, 0.612f, 1.0f);
	m_arrColorPreset[CH_AUGUSTA]	= _float4(0.969f, 0.451f, 1.000f, 1.0f);
	m_arrColorPreset[CH_GALBRENA]	= _float4(1.000f, 0.416f, 0.416f, 1.0f);

	m_arrAdvColorPreset[CH_ROVER]		= _float4(0.485f, 0.193f, 0.367f, 1.0f);
	m_arrAdvColorPreset[CH_AUGUSTA]		= _float4(0.581f, 0.271f, 0.600f, 1.0f);
	m_arrAdvColorPreset[CH_GALBRENA]	= _float4(0.600f, 0.250f, 0.250f, 1.0f);
}

_float3 CUI_CurveTrace::EvalProjectilePos(const _float3& vPosition, const _float3& vVelocity, const _float3& vAcceleration, _float fTime)		// Start Position, Start Velocity, Gravity
{
	// iSeg + 1개의 기준점 계산
	_float3 out;

	out.x = vPosition.x + vVelocity.x * fTime + 0.5f * vAcceleration.x * pow(fTime, 2);
	out.y = vPosition.y + vVelocity.y * fTime + 0.5f * vAcceleration.y * pow(fTime, 2);
	out.z = vPosition.z + vVelocity.z * fTime + 0.5f * vAcceleration.z * pow(fTime, 2);

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
	//_float3 vCamPos = {
	//	m_pGameInstance->Get_CamPos()->x,
	//	m_pGameInstance->Get_CamPos()->y,
	//	m_pGameInstance->Get_CamPos()->z
	//};
	//
	//_float3 vCamDir;
	//vCamDir.x = vPos.x - vCamPos.x;
	//vCamDir.y = vPos.y - vCamPos.y;
	//vCamDir.z = vPos.z - vCamPos.z;
	//
	//_vector vT = XMVector3Normalize(XMLoadFloat3(&vTan));
	//_vector vC = XMVector3Normalize(XMLoadFloat3(&vCamDir));
	//
	//
	//// 1. 1차 외적 시도
	//_vector vSideVec = XMVector3Cross(vC, vT);
	//_vector vLengthSq = XMVector3LengthSq(vSideVec);
	//
	//// 2. [안전장치] 평행해서 길이가 너무 짧다면(0.001 미만), 월드 Up 벡터를 대신 사용
	//if (XMVectorGetX(vLengthSq) < 0.001f)
	//{
	//	// 진행 방향이 위(0,1,0)인 경우까지 대비해 Right(1,0,0)도 예비로 둠
	//	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	//	if (abs(XMVectorGetY(vT)) > 0.99f) // 진행방향이 수직이면
	//		vWorldUp = XMVectorSet(1.f, 0.f, 0.f, 0.f); // X축 사용
	//
	//	vSideVec = XMVector3Cross(vWorldUp, vT);
	//}
	//
	//_vector vSide = XMVector3Normalize(vSideVec);
	//
	//
	//_float3 out;
	//XMStoreFloat3(&out, vSide);
	//return out;

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

void CUI_CurveTrace::Update_CurveVB()
{
		// 파라미터가 변경되었을 때에만 리본 버텍스를 다시 생성
	//if (m_isModified)
	// 
		// 1) 기준 포인트(포물선) 샘플링
		const _uint iSeg = m_tDesc.iSegmentCount;
		if (iSeg == 0)
			return;

		const _uint iPointCount = iSeg + 1;
		const _uint iVertexCount = iPointCount * 2;

		vector<_float3>			vecPoints(iPointCount);
		vector<VTXUICURVE>		vecVerts(iVertexCount);

		// 포물선 위치 계산 함수 (로컬 람다로 처리)
		

		// [3-1] 포물선 기준점 샘플링
		for (_uint i = 0; i <= iSeg; ++i)
		{
			_float fRatio = (_float)i / (_float)iSeg;       // 0~1
			_float fT = fRatio * m_tDesc.fMaxTime;      // 0~MaxTime

			vecPoints[i] = EvalProjectilePos(
				m_tDesc.vStartPos,
				m_tDesc.vStartVel,
				m_tDesc.vGravity,
				fT);
		}

		// [3-2] 카메라 방향 얻기 (뷰/역행렬 이용)
		// -> GameInstace 통하여 진행

		const _float fHalfWidth = m_tDesc.fWidth * 0.5f;


		_float3 prevSide = {};
		bool    hasPrevSide = false;

		for (_uint i = 0; i < iPointCount; ++i)
		{
			_float3 vPos = vecPoints[i];
			_float3 vTan = CalcTangent(vecPoints.data(), iPointCount, i);
			_float3 vSide = CalcSide(vTan, vPos); // 원래대로 계산

			// [연속성 보정]
			if (hasPrevSide)
			{
				_vector s = XMLoadFloat3(&vSide);
				_vector ps = XMLoadFloat3(&prevSide);
				float dot = XMVectorGetX(XMVector3Dot(s, ps));

				// dot이 0보다 작으면 (90도 넘게 꺾이면) 방향 뒤집기
				if (dot < 0.0f)
				{
					vSide.x *= -1.f;
					vSide.y *= -1.f;
					vSide.z *= -1.f;
				}
			}

			prevSide = vSide;
			hasPrevSide = true;

			// 이하 기존 좌/우 계산
			_float3 vLeft, vRight;

			vLeft.x = vPos.x - vSide.x * fHalfWidth;
			vLeft.y = vPos.y - vSide.y * fHalfWidth;
			vLeft.z = vPos.z - vSide.z * fHalfWidth;

			vRight.x = vPos.x + vSide.x * fHalfWidth;
			vRight.y = vPos.y + vSide.y * fHalfWidth;
			vRight.z = vPos.z + vSide.z * fHalfWidth;

			const _float fCurveU = (_float)i / (_float)iSeg;
			const _uint  idx = i * 2;

			vecVerts[idx + 0].vPosition = vLeft;
			vecVerts[idx + 0].fCurve = fCurveU;
			vecVerts[idx + 0].fWidth = 0;

			vecVerts[idx + 1].vPosition = vRight;
			vecVerts[idx + 1].fCurve = fCurveU;
			vecVerts[idx + 1].fWidth = 1;
		}

		// [3-4] VIBuffer 에 업로드
		//  → CVIBuffer_CurveRibbon::UpdateVertices(const VTX_CURVERIBBON*, _uint iSegCount)
		static_cast<CVIBuffer_CurveTrace*>(m_pVIBufferCom)->UpdateVertices(vecVerts.data(), iSeg);

		m_isModified = true;// false;
	//}
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

CGameObject* CUI_CurveTrace::Clone(void* pArg)
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
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}
