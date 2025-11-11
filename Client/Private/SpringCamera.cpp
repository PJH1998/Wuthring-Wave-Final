#include "ClientPch.h"
#include "SpringCamera.h"

#include "Event_Camera.h"

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
	XMStoreFloat4(&m_vTargetPosition, TargetPos);
}

_vector CSpringCamera::Get_LookVector()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	return vLook;
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
	vRight = XMVectorSetY(vRight, 0.f);  // Pitch ����
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

	m_fDistance = 3.f;
	m_fFixedDistance = 3.f;
	m_fLerpSpeed = 1.5f;
	m_fMinDistance = 1.f;
	m_fMaxDistance = 6.f;

	m_fLockOnMinDistance = 4.f;

	m_fStiffness = 0.3f;

	m_fLockOnOffsetY = 1.5f;
	Ready_Event();
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

	// Action
	if (CAMERA_STATE::ACTION == m_eCameraState)
	{
		if (true == m_isRecovery)
			Recovery(fTimeDelta);
		else
			Action(fTimeDelta);
	}
	else
	{
		Mouse_Scroll(fTimeDelta);
		// 0. Cam Rotate
		if (CAMERA_STATE::TARGET == m_eCameraState)
			__super::Mouse_Move_Up();
	}

	// 1. Camera Position 계산
	Compute_CamPos();
	// 2. Ray Cast => 벽 충돌
	if(CAMERA_STATE::TARGET == m_eCameraState)
		Check_Ray();

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD7) == KEYSTATE::DOWN)
	{
		CAMERA_SHAKE Shake = {};
		Shake.fDuration = 0.25f;
		Shake.fFrequency = 12.f;
		Shake.fAmplitude = 1.f;
		Shake.fFovKick = XMConvertToRadians(0.7f);
		Shake.vRotation = _float3(0.1f, 0.1f, 0.1f);
		m_pGameInstance->OnShake(Shake);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD8) == KEYSTATE::DOWN)
	{
		CAMERA_SHAKE Shake = {};
		Shake.fDuration = 0.12f;
		Shake.fFrequency = 12.f;
		Shake.fAmplitude = 1.f;
		Shake.fFovKick  = XMConvertToRadians(0.f);
		Shake.vRotation = _float3(0.1f, 0.1f, 0.f);  // Pitch(x: 위아래), Yaw(y: 좌우), Roll(z: 0)
		m_pGameInstance->OnShake(Shake);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9) == KEYSTATE::DOWN)
	{
		CAMERA_SHAKE Shake = {};
		Shake.fDuration = 0.12f;
		Shake.fFrequency = 12.f;
		Shake.fAmplitude = 1.f;
		Shake.fFovKick = XMConvertToRadians(0.f);
		Shake.vRotation = _float3(0.1f, 0.f, 0.f);  // Pitch(x: 위아래), Yaw(y: 좌우), Roll(z: 0)
		m_pGameInstance->OnShake(Shake);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN)
		m_pGameInstance->Set_CurrentCamera_Far(300.f);

	Shaking(fTimeDelta);
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
	//cout << "LDO : " << m_fLockOnDistanceOffset << endl;
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
	// ���� ȸ����
	_vector vPreQuat = m_pTransformCom->Get_Quaternion();

	// ���� Dir
	_vector vDestinationDir = XMVector3Normalize(XMLoadFloat4(&m_vLookPosition) - XMLoadFloat4(&m_vTargetPosition));

	_vector vCamPos = XMLoadFloat4(&m_vLookPosition) - vDestinationDir * m_fDistance;
	vCamPos.m128_f32[1] += m_fLockOnOffsetY;
	_vector vLookDir = XMLoadFloat4(&m_vLookPosition) - vCamPos;
	// ��ǥ Dir
	m_pTransformCom->LookDir(vLookDir);
	_vector vCurrentQuat = m_pTransformCom->Get_Quaternion();

	_float fDot = XMVectorGetX(XMQuaternionDot(vPreQuat, vCurrentQuat));
	// ȸ�� ����
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

	_float fRatio = 0.4f;
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

	m_fFixedDistance = max(m_fLockOnMinDistance, sqrt(fDistance * fDistance + m_fLockOnOffsetY * m_fLockOnOffsetY));
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

void CSpringCamera::Action(_float fTimeDelta)
{
	// Action End
	if (m_iFrameIndex >= static_cast<_int>(m_Frames.size() - 1))
	{
		if (false == m_isMaintain)
			SetUp_Recovery();
		return;
	}

	_float fStartFrame{}, fEndFrame{};
	fStartFrame = -1 == m_iFrameIndex ? m_fFirstFrame : m_Frames[m_iFrameIndex].fStartFrame;
	fEndFrame = m_Frames[m_iFrameIndex + 1].fStartFrame;

	_float fRatio = (m_fTrackPosition - fStartFrame) / (fEndFrame - fStartFrame);
	m_fTrackPosition += fTimeDelta * m_fTrackPerSec;

	// Rotation
	_vector vPreQuaternion = {};
	_vector vPreTranslation = {};
	if (-1 == m_iFrameIndex)
	{
		vPreQuaternion = XMLoadFloat4(&m_vPreQuaternion);
		vPreTranslation = XMLoadFloat3(&m_vPreTranslation);
		m_fPreFovy = m_fFovy;
	}
	else
	{
		vPreQuaternion = XMLoadFloat4(&m_Frames[m_iFrameIndex].vRotation);
		vPreQuaternion = XMQuaternionMultiply(vPreQuaternion, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		vPreTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex].vTranslation);
		vPreTranslation = XMVector3TransformNormal(vPreTranslation, XMMatrixRotationQuaternion(vPreQuaternion));
		// Fov
		m_fPreFovy = XMConvertToRadians(m_Frames[m_iFrameIndex].fFovy);
	}

	m_isLerp = m_Frames[m_iFrameIndex + 1].isLerp;

	if (true == m_isLerp)
	{
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		vDestQuat = XMQuaternionMultiply(vDestQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		_vector vLerpQuat = XMQuaternionSlerp(vPreQuaternion, vDestQuat, fRatio);
		m_pTransformCom->Rotation_Quaternion(vLerpQuat);

		// Translation Offset
		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		//vDestTranslation = XMVector3TransformCoord(vDestTranslation, XMLoadFloat4x4(&m_OwnerMatrix));
		_vector vLerpTranslation = XMVectorLerp(vPreTranslation, vDestTranslation, fRatio);
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));
		XMStoreFloat3(&m_vEndTranslation, vDestTranslation);

		// Distance
		m_fFixedDistance = m_Frames[m_iFrameIndex + 1].fDistance;

		// Fov Lerp
		m_fFovy = m_fPreFovy + (XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy) - m_fPreFovy) * fRatio;
	}
	// None Lerp (Teleport)
	else
	{
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		vDestQuat = XMQuaternionMultiply(vDestQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		m_pTransformCom->Rotation_Quaternion(vDestQuat);

		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		//vDestTranslation = XMVector4Transform(vDestTranslation, XMLoadFloat4x4(&m_OwnerMatrix));
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vDestTranslation, 1.f));
		XMStoreFloat3(&m_vEndTranslation, vDestTranslation);

		m_fFixedDistance = m_fFixedDistance = m_Frames[m_iFrameIndex + 1].fDistance;
		m_fDistance = m_fFixedDistance;

		m_fFovy = XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy);
	}

	if (m_Frames[m_iFrameIndex + 1].fStartFrame < m_fTrackPosition)
		++m_iFrameIndex;
}

void CSpringCamera::Recovery(_float fTimeDelta)
{
	m_fTrackPosition += fTimeDelta;

	if (m_fTrackPosition >= 1.5f)
	{
		m_isRecovery = false;
		m_eCameraState = CAMERA_STATE::TARGET;
		m_pTransformCom->Rotation_Quaternion(XMLoadFloat4(&m_vPreQuaternion));
		return;
	}

	_vector vLerpQuat = XMQuaternionSlerp(XMLoadFloat4(&m_vEndQuaternion), XMLoadFloat4(&m_vPreQuaternion), m_fTrackPosition / 1.5f);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
	_vector vLerpTranslation = XMVectorLerp(XMLoadFloat3(&m_vEndTranslation), XMLoadFloat3(&m_vPreTranslation), m_fTrackPosition / 1.5f);
	XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));
}

void CSpringCamera::SetUp_Recovery()
{
	m_fTrackPosition = 0.f;
	if (false == m_isRecovery)
		m_isRecovery = true;
	m_fFixedDistance = m_fPreFixedDistance;
	XMStoreFloat4(&m_vEndQuaternion, m_pTransformCom->Get_Quaternion());
	//m_vEndTranslation = _float3(0.f, 0.f, 0.f);
}

void CSpringCamera::Ready_Event()
{
	m_pGameInstance->Subscribe<CAMERA_ACTION_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), [this](const CAMERA_ACTION_EVENT& event) {
		if (CAMERA_STATE::ACTION != m_eCameraState && true == event.isAction)
		{
			_matrix Matrix = XMLoadFloat4x4(&event.WorldMatrix);
			_vector vScale{}, vQuat{}, vTranslation{};
			XMMatrixDecompose(&vScale, &vQuat, &vTranslation, Matrix);
			XMStoreFloat4x4(&m_OwnerMatrix, XMMatrixRotationQuaternion(vQuat));
			m_iFrameIndex = -1;
			m_fTrackPosition = static_cast<_float>(event.iStart);
			m_fFirstFrame = m_fTrackPosition;
			m_eCameraState = CAMERA_STATE::ACTION;
			m_Frames = event.pFrame;
			XMStoreFloat4(&m_vPreQuaternion, m_pTransformCom->Get_Quaternion());
			m_vPreTranslation = _float3(0.f, 0.f, 0.f);
			m_fPreFixedDistance = m_fFixedDistance;
			m_fDuration = static_cast<_float>(event.iEnd - event.iStart);
			m_isMaintain = event.isMaintain;
		}
		else
		{
			SetUp_Recovery();
		}
		});
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
