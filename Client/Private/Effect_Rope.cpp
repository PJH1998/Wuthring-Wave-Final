#include "ClientPch.h"
#include "Effect_Rope.h"

CEffect_Rope::CEffect_Rope(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Rope::CEffect_Rope(const CEffect_Rope& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Rope::Initialize_Prototype()
{

    return S_OK;
}

HRESULT CEffect_Rope::Initialize_Clone(void* pArg)
{

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

	//로프 이펙트는 굳이 툴로 만들 필요없을듯
	//그냥 내가 쓰고싶은 설정값들 설정해주면 됨.

	m_fLifeTime = 0.25f;
	m_iShaderPass = 3;


	m_isActivate = false;

    return S_OK;
}

void CEffect_Rope::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Rope::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
 
}

void CEffect_Rope::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	Update_Position();

	m_fCurrentTime += fTimeDelta;
	m_fSpawnTimer += fTimeDelta;

	//라이프타임 체크.
	while (!m_Samples.empty())
	{
		SAMPLE_DESC Desc = m_Samples.front();

		_float fAge = m_fCurrentTime - Desc.fSpawnTime;

		if (fAge >= m_fLifeTime)
		{
			m_Samples.pop_front();
		}
		else
			break;
	}

	//만약 오브젝트가 비활성화 되면 기록들까진 그리고 꺼지게 처맂 스
	if (!m_IsObectActive)
	{
		if (m_Samples.size() == 0)
		{
			m_isActivate = false;
			return;
		}

		if (m_Samples.size() >= 2)
		{
			const _float4* vCamPos = m_pGameInstance->Get_CamPos();
			m_pVIBufferCom->Update_Spectrum(m_Samples, m_Samples.size(), vCamPos);

			m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this);
		}
		
		return;
	}

	m_Samples.clear();

	//플레이어 위치로 갱신
	SAMPLE_DESC P1Desc = {};
	P1Desc.vPos, m_vPlayerPos;
	P1Desc.fSpawnTime = m_fCurrentTime;
	m_Samples.push_back(P1Desc);

	SAMPLE_DESC P2Desc = {};
	P2Desc.vPos = m_vRopeObjectPos;
	P2Desc.fSpawnTime = m_fCurrentTime;
	m_Samples.push_back(P2Desc);

	if (m_Samples.size() >= 2)
	{
		const _float4* vCamPos = m_pGameInstance->Get_CamPos();
		m_pVIBufferCom->Update_Spectrum(m_Samples, m_Samples.size(), vCamPos);
	}

    m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this);
}

void CEffect_Rope::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(m_iShaderPass);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

void CEffect_Rope::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	ROPE_INFO* pDesc = static_cast<ROPE_INFO*>(pArg);

	m_fCurrentTime = 0.f;
	m_fSpawnTimer = 0.f;

	m_pIsActive = pDesc->pIsActive;
	m_pPlayerMatrixPtr = pDesc->pPlayerMatrixPtr;
	m_pBoneMatrixPtr = pDesc->pBoneMatrixPtr;

	m_vRopeObjectPos = pDesc->vRopeObjectPos;

	m_isActivate = *m_pIsActive;
	m_IsObectActive = *m_pIsActive;
}

void CEffect_Rope::Update_Position()
{
	if ((m_pBoneMatrixPtr != nullptr) && (*m_pIsActive))
	{
		_float4x4 ObjectMatrix = *m_pPlayerMatrixPtr;
		_float4x4 BoneMatrix = *m_pBoneMatrixPtr;

		_matrix SpawnMatrix = XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&ObjectMatrix);

		_vector vScale = {};
		_vector vPos = {};
		_vector vRot = {};
		XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);

		//여기서 Pos가 플레이어의 손 뼈 위치
		XMStoreFloat3(&m_vPlayerPos, vPos);

	}
	else if (!(*m_pIsActive) && m_IsObectActive)
		m_IsObectActive = false;
}


HRESULT CEffect_Rope::Ready_Components()
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Spectrum"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(),TEXT("Prototype_Componenet_VIBuffer_Spectrum_tat"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_SpectrumTexture_T_Trail_10018"),
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_SpectrumTexture_T_Color_003"),
        TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Rope::Bind_ShaderResources()
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

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskSpeed", &m_fMaskSpeed, sizeof(_float))))
		return E_FAIL;

    return S_OK;
}

CEffect_Rope* CEffect_Rope::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Rope* pInstance = new CEffect_Rope(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Rope");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Rope::Clone(void* pArg)
{
    CEffect_Rope* pInstance = new CEffect_Rope(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Rope");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Rope::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pColorTextureCom);
}
