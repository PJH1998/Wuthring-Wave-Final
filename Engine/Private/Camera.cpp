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

	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
}

void CCamera::Update(_float fTimeDelta)
{
}

void CCamera::Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta)
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
	m_pGameInstance->Set_TransformState(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inv());
	m_pGameInstance->Set_TransformState(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));
}

void CCamera::Lerp_Distance(_float fTimeDelta)
{
	if (0.1f < fabsf(m_fFixedDistance - m_fDistance))
		m_fDistance += (m_fFixedDistance - m_fDistance) * fTimeDelta;// *m_fLerpSpeed;
	
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
	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::X) * m_fMouseSensor);

	_vector vPreLook = m_pTransformCom->Get_State(STATE::LOOK);
	m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::Y) * m_fMouseSensor);
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVector4Normalize(vLook);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_float fDot = XMVector3Dot(vLook, vUp).m128_f32[0];
	if (cosf(XMConvertToRadians(10.f)) <= fabsf(fDot))
		m_pTransformCom->LookDir(XMVector4Normalize(vPreLook));
}

void CCamera::Free()
{
	__super::Free();
}
