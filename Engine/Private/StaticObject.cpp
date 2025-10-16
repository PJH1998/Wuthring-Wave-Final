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

void CStaticObject::Free()
{
	__super::Free();
}
