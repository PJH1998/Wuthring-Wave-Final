#include "ClientPch.h"
#include "Spectrum.h"

CSpectrum::CSpectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CSpectrum::CSpectrum(const CSpectrum& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CSpectrum::Initialize_Prototype()
{

    return S_OK;
}

HRESULT CSpectrum::Initialize_Clone(void* pArg)
{
    SPECTRUM_DESC* pDesc = static_cast<SPECTRUM_DESC*>(pArg);
	m_tDesc = *pDesc;

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_iShaderPass = m_tDesc.iShaderPass;
    m_fLifeTime = m_tDesc.fLifeTime;
    m_fGeneration = m_tDesc.fGeneration;
    
	m_isActivate = false;

    return S_OK;
}

void CSpectrum::Priority_Update(_float fTimeDelta)
{
}

void CSpectrum::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;


    Update_Position();

    m_fCurrentTime += fTimeDelta;
    m_fSpawnTimer += fTimeDelta;
	m_fSweep += fTimeDelta;
	 
    //라이프타임 체크.
    while (!m_Samples.empty())
    {
        SAMPLE_DESC Desc = m_Samples.front();

        _float fAge = m_fCurrentTime - Desc.fSpawnTime;
        
        if (fAge >= m_fLifeTime)
        {
            m_Samples.pop_front();
            m_SamleCount -= 1;
        }
        else
            break;
    }

    if (m_fSpawnTimer >= m_fGeneration)
    {
        XMVECTOR vPrevPos = XMLoadFloat3(&m_vPreviousPos);
        _float fPrevLength = XMVectorGetX(XMVector3Length(vPrevPos));

		XMVECTOR vCurrentPos = m_UpdatePosition;
        _float fCurrentLength = XMVectorGetX(XMVector3Length(vCurrentPos));

        _float fDistance = fCurrentLength - fPrevLength;

        if (fDistance > m_fMinDistance)
        {
            SAMPLE_DESC Desc = {};
            XMStoreFloat3(&Desc.vPos, m_UpdatePosition);
            Desc.fSpawnTime = m_fCurrentTime;

            m_SamleCount += 1;
            m_Samples.push_back(Desc);

			XMStoreFloat3(&m_vPreviousPos, m_UpdatePosition);
            m_fSpawnTimer = 0.f;
        }
    }

    if (m_SamleCount >= 2)
    {
        const _float4* vCamPos = m_pGameInstance->Get_CamPos();
        m_pVIBufferCom->Update_Spectrum(m_Samples, m_SamleCount, vCamPos);
    }

}

void CSpectrum::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CSpectrum::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(m_iShaderPass);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

void CSpectrum::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	SPECTRUM_INFO* pDesc = static_cast<SPECTRUM_INFO*>(pArg);

	m_fCurrentTime = 0.f;
	m_fSweep = 0.f;

	m_pIsActive = pDesc->pIsActive;
	m_pObjectMatrixPtr = pDesc->pModelMarixPtr;
	m_pBoneMatrixPtr = pDesc->pBoneMatrixPtr;

	m_isActivate = *m_pIsActive;
}

void CSpectrum::Update_Position()
{
	if (m_pBoneMatrixPtr != nullptr)
	{

		_float4x4 ObjectMatrix = *m_pObjectMatrixPtr;
		_float4x4 BoneMatrix = *m_pBoneMatrixPtr;

		_matrix SpawnMatrix = XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&ObjectMatrix);

		_vector vScale = {};
		_vector vPos = {};
		_vector vRot = {};
		XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);

		m_UpdatePosition = vPos;

	}
	else if (m_pObjectMatrixPtr != nullptr)
	{

		_float4x4 ObjectMatrix = *m_pObjectMatrixPtr;
		_matrix SpawnMatrix = XMLoadFloat4x4(&ObjectMatrix);

		_vector vScale = {};
		_vector vPos = {};
		_vector vRot = {};
		XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);

		m_UpdatePosition = vPos;
	}
}


HRESULT CSpectrum::Ready_Components(SPECTRUM_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Spectrum"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(m_tDesc.CurrentLevel, Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    //텍스처 여러개 써야하는데 어떻게 할지 고민해보자
    if (FAILED(CGameObject::Add_Component(m_tDesc.CurrentLevel, Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(m_tDesc.CurrentLevel, Desc.strColorTextureTag,
        TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CSpectrum::Bind_ShaderResources()
{
  
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
            return E_FAIL;
 

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pColorTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture", 0)))
        return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Time", &m_fCurrentTime, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Sweep", &m_fSweep, sizeof(_float))))
		return E_FAIL;

    return S_OK;
}

CSpectrum* CSpectrum::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSpectrum* pInstance = new CSpectrum(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CSpectrum");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSpectrum::Clone(void* pArg)
{
    CSpectrum* pInstance = new CSpectrum(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CSpectrum");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSpectrum::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pColorTextureCom);
}
