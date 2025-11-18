#include "ClientPch.h"
#include "Augusta_UltiSFX.h"

CAugusta_UltiSFX::CAugusta_UltiSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect { pDevice, pContext }
{
}

CAugusta_UltiSFX::CAugusta_UltiSFX(const CAugusta_UltiSFX& Prototype)
	: CScreenEffect { Prototype }
	, m_vScale { Prototype.m_vScale }
	, m_vPos { Prototype.m_vPos }
	, m_vLutTime{ Prototype.m_vLutTime }
	, m_vEffectTime { Prototype.m_vEffectTime }
	, m_iSFXLutIndex { Prototype.m_iSFXLutIndex }
	, m_fSFXLutIntensity { Prototype.m_fSFXLutIntensity }
{
}

HRESULT CAugusta_UltiSFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vScale = _float2((m_vWinSize.x * 2.f), m_vWinSize.y * 2.f);
	m_vPos = _float2(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.4f);

	m_vEffectTime = _float2(0.f, 0.5f);
	m_vLutTime = _float2(0.f, 0.3f);


	m_iSFXLutIndex = 3;
	m_fSFXLutIntensity = 1.f;

	return S_OK;
}

HRESULT CAugusta_UltiSFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Textures()))
		return E_FAIL;

	Setting_Scale(m_vScale.x, m_vScale.y);
	m_pTransformCom->Rotation_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, 0.f, XMConvertToRadians(20.f)));
	Setting_Pos(m_vPos.x, m_vPos.y);

	return S_OK;
}

void CAugusta_UltiSFX::Priority_Update(_float fTimeDelta)
{

}

void CAugusta_UltiSFX::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	if(m_fCurrentTime >= m_vLutTime.y)
		m_pGameInstance->Setting_LUT(m_iPrevLutIndex, m_fPrevLutIntensity, m_PrevIsDynamicLUT);

	if (m_fCurrentTime >= m_vEffectTime.y)
	{	
		m_isActivate = false;
		return;
	}

	_float fCurSizeY = m_vScale.y * (1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime));
	_float fSizeY = max(fCurSizeY, 0.1f);

	Setting_Scale((m_vScale.x), fSizeY);
}

void CAugusta_UltiSFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CAugusta_UltiSFX::Render()
{

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pMaskTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind MaskTexture");

	if(FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");
	
	m_pShader->Begin(ENUM_CLASS(SHADER_SCREENEFFECT::AUGUSTA_ULTI));

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CAugusta_UltiSFX::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;
	Setting_Scale(m_vScale.x, m_vScale.y);

	m_pGameInstance->Get_Current_LutSetting(&m_iPrevLutIndex, &m_fPrevLutIntensity, &m_PrevIsDynamicLUT);
	m_pGameInstance->Setting_LUT(m_iSFXLutIndex, m_fSFXLutIntensity, true);
}

HRESULT CAugusta_UltiSFX::Ready_Textures()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Slash"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pMaskTexture), nullptr)))
		ASSERT_CRASH(m_pMaskTexture);

	return S_OK;
}

CAugusta_UltiSFX* CAugusta_UltiSFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAugusta_UltiSFX* pInstance = new CAugusta_UltiSFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAugusta_UltiSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAugusta_UltiSFX::Clone(void* pArg)
{
	CAugusta_UltiSFX* pInstance = new CAugusta_UltiSFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAugusta_UltiSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugusta_UltiSFX::Free()
{
	__super::Free();

	Safe_Release(m_pMaskTexture);
}
