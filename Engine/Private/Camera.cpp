#include "EnginePch.h"
#include "Camera.h"

#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject { Prototype }
{
}

void CCamera::OnShake(const _float3& vDir)
{
	m_isShake = true;
	m_fShakeTimeAcc = 0.f;
	XMStoreFloat3(&m_vShakeVelocity, XMLoadFloat3(&m_vShakeVelocity) + XMLoadFloat3(&vDir) * 10.f);
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	_uint iNumViewport = { 1 };
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewport, &ViewPort);

	CAMERA_DESC* pDesc = static_cast<CAMERA_DESC*>(pArg);
	m_fFovy = pDesc->fFovy;
	m_fAspect = ViewPort.Width / ViewPort.Height;
	m_fNear = pDesc->fNear;
	m_fFar = pDesc->fFar;
	m_fMouseSensor = pDesc->fMouseSensor;

	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&(pDesc->vEye)));
	m_pTransformCom->LookAt(XMLoadFloat4(&(pDesc->vAt)));

	m_fShakeDuration = 0.3f;
	m_fShakeStiffness = 70.f;
	m_fShakeDamp = 0.8f;

	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
}

void CCamera::Update(_float fTimeDelta)
{
}

void CCamera::Late_Update(_float fTimeDelta)
{
}

void CCamera::Render()
{
}

void CCamera::Update_Matrix()
{
	m_pGameInstance->Set_PrevTransformState(D3DTS::VIEW, m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::VIEW)]);
	m_pGameInstance->Set_PrevTransformState(D3DTS::PROJ, m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::PROJ)]);

	_matrix CurViewMatrix = m_pTransformCom->Get_WorldMatrix_Inv();
	_matrix CurProjMatrix = XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar);

	m_pGameInstance->Set_TransformState(D3DTS::VIEW, CurViewMatrix);
	m_pGameInstance->Set_TransformState(D3DTS::PROJ, CurProjMatrix);

	XMStoreFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::VIEW)], CurViewMatrix);
	XMStoreFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::PROJ)], CurProjMatrix);
}

void CCamera::Key_Move(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_UP) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_DOWN) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Right(fTimeDelta);
}

void CCamera::Mouse_Move()
{
	m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::UP), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::X) * m_fMouseSensor);
	m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::Y) * m_fMouseSensor);
}

void CCamera::Mouse_Move_Up()
{
	_float fTurnValueX = m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::X) * m_fMouseSensor;
	_float fTurnValueY = m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::Y) * m_fMouseSensor;
	
	m_pTransformCom->Turn_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, fTurnValueX, 0.f));

	// Camera 위 아래 방향 전환 시, Camera 시야가 위를 바라볼 때 뒤로 넘어가지 않도록 조정
	_vector vPreLook = m_pTransformCom->Get_State(STATE::LOOK);
	m_pTransformCom->Turn_Quaternion(XMQuaternionRotationAxis(m_pTransformCom->Get_State(STATE::RIGHT), fTurnValueY));
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVector4Normalize(vLook);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_float fDot = XMVector3Dot(vLook, vUp).m128_f32[0];
	if (cosf(XMConvertToRadians(10.f)) <= fabsf(fDot))
		m_pTransformCom->LookDir(XMVector4Normalize(vPreLook));
}

void CCamera::Shaking(_float fTimeDelta)
{
	if (false == m_isShake)
		return;

	m_fShakeTimeAcc += fTimeDelta;

	if (m_fShakeTimeAcc > m_fShakeDuration)
	{
		m_isShake = false;

		m_vShakeOffset = { 0.f, 0.f, 0.f };
		return;
	}

	_vector vVelocity = XMVector3TransformNormal(XMLoadFloat3(&m_vShakeVelocity), XMMatrixRotationQuaternion(m_pTransformCom->Get_Quaternion()));

	XMStoreFloat3(&m_vShakeOffset, XMLoadFloat3(&m_vShakeOffset) +  vVelocity * fTimeDelta);
	XMStoreFloat3(&m_vShakeVelocity, XMLoadFloat3(&m_vShakeVelocity) - (XMLoadFloat3(&m_vShakeOffset) * m_fShakeStiffness * fTimeDelta));
	XMStoreFloat3(&m_vShakeVelocity, XMLoadFloat3(&m_vShakeVelocity) * m_fShakeDamp);

	m_pTransformCom->Set_State(STATE::POSITION, m_pTransformCom->Get_State(STATE::POSITION) + XMLoadFloat3(&m_vShakeOffset));
}

void CCamera::Free()
{
	__super::Free();
}
