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

    return S_OK;
}

void CSpringCamera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
	//__super::Key_Move(fTimeDelta);
	// 0. Cam Rotate
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::RB) == KEYSTATE::PRESS)
		__super::Mouse_Move_Up();
	Mouse_Scroll(fTimeDelta);

	// 1. Spring

	// 2. 거리 제한으로 인한 간격 보정
	Compute_CamPos();
	// 3. Ray Cast 이용하여 지형, 오브젝트와 충돌
	Check_Ray();
}

void CSpringCamera_Edit::Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta)
{
}

void CSpringCamera_Edit::Late_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Render()
{
}

void CSpringCamera_Edit::Mouse_Scroll(_float fTimeDelta)
{
	m_fDistance -= m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::WHEEL) * fTimeDelta * 20.f;
}

void CSpringCamera_Edit::Compute_CamPos()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// Offset Y Adjust
	m_vTargetPosition.y += m_fOffsetY;
	_vector vTargetPos;
	vTargetPos = XMLoadFloat4(&m_vTargetPosition);

	_vector vCamPos = XMVectorSetW(vTargetPos - vLook * m_fDistance , 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vCamPos);
}

void CSpringCamera_Edit::Check_Ray()
{
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	_float4 vOut;
	if (true == m_pGameInstance->Ray_Cast(vMyPos, vMyPos + vLook * m_fDistance, &vOut))
	{
		cout << "맞음!" << endl;
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vOut));
	}
	//else
	//	cout << "안맞음!" << endl;

}

void CSpringCamera_Edit::Ready_Component()
{
	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//ColliderDesc.vPos = _float3(0.f, 0.f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::CAMERA);
	ColliderDesc.fHeight = 5.f;
	ColliderDesc.fRadius = 5.f; //m_pGameInstance->Rand(5.f, 20.f);
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
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

	Safe_Release(m_pColliderCom);
}
