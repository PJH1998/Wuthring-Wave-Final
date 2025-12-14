#include "ClientPch.h"
#include "Excute_SFX.h"

CExcute_SFX::CExcute_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect{ pDevice, pContext }
{
}

CExcute_SFX::CExcute_SFX(const CExcute_SFX& Prototype)
	: CScreenEffect{ Prototype }
	, m_vScale{ Prototype.m_vScale }
	, m_vPos{ Prototype.m_vPos }
{
}

HRESULT CExcute_SFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vScale = _float2((m_vWinSize.x * 2.f), m_vWinSize.y * 2.f);
	m_vPos = _float2(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.4f);

	m_vEffectTime = _float2(0.f, 0.5f);

	return S_OK;
}

HRESULT CExcute_SFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Setting_Scale(m_vScale.x, m_vScale.y);
	m_pTransformCom->Rotation_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, 0.f, XMConvertToRadians(20.f)));
	Setting_Pos(m_vPos.x, m_vPos.y);

	return S_OK;
}

void CExcute_SFX::Priority_Update(_float fTimeDelta)
{

}

void CExcute_SFX::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		return;
	}

	_float fRatio = (1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime));

	_float fCurSizeY = m_vScale.y * fRatio;
	_float fSizeY = max(fCurSizeY, 0.1f);

	Setting_Scale((m_vScale.x), fSizeY);
}

void CExcute_SFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CExcute_SFX::Render()
{

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pMaskTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind MaskTexture");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(ENUM_CLASS(SHADER_SFX_BURST::AUGUSTA_SLASH));

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CExcute_SFX::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;
	Setting_Scale(m_vScale.x, m_vScale.y);
}

HRESULT CExcute_SFX::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_SFX_Slash"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pMaskTexture), nullptr)))
		ASSERT_CRASH(m_pMaskTexture);

	return S_OK;
}

CExcute_SFX* CExcute_SFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CExcute_SFX* pInstance = new CExcute_SFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CExcute_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CExcute_SFX::Clone(void* pArg)
{
	CExcute_SFX* pInstance = new CExcute_SFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CExcute_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CExcute_SFX::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
	Safe_Release(m_pMaskTexture);
}
