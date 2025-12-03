#include "ClientPch.h"
#include "UI_CurveTrace.h"
#include "VIBuffer_CurveTrace.h"


CUI_CurveTrace::CUI_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CUI_CurveTrace::CUI_CurveTrace(const CUI_CurveTrace& Prototype)
	: CGameObject(Prototype)
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


	m_isActivate = false;
	m_isCloned = true;

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

	// 파라미터가 변경되었을 때에만 리본 버텍스를 다시 생성
	if (m_isModified)
	{
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

		// [3-3] 리본용 좌/우 정점 생성
		for (_uint i = 0; i < iPointCount; ++i)
		{
			_float3 vPos = vecPoints[i];
			_float3 vTan = CalcTangent(vecPoints.data(), iPointCount, i);
			_float3 vSide = CalcSide(vTan, vPos);

			_float3 vLeft;
			_float3 vRight;

			vLeft.x = vPos.x - vSide.x * fHalfWidth;
			vLeft.y = vPos.y - vSide.y * fHalfWidth;
			vLeft.z = vPos.z - vSide.z * fHalfWidth;

			vRight.x = vPos.x + vSide.x * fHalfWidth;
			vRight.y = vPos.y + vSide.y * fHalfWidth;
			vRight.z = vPos.z + vSide.z * fHalfWidth;

			const _float fCurveU = (_float)i / (_float)iSeg; // 진행도 0~1

			const _uint idx = i * 2;

			// left
			vecVerts[idx + 0].vPosition = vLeft;
			vecVerts[idx + 0].fCurve = fCurveU;
			vecVerts[idx + 0].fWidth = 0;

			// right
			vecVerts[idx + 1].vPosition = vRight;
			vecVerts[idx + 1].fCurve = fCurveU;
			vecVerts[idx + 1].fWidth = 1;
		}

		// [3-4] VIBuffer 에 업로드
		//  → CVIBuffer_CurveRibbon::UpdateVertices(const VTX_CURVERIBBON*, _uint iSegCount)
		static_cast<CVIBuffer_CurveTrace*>(m_pVIBufferCom)->UpdateVertices(vecVerts.data(), iSeg);

		m_isModified = false;
	}
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

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Binding_Matrix_Failed");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Binding_Matrix_Failed");

	if (FAILED(m_pShaderCom->Bind_Value("g_BaseColor", &m_tDesc.vBaseColor, sizeof(m_tDesc.vBaseColor))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_HeadColor", &m_tDesc.vHeadColor, sizeof(m_tDesc.vHeadColor))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_TailColor", &m_tDesc.vTailColor, sizeof(m_tDesc.vTailColor))))
		CRASH("Binding_Value_Failed");

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
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		return E_FAIL;
}

void CUI_CurveTrace::Ready_Presets()
{



}

_float3 CUI_CurveTrace::EvalProjectilePos(const _float3& vP0, const _float3& vV0, const _float3& vG, _float fT)
{
	// iSeg + 1개의 기준점 계산
	_float3 out;

	out.x = vP0.x + vV0.x * fT + 0.5f * vG.x * fT * fT;
	out.y = vP0.y + vV0.y * fT + 0.5f * vG.y * fT * fT;
	out.z = vP0.z + vV0.z * fT + 0.5f * vG.z * fT * fT;

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

// side 벡터 (카메라 방향과 탄젠트로부터)
_float3 CUI_CurveTrace::CalcSide(_float3& vTan, _float3& vPos)
{
	_float3 vCamPos = {
		m_pGameInstance->Get_CamPos()->x,
		m_pGameInstance->Get_CamPos()->y,
		m_pGameInstance->Get_CamPos()->z
	};

	_float3 vCamDir;
	vCamDir.x = vPos.x - vCamPos.x;
	vCamDir.y = vPos.y - vCamPos.y;
	vCamDir.z = vPos.z - vCamPos.z;

	_vector vT = XMVector3Normalize(XMLoadFloat3(&vTan));
	_vector vC = XMVector3Normalize(XMLoadFloat3(&vCamDir));

	_vector vSide = XMVector3Normalize(XMVector3Cross(vC, vT));

	_float3 out;
	XMStoreFloat3(&out, vSide);
	return out;
};

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
