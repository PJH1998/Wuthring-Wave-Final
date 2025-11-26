#include "ClientPch.h"
#include "CLeviatan.h"

CLeviatan::CLeviatan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CLeviatan::CLeviatan(const CLeviatan& Prototype)
	: CActor { Prototype }
{
}

HRESULT CLeviatan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeviatan::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CLeviatan::Priority_Update(_float fTimeDelta)
{
}

void CLeviatan::Update(_float fTimeDelta)
{
}

void CLeviatan::Late_Update(_float fTimeDelta)
{
}

void CLeviatan::Render()
{
}

void CLeviatan::Render_Shadow()
{
}

void CLeviatan::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLeviatan::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
}

void CLeviatan::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CLeviatan::Object_Func(const _wstring& wStrObjectTag)
{
}

HRESULT CLeviatan::Bind_Resources()
{
	return S_OK;
}

void CLeviatan::Ready_Component(LEVIATAN_DESC* pDesc)
{
}

void CLeviatan::Ready_PartObjects(LEVIATAN_DESC* pDesc)
{
}

void CLeviatan::Calculate_PosAndDir()
{
}

void CLeviatan::Reset_Condition(_float fTimeDelta)
{
}

void CLeviatan::After_Condition(_float fTimeDelta)
{
}

void CLeviatan::OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLeviatan::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLeviatan::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer)
{
}

void CLeviatan::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLeviatan::TurnFix()
{
}

void CLeviatan::TurnLerp(_bool isActive)
{
}

void CLeviatan::DistanceInterpolate(_bool isActive)
{
}

_bool CLeviatan::isKnockDown()
{
	return _bool();
}

_bool CLeviatan::isAttackEnable()
{
	return _bool();
}

_bool CLeviatan::DodgeCooldown()
{
	return _bool();
}

_bool CLeviatan::Attack(_uint iIndex, _float fInterval)
{
	return _bool();
}

void CLeviatan::Attack_Arrange()
{
}

_bool CLeviatan::Back()
{
	return _bool();
}

_bool CLeviatan::Front()
{
	return _bool();
}

_bool CLeviatan::Left()
{
	return _bool();
}

_bool CLeviatan::Right()
{
	return _bool();
}

CLeviatan* CLeviatan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeviatan* pInstance = new CLeviatan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Leviatan");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeviatan::Clone(void* pArg)
{
	CLeviatan* pClone = new CLeviatan(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Leviatan (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLeviatan::Free()
{
	__super::Free();

}
