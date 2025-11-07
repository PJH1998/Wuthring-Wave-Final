#include "EditorPch.h"
#include "SQ_Camera_Edit.h"

CSQ_Camera_Edit::CSQ_Camera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSQ_Item_Edit { pDevice, pContext }
{
}

CSQ_Camera_Edit::CSQ_Camera_Edit(const CSQ_Camera_Edit& Prototype)
	: CSQ_Item_Edit { Prototype }
{
}

HRESULT CSQ_Camera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_Camera_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Camera_Edit")

    return S_OK;
}

void CSQ_Camera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Update(_float fTimeDelta)
{
	m_fTrackPosition += fTimeDelta * m_fTrackPerSec;
	if (m_fTrackPosition >= m_fEndFrame || m_iFrameIndex == Frames.size() - 1)
	{
		m_isActivate = false;
		return;
	}

	m_fRatio = (m_fTrackPosition - Frames[m_iFrameIndex].fStartFrame) / (Frames[m_iFrameIndex + 1].fStartFrame - Frames[m_iFrameIndex].fStartFrame);
	// 0. Default SetUp
	Default_SetUp();
	// 1. Quat SLerp
	Lerp_Quat();
	// 2. Spline (Catmull-Rom)
	Spline(fTimeDelta);

	// Frame Check
	if (m_fTrackPosition >= Frames[m_iFrameIndex + 1].fStartFrame)
		++m_iFrameIndex;
}

void CSQ_Camera_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	SQ_CAMERA_DATA* pData = static_cast<SQ_CAMERA_DATA*>(pArg);
	if (nullptr == pData)
		return;
	m_isActivate = true;

	m_iFrameIndex = 0;
	Frames = pData->Frames;
	m_fStartFrame = pData->fStartFrame;
	m_fEndFrame = pData->fEndFrame;
	m_fTrackPosition = m_fStartFrame;
}

void CSQ_Camera_Edit::Default_SetUp()
{
	_float fLerpSpeedRate = Frames[m_iFrameIndex].fSpeedRate * (1.f - m_fRatio) + Frames[m_iFrameIndex + 1].fSpeedRate * m_fRatio;
	m_fVelocity = m_fSpeed * fLerpSpeedRate;
}

void CSQ_Camera_Edit::Lerp_Quat()
{
	_vector vPreQuat = XMLoadFloat4(&Frames[m_iFrameIndex].vQuaternion);
	_vector vDestQuat = XMLoadFloat4(&Frames[m_iFrameIndex + 1].vQuaternion);
	_vector vLerpQuat = XMQuaternionSlerp(vPreQuat, vDestQuat, m_fRatio);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
}

void CSQ_Camera_Edit::Spline(_float fTimeDelta)
{
	_int iEnd = Frames.size() - 1;
	_int iIndex0 = clamp(m_iFrameIndex - 1,	0, iEnd);
	_int iIndex1 = clamp(m_iFrameIndex,			0, iEnd);
	_int iIndex2 = clamp(m_iFrameIndex + 1,	0, iEnd);
	_int iIndex3 = clamp(m_iFrameIndex + 2,	0, iEnd);
}

CSQ_Camera_Edit* CSQ_Camera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_Camera_Edit* pInstance = new CSQ_Camera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_Camera_Edit")

    return pInstance;
}

CGameObject* CSQ_Camera_Edit::Clone(void* pArg)
{
	CSQ_Camera_Edit* pClone = new CSQ_Camera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_Camera_Edit (Clone)")

	return pClone;
}

void CSQ_Camera_Edit::Free()
{
	__super::Free();
}
