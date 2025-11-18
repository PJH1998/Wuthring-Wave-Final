#include "ClientPch.h"
#include "Augusta_UltiPostSFX.h"

CAugusta_UltiPostSFX::CAugusta_UltiPostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect { pDevice, pContext }
{
}

CAugusta_UltiPostSFX::CAugusta_UltiPostSFX(const CAugusta_UltiPostSFX& Prototype)
	: CScreenEffect{ Prototype }
	, m_fRadialLengthScale { Prototype.m_fRadialLengthScale }
	, m_RadialData { Prototype.m_RadialData }
	, m_vEffectTime { Prototype.m_vEffectTime }
{
}

HRESULT CAugusta_UltiPostSFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	/*m_SlashData.vSlashPoint0 = _float2(1920.f, 884.f);
	m_SlashData.vSlashPoint1 = _float2(0.f, 784.f);
	m_SlashData.fOffset = 100.f;
	m_SlashData.fIntensity = 1.f;*/

	m_fRadialLengthScale = -0.4f;

	m_RadialData.fMinDistance = 0.f;
	m_RadialData.fMaxDistance = 0.65f;
	m_RadialData.fLengthScale = -0.4f;
	m_RadialData.vPivot = _float2(0.5f, 0.4f);

	m_vEffectTime = _float2(0.f, 0.5f);

    return S_OK;
}

HRESULT CAugusta_UltiPostSFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Buffer()))
		return E_FAIL;

	if (FAILED(Ready_Texture()))
		return E_FAIL;

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	//memcpy(SubResource.pData, &m_SlashData, sizeof(SLASH_DATA));
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

    return S_OK;
}

void CAugusta_UltiPostSFX::Priority_Update(_float fTimeDelta)
{
}

void CAugusta_UltiPostSFX::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		return;
	}

	m_RadialData.fLengthScale = (m_fRadialLengthScale * (1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime)));

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);
}

void CAugusta_UltiPostSFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::POST_SFX, this)))
		return;
}

void CAugusta_UltiPostSFX::Render()
{

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("vScreenSize", &m_vWinSize, sizeof(_float2))))
		CRASH("Failed to Bind vScreenSize");

	if(FAILED(m_pNoiseTexture->Bind_Shader_Resource(m_pShader, "g_NoiseTexture")))
		CRASH("Failed to Bind NoseTexture");

	if (FAILED(m_pShader->Bind_Texture("g_SceneTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed to Bind SceneTexture");

	m_pShader->Begin(ENUM_CLASS(SHADER_SCREENEFFECT::AUGUSTA_ULTI_POST));

	m_pContext->PSSetConstantBuffers(0, 1, &m_pBuffer);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CAugusta_UltiPostSFX::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;
	m_RadialData.fLengthScale = m_fRadialLengthScale;
}

HRESULT CAugusta_UltiPostSFX::Ready_Texture()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Noise"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pNoiseTexture), nullptr)))
		ASSERT_CRASH(m_pNoiseTexture);

	return S_OK;
}

HRESULT CAugusta_UltiPostSFX::Ready_Buffer()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(RADIAL_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Create : Constant Buffer");

	return S_OK;
}

CAugusta_UltiPostSFX* CAugusta_UltiPostSFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAugusta_UltiPostSFX* pInstance = new CAugusta_UltiPostSFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAugusta_UltiPostSFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAugusta_UltiPostSFX::Clone(void* pArg)
{
	CAugusta_UltiPostSFX* pInstance = new CAugusta_UltiPostSFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAugusta_UltiPostSFX");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CAugusta_UltiPostSFX::Free()
{
	__super::Free();

	Safe_Release(m_pBuffer);
	Safe_Release(m_pNoiseTexture);
}
