#include "EditorPch.h"
#include "Edit_ScreenEffect.h"

CEdit_ScreenEffect::CEdit_ScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CEdit_ScreenEffect::CEdit_ScreenEffect(const CEdit_ScreenEffect& Prototype)
	: CGameObject { Prototype }
	, m_ViewMatrix { Prototype.m_ViewMatrix }
	, m_ProjMatrix { Prototype.m_ProjMatrix }
{

}

HRESULT CEdit_ScreenEffect::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.0f, 1.f));

    return S_OK;
}

HRESULT CEdit_ScreenEffect::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_isActivate = false;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scale(_float3(g_iWinSizeX, g_iWinSizeY, 1.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f, 1.f));

    return S_OK;
}

void CEdit_ScreenEffect::Priority_Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Late_Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Render()
{
}

HRESULT CEdit_ScreenEffect::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ScreenEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

void CEdit_ScreenEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
}
