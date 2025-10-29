#include "ClientPch.h"
#include "SpringCamera.h"

CSpringCamera::CSpringCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CSpringCamera::CSpringCamera(const CSpringCamera& Prototype)
	: CCamera { Prototype }
{
}

void CSpringCamera::Update_Target(const _fvector & TargetPos, _float fOffsetY)
{
	m_fOffsetY = fOffsetY;
	// Lerp
	//_vector vPos = XMVectorLerp(XMLoadFloat4(&m_vTargetPosition), TargetPos, 1.f - exp(-1.f * 0.0016f * 30.f));
	XMStoreFloat4(&m_vTargetPosition, TargetPos);
}

_vector CSpringCamera::Get_LookVector_NoPitch()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	vLook = XMVectorSetY(vLook, 0.f);
	return XMVector3Normalize(vLook);
}

_vector CSpringCamera::Get_RightVector_NoPitch()
{
	_vector vRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
	vRight = XMVectorSetY(vRight, 0.f);  // Pitch 제거
	return XMVector3Normalize(vRight);
}


HRESULT CSpringCamera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpringCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");

	m_fDistance = 10.f;
	m_fFixedDistance = 10.f;
	m_fLerpSpeed = 0.5f;
	m_fMinDistance = 5.f;
	m_fMaxDistance = 15.f;

	m_fStiffness = 0.3f;

	m_fLockOnOffsetY = 3.5f;

    return S_OK;
}

void CSpringCamera::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera::Update(_float fTimeDelta)
{
	// Look Position Init
	m_vLookPosition = m_vTargetPosition;
	m_vLookPosition.y += m_fOffsetY;

	// Spring
	if (CAMERA_STATE::SPRING == m_eCameraState)
		Spring(fTimeDelta);
	else
		Lerp_Distance(fTimeDelta);

	// Lock-On
	if (CAMERA_STATE::LOCKON == m_eCameraState)
		Dual_Targeting(fTimeDelta);

	Mouse_Scroll(fTimeDelta);
	// 0. Cam Rotate
	if (CAMERA_STATE::TARGET == m_eCameraState)
		__super::Mouse_Move_Up();

	// 1. 거리 제한으로 인한 간격 보정
	Compute_CamPos();
	// 2. Ray Cast 이용하여 지형, 오브젝트와 충돌
	if(CAMERA_STATE::TARGET == m_eCameraState)
		Check_Ray();
}

void CSpringCamera::Late_Update(_float fTimeDelta)
{
	//if (CAMERA_STATE::LOCKON == m_eCameraState && 0 == m_TargetTransforms.size())
	//	m_eCameraState = CAMERA_STATE::TARGET;
}

void CSpringCamera::Render()
{
}

void CSpringCamera::Lerp_Distance(_float fTimeDelta)
{
	if (0.1f < fabsf(m_fFixedDistance - m_fDistance))
		m_fDistance += (m_fFixedDistance - m_fDistance) * fTimeDelta * m_fLerpSpeed;
}

void CSpringCamera::Mouse_Scroll(_float fTimeDelta)
{
	m_fFixedDistance -= m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::WHEEL) * fTimeDelta * 20.f;

	m_fFixedDistance = max(m_fMinDistance, min(m_fMaxDistance, m_fFixedDistance));
}

void CSpringCamera::Spring(_float fTimeDelta)
{
	_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vLookPosition) - m_pTransformCom->Get_State(STATE::POSITION)));

	if (fDistance < m_fDestination)
	{
		m_eCameraState = CAMERA_STATE::TARGET;
		m_fDistance = m_fDestination;
		return;
	}
	m_fDistance += (m_fDestination - m_fFixedDistance) * m_fStiffness / m_fSpringDuration * fTimeDelta;
}

void CSpringCamera::Compute_CamPos()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// Offset Y Adjust
	_vector vTargetPos;
	vTargetPos = XMLoadFloat4(&m_vLookPosition);

	m_fLockOnDistanceOffset = 0.f;
	if(CAMERA_STATE::LOCKON ==  m_eCameraState)
		Adjust_LockOn_Distance();
	cout << "LDO : " << m_fLockOnDistanceOffset << endl;
	_vector vCamPos = XMVectorSetW(vTargetPos - vLook * (m_fDistance + m_fLockOnDistanceOffset), 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vCamPos);
}

void CSpringCamera::Check_Ray()
{
	_vector vCamPos = m_pTransformCom->Get_State(STATE::POSITION);

	_vector vStartPos = XMLoadFloat4(&m_vLookPosition);
	_float4 vOut;
	if (true == m_pGameInstance->Ray_Cast(vStartPos, vCamPos, &vOut))
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vOut));
}


void CSpringCamera::Lerp_Move(_float fTimeDelta)
{
	// 현재 회전량
	_vector vPreQuat = m_pTransformCom->Get_Quaternion();

	// 현재 Dir
	_vector vDestinationDir = XMVector3Normalize(XMLoadFloat4(&m_vLookPosition) - XMLoadFloat4(&m_vTargetPosition));

	_vector vCamPos = XMLoadFloat4(&m_vLookPosition) - vDestinationDir * m_fDistance;
	vCamPos.m128_f32[1] += m_fLockOnOffsetY;
	_vector vLookDir = XMLoadFloat4(&m_vLookPosition) - vCamPos;
	// 목표 Dir
	m_pTransformCom->LookDir(vLookDir);
	_vector vCurrentQuat = m_pTransformCom->Get_Quaternion();

	_float fDot = XMVectorGetX(XMQuaternionDot(vPreQuat, vCurrentQuat));
	// 회전 보간
	_float fLerp = {};
	if (fDot < cos(XMConvertToRadians(25.f)))
		fLerp = 1.f - exp(-1.f * fTimeDelta * 2.5f);
	else
		fLerp = 1.f - exp(-1.f * fTimeDelta * 1.25f * min(1.f, (cos(XMConvertToRadians(25.f) - fDot))));
	m_pTransformCom->Rotation_Quaternion(XMQuaternionSlerp(vPreQuat, vCurrentQuat, fLerp));
}

void CSpringCamera::Dual_Targeting(_float fTimeDelta)
{
	if (nullptr == m_pTargetTransform)
		return;

	_float fRatio = 0.1f;
	XMStoreFloat4(&m_vLookPosition, XMLoadFloat4(&m_vTargetPosition) * (1.f - fRatio) + m_pTargetTransform->Get_State(STATE::POSITION) * fRatio);
	// Dynamic Distance
	Dynamic_Distance();
	// Moving Lerp
	Lerp_Move(fTimeDelta);
}

void CSpringCamera::Dynamic_Distance()
{
	if (nullptr == m_pTargetTransform)
		return;
	//_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vLookPosition) - XMLoadFloat4(&m_vTargetPosition)));
	_float fDistance = XMVectorGetX(XMVector3Length(m_pTargetTransform->Get_State(STATE::POSITION) - XMLoadFloat4(&m_vTargetPosition)));

	m_fFixedDistance = max(m_fMinDistance, sqrt(fDistance * fDistance + m_fLockOnOffsetY * m_fLockOnOffsetY));
}

void CSpringCamera::Adjust_LockOn_Distance()
{
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	_vector vLookRemoveY = vLook;
	vLookRemoveY.m128_f32[1] = 0.f;

	_float fRadian = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vLook), XMVector3Normalize(vLookRemoveY)));

	_float fLength = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vTargetPosition) - XMLoadFloat4(&m_vLookPosition)));
	m_fLockOnDistanceOffset = fLength / fRadian;
}

CSpringCamera* CSpringCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpringCamera* pInstance = new CSpringCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : SpringCamera");
		Safe_Release(pInstance);
	}

    return pInstance;
}

CGameObject* CSpringCamera::Clone(void* pArg)
{
	CSpringCamera* pClone = new CSpringCamera(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : SpringCamera (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpringCamera::Free()
{
	__super::Free();

	//m_TargetTransforms.clear();
	m_pTargetTransform = nullptr;
}
