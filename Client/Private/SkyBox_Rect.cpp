#include "ClientPch.h"
#include "SkyBox_Rect.h"

CSkyBox_Rect::CSkyBox_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CSkyBox_Rect::CSkyBox_Rect(const CSkyBox_Rect& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CSkyBox_Rect::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSkyBox_Rect::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;



	return E_NOTIMPL;
}

void CSkyBox_Rect::Priority_Update(_float fTimeDelta)
{
}

void CSkyBox_Rect::Update(_float fTimeDelta)
{
}

void CSkyBox_Rect::Late_Update(_float fTimeDelta)
{
}

void CSkyBox_Rect::Render()
{
}

void CSkyBox_Rect::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

HRESULT CSkyBox_Rect::Ready_Components()
{


	return S_OK;
}

CSkyBox_Rect* CSkyBox_Rect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CSkyBox_Rect::Clone(void* pArg)
{
	return nullptr;
}

void CSkyBox_Rect::Free()
{
}
