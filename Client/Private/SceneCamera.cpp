#include "ClientPch.h"
#include "SceneCamera.h"

CSceneCamera::CSceneCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice , pContext }
{
}

CSceneCamera::CSceneCamera(const CSceneCamera& Prototype)
	: CCamera { Prototype }
{
}

HRESULT CSceneCamera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSceneCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");

    return S_OK;
}

void CSceneCamera::Priority_Update(_float fTimeDelta)
{
}

void CSceneCamera::Update(_float fTimeDelta)
{
	m_fTrackPosition += fTimeDelta * m_fVelocity;
	if (m_fTrackPosition >= m_fEndFrame || m_iFrameIndex == m_Frames.size() - 1)
	{
		m_isActivate = false;
		return;
	}

	m_fRatio = (m_fTrackPosition - m_Frames[m_iFrameIndex].fStartFrame) / (m_Frames[m_iFrameIndex + 1].fStartFrame - m_Frames[m_iFrameIndex].fStartFrame);
	// 0. Default SetUp
	Default_SetUp();
	// 1. Quat SLerp
	Lerp_Quat();
	// 2. Spline (Catmull-Rom)
	Spline();

	// Frame Check
	if (m_fTrackPosition >= m_Frames[m_iFrameIndex + 1].fStartFrame)
		++m_iFrameIndex;
}

void CSceneCamera::Late_Update(_float fTimeDelta)
{
}

void CSceneCamera::Render()
{
}

void CSceneCamera::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	SQ_CAMERA_DATA* pData = static_cast<SQ_CAMERA_DATA*>(pArg);
	if (nullptr == pData)
		return;
	m_isActivate = true;

	m_iFrameIndex = 0;
	m_Frames = pData->Frames;
	m_fStartFrame = pData->fStartFrame;
	m_fEndFrame = pData->fEndFrame;
	m_fTrackPosition = m_fStartFrame;
}

void CSceneCamera::Default_SetUp()
{
	_float fLerpSpeedRate = m_Frames[m_iFrameIndex].fSpeedRate * (1.f - m_fRatio) + m_Frames[m_iFrameIndex + 1].fSpeedRate * m_fRatio;
	m_fVelocity = m_fSpeed * fLerpSpeedRate;
}

void CSceneCamera::Lerp_Quat()
{
	_vector vPreQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex].vQuaternion);
	_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vQuaternion);
	_vector vLerpQuat = XMQuaternionSlerp(vPreQuat, vDestQuat, m_fRatio);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
}

void CSceneCamera::Spline()
{
	_int iEnd = m_Frames.size() - 1;
	_int iIndex0 = clamp(m_iFrameIndex - 1, 0, iEnd);
	_int iIndex1 = clamp(m_iFrameIndex, 0, iEnd);
	_int iIndex2 = clamp(m_iFrameIndex + 1, 0, iEnd);
	_int iIndex3 = clamp(m_iFrameIndex + 2, 0, iEnd);

	_vector vP0 = XMLoadFloat3(&m_Frames[iIndex0].vPosition);
	_vector vP1 = XMLoadFloat3(&m_Frames[iIndex1].vPosition);
	_vector vP2 = XMLoadFloat3(&m_Frames[iIndex2].vPosition);
	_vector vP3 = XMLoadFloat3(&m_Frames[iIndex3].vPosition);

	_float fSplineRatio = (m_fTrackPosition - m_Frames[iIndex1].fStartFrame) / (m_Frames[iIndex2].fStartFrame - m_Frames[iIndex1].fStartFrame);

	_float fRatio2 = fSplineRatio * fSplineRatio;
	_float fRatio3 = fRatio2 * fSplineRatio;

	_vector vSpline = 0.5f * (
		(2.f * vP1) +
		(-1.f * vP0 + vP2) * fSplineRatio +
		(2.f * vP0 - 5.f * vP1 + 4.f * vP2 - vP3) * fRatio2 +
		(-1.f * vP0 + 3.f * vP1 - 3.f * vP2 + vP3) * fRatio3
		);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vSpline, 1.f));
}

CSceneCamera* CSceneCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSceneCamera* pInstance = new CSceneCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("Logo Camera");

    return pInstance;
}

CGameObject* CSceneCamera::Clone(void* pArg)
{
	CSceneCamera* pClone = new CSceneCamera(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("Logo Camera");

	return pClone;
}

void CSceneCamera::Free()
{
	__super::Free();
}

