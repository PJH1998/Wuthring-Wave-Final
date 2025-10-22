#include "EnginePch.h"
#include "StaticObject.h"

CStaticObject::CStaticObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CStaticObject::CStaticObject(const CStaticObject& Prototype)
	: CGameObject { Prototype }
{
}

_float CStaticObject::Compute_Distance(const _fvector& vCamPos)
{
    return XMVectorGetX(XMVector3Length(vCamPos - m_pTransformCom->Get_State(STATE::POSITION)));
}

void CStaticObject::Free()
{
	__super::Free();
}
