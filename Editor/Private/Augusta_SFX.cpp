#include "EditorPch.h"
#include "Augusta_SFX.h"

CAugusta_SFX::CAugusta_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect { pDevice, pContext }
{
}

CAugusta_SFX::CAugusta_SFX(const CAugusta_SFX& Prototype)
	: CEdit_ScreenEffect { Prototype }
{
}

HRESULT CAugusta_SFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	if (FAILED(Ready_Textures()))
		return E_FAIL;

	return S_OK;
}

HRESULT CAugusta_SFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CAugusta_SFX::Priority_Update(_float fTimeDelta)
{

}

void CAugusta_SFX::Update(_float fTimeDelta)
{
}

void CAugusta_SFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CAugusta_SFX::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pBaseTexture->Bind_Shader_Resource(m_pShader, "g_DiffuseTexture")))
		CRASH("Failed to Bind BaseTexture");

	if (FAILED(m_pMaskTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind BaseTexture");

	if (FAILED(m_pSecondTexture->Bind_Shader_Resource(m_pShader, "g_NormalTexture")))
		CRASH("Failed to Bind BaseTexture");

	if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(m_pShader, "g_NoiseTexture")))
		CRASH("Failed to Bind BaseTexture");

	m_pShader->Begin(0);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CAugusta_SFX::Play()
{
}

void CAugusta_SFX::Stop()
{
}

void CAugusta_SFX::Reset()
{
}

HRESULT CAugusta_SFX::Ready_Textures()
{
	m_pBaseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Tile_262.png"), 1);
	ASSERT_CRASH(m_pBaseTexture);

	m_pMaskTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_300156.png"), 1);
	ASSERT_CRASH(m_pMaskTexture);

	m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Tile_30010.png"), 1);
	ASSERT_CRASH(m_pNoiseTexture);

	m_pSecondTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Tile_530002.png"), 1);
	ASSERT_CRASH(m_pSecondTexture);

	return S_OK;
}

CAugusta_SFX* CAugusta_SFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAugusta_SFX* pInstance = new CAugusta_SFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAugusta_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAugusta_SFX::Clone(void* pArg)
{
	CAugusta_SFX* pInstance = new CAugusta_SFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAugusta_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugusta_SFX::Free()
{
	__super::Free();

	Safe_Release(m_pBaseTexture);
	Safe_Release(m_pMaskTexture);
	Safe_Release(m_pNoiseTexture);
	Safe_Release(m_pSecondTexture);
}
