#include "EditorPch.h"
#include "SpringCamera_Edit.h"

CSpringCamera_Edit::CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CSpringCamera_Edit::CSpringCamera_Edit(const CSpringCamera_Edit& Prototype)
	: CCamera { Prototype }
{
}

HRESULT CSpringCamera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpringCamera_Edit::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");

	Ready_Component();

	m_fDistance = 100.f;
	m_fFixedDistance = 100.f;
	m_fLerpSpeed = 2.f;

	m_fStiffness = 3.f;

	m_fLockOnOffsetY = 50.f;

    return S_OK;
}

void CSpringCamera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Update(_float fTimeDelta)
{
	// GUI
	ImGui::Begin("Camera Tool");
	if(CAMERA_STATE::LOCKON == m_eCameraState)
		ImGui::Text("Lock On");
	ImGui::End();

	// Look Position Init
	m_vLookPosition = m_vTargetPosition;
	// Lock-On
	if (CAMERA_STATE::LOCKON == m_eCameraState)
	{
		Sorting_Target();
		Dual_Targeting(fTimeDelta);
	}

	// Target Transform Rest
	m_TargetTransforms.clear();
	m_pTargetTransform = nullptr;

	// Spring
	if (CAMERA_STATE::SPRING == m_eCameraState)
		Spring(fTimeDelta);
	else
		Lerp_Distance(fTimeDelta);

	// 0. Cam Rotate
	if (CAMERA_STATE::TARGET == m_eCameraState)
		__super::Mouse_Move_Up();
	Mouse_Scroll(fTimeDelta);
	// 1. 거리 제한으로 인한 간격 보정
	Compute_CamPos();
	// 2. Ray Cast 이용하여 지형, 오브젝트와 충돌
	Check_Ray();

	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	// Test
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::WB) == KEYSTATE::DOWN)
		Lock_On();
}

void CSpringCamera_Edit::Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta)
{
}

void CSpringCamera_Edit::Late_Update(_float fTimeDelta)
{
	if (CAMERA_STATE::LOCKON == m_eCameraState && 0 == m_TargetTransforms.size())
		m_eCameraState = CAMERA_STATE::TARGET;
}

void CSpringCamera_Edit::Render()
{
}

void CSpringCamera_Edit::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pDesc);
	if (nullptr == pTargetTransform)
		return;
	m_TargetTransforms.push_back(pTargetTransform);
}

void CSpringCamera_Edit::Lerp_Distance(_float fTimeDelta)
{
	if (0.1f < fabsf(m_fFixedDistance - m_fDistance))
		m_fDistance += (m_fFixedDistance - m_fDistance) * fTimeDelta * m_fLerpSpeed;

}

void CSpringCamera_Edit::Mouse_Scroll(_float fTimeDelta)
{
	m_fFixedDistance -= m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::WHEEL) * fTimeDelta * 20.f;
}

void CSpringCamera_Edit::Spring(_float fTimeDelta)
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

void CSpringCamera_Edit::Compute_CamPos()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// Offset Y Adjust
	_vector vTargetPos;
	vTargetPos = XMLoadFloat4(&m_vTargetPosition);

	_vector vCamPos = XMVectorSetW(vTargetPos - vLook * m_fDistance, 1.f);
	vCamPos.m128_f32[1] += m_fOffsetY;
	//XMStoreFloat4(&m_vCurrentPosition, vCamPos);
	m_pTransformCom->Set_State(STATE::POSITION, vCamPos);
}

void CSpringCamera_Edit::Check_Ray()
{
	_vector vCamPos = m_pTransformCom->Get_State(STATE::POSITION);
	//_vector vCamPos = XMLoadFloat4(&m_vCurrentPosition);
	_vector vStartPos = XMLoadFloat4(&m_vLookPosition);
	_float4 vOut;
	if (true == m_pGameInstance->Ray_Cast(vStartPos, vCamPos, &vOut))
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vOut));

}

_vector CSpringCamera_Edit::Lerp_Quat(_float fTimeDelta)
{
	_vector vPreQuat = m_pTransformCom->Get_Quaternion();
	// 현재 Dir
	_vector vLookDir = XMLoadFloat4(&m_vLookPosition) - (m_pTransformCom->Get_State(STATE::POSITION) + XMVectorSet(0.f, m_fLockOnOffsetY, 0.f, 0.f));
	// 목표 Dir
	m_pTransformCom->LookDir(vLookDir);
	_vector vCurrentQuat = m_pTransformCom->Get_Quaternion();
	
	return XMQuaternionSlerp(vPreQuat, vCurrentQuat, 1 - exp(-1.f * fTimeDelta / 0.1f));
}

void CSpringCamera_Edit::Sorting_Target()
{
	sort(m_TargetTransforms.begin(), m_TargetTransforms.end(), [this](CTransform* pSrcTransform, CTransform* pDstTransform)->_bool {
		_float fSrcDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pSrcTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pDstTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

	if (0 < m_TargetTransforms.size())
		m_pTargetTransform = m_TargetTransforms[0];
}

void CSpringCamera_Edit::Dual_Targeting(_float fTimeDelta)
{
	if (nullptr == m_pTargetTransform)
		return;

	XMStoreFloat4(&m_vLookPosition, (XMLoadFloat4(&m_vTargetPosition) + m_pTargetTransform->Get_State(STATE::POSITION)) * 0.5f);
	_vector vLerpQuat = Lerp_Quat(fTimeDelta);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
}

void CSpringCamera_Edit::Ready_Component()
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	RigidbodyDesc.vExtent = _float3(400.f, 400.f, 400.f);
	//RigidbodyDesc.vExtent = _float3(100.f, 100.f, 100.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"), 
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			OnCollide_During(iLayer, pDesc, Manifold);
		});
}

CSpringCamera_Edit* CSpringCamera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpringCamera_Edit* pInstance = new CSpringCamera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit");
		Safe_Release(pInstance);
	}

    return pInstance;
}

CGameObject* CSpringCamera_Edit::Clone(void* pArg)
{
	CSpringCamera_Edit* pClone = new CSpringCamera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpringCamera_Edit::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);
	m_TargetTransforms.clear();
}
