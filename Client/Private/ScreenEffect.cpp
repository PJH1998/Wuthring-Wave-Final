#include "ClientPch.h"
#include "ScreenEffect.h"

CScreenEffect::CScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CScreenEffect::CScreenEffect(const CScreenEffect& Prototype)
	: CGameObject { Prototype }
	, m_ViewMatrix { Prototype.m_ViewMatrix }
	, m_ProjMatrix { Prototype.m_ProjMatrix }
	, m_vWinSize{ Prototype.m_vWinSize }
{
}

HRESULT CScreenEffect::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vWinSize = _float2(static_cast<_float>(g_iWinSizeX), static_cast<_float>(g_iWinSizeY));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.0f, 1.f));

    return S_OK;
}

HRESULT CScreenEffect::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_isActivate = false;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Setting_Scale(m_vWinSize.x, m_vWinSize.y);

	Setting_Pos(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.5f);


    return S_OK;
}

void CScreenEffect::Priority_Update(_float fTimeDelta)
{
}

void CScreenEffect::Update(_float fTimeDelta)
{
}

void CScreenEffect::Late_Update(_float fTimeDelta)
{
}

void CScreenEffect::Render()
{
}

void CScreenEffect::Setting_Scale(_float fSizeX, _float fSizeY)
{
	if (fSizeX > 0.f && fSizeY > 0.f)
		m_pTransformCom->Scale(_float3(fSizeX, fSizeY, 1.f));
}

void CScreenEffect::Setting_Pos(_float fPosX, _float fPosY)
{
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(fPosX - (g_iWinSizeX * 0.5f), -fPosY + g_iWinSizeY * 0.5f, 0.f, 1.f));
}

HRESULT CScreenEffect::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ScreenEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

void CScreenEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
}
