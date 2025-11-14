#include "Editorpch.h"
#include "Trail_Mesh.h"

CTrail_Mesh::CTrail_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTrail_Mesh::CTrail_Mesh(const CTrail_Mesh& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CTrail_Mesh::Initialize_Prototype()
{
    XMStoreFloat4x4(&m_ComBindMatrix, XMMatrixIdentity());

    return S_OK;
}

HRESULT CTrail_Mesh::Initialize_Clone(void* pArg)
{
    TRAILMESH_DESC* pDesc = static_cast<TRAILMESH_DESC*>(pArg);
	
	m_IsDissolve = pDesc->IsDissolve;
	m_IsDistortion = pDesc->IsDistortion;

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_iShaderPass = pDesc->iShaderPass;
    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

    //트레일 전용 값
    m_fSweepSpeed = pDesc->fSweep;
    m_fSweepWitdh = pDesc->fSweepWitdh;

    _vector Pos = XMVectorSet(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    m_IsRoot = pDesc->IsRootOn;
	
	m_fSoft = pDesc->fSoft;
	m_fColorSpeed = pDesc->fColorSpeed;
	m_fMaskSpeed = pDesc->fMaskSpeed;
	m_fAlpha = pDesc->fAlpha;
	m_fColorGain = pDesc->fColorGain;
	m_fColorGamma = pDesc->fColorGamma;
	m_iDirFalg = pDesc->iDirFlag;
	m_iMaskFlag = pDesc->iMaskFlag;

	m_fDistortionWeight = pDesc->fDistortionWeight;

    //임시처리
    m_isActivate = false;


    XMStoreFloat4x4(&m_ComBindMatrix, XMMatrixIdentity());
    Root_Transform(XMLoadFloat4x4(&m_ComBindMatrix));

    return S_OK;
}

void CTrail_Mesh::Priority_Update(_float fTimeDelta)
{
}

void CTrail_Mesh::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    //여기서 Sweep 계산 후 셰이더에 바인딩 해줘야 함.
    m_fSweep += fTimeDelta * m_fSweepSpeed;
    m_fColorSweep += fTimeDelta * m_fColorSpeed;
	m_fMaskSweep += fTimeDelta * m_fMaskSpeed;
    m_vLifeTime.x += fTimeDelta;

	//if (m_IsRoot)
	//	Update_Transform();

    if (m_vLifeTime.x >= m_vLifeTime.y)
    {
        m_fSweep = 0.f;
        m_isActivate = false;
        m_fColorSweep = 0.f;
        m_vLifeTime.x = 0.f;
		m_fMaskSweep = 0.f;
    }

    if (m_fSweep >= 1.f + m_fSweepWitdh)
    {
        m_fSweep = 0.f;
        m_isActivate = false;
        m_fColorSweep = 0.f;
		m_fMaskSweep = 0.f;
		m_vLifeTime.x = 0.f;
    }
}

void CTrail_Mesh::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	if(m_IsDistortion)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::DISTORTION, this);
	else
		m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this);
}

void CTrail_Mesh::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(m_iShaderPass);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

void CTrail_Mesh::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    /*if (_bool* IsActivate = static_cast<_bool*>(pArg))
        m_isActivate = *IsActivate;

    m_fSweep = 0.f;
    m_fColorSweep = 0.f;
    m_vLifeTime.x = 0.f;
    Root_Transform(WorldMatrix);*/

	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

	if (m_isActivate)
	{
		if (pDesc->pBoneMatrixPtr != nullptr)
		{
			//뼈에 붙여야한다는 것. 근데 파티클은 뼈에 안붙여도 될듯?
			m_pBoneMatrixPtr = pDesc->pBoneMatrixPtr;
			m_pObjectMatrixPtr = pDesc->pObjectMatrixPtr;
			m_IsRoot = true;
			m_OffsetMatrix = WorldMatrix;
			m_fSweep = 0.f;
			m_fColorSweep = 0.f;
			m_vLifeTime.x = 0.f;
			m_fMaskSweep = 0.f;
		}
		else if (pDesc->pBoneMatrixPtr == nullptr)
		{
			//기존처리
			m_vLifeTime.x = 0.f;
			Root_Transform(WorldMatrix);
			m_IsRoot = false;
			m_fSweep = 0.f;
			m_fColorSweep = 0.f;
			m_vLifeTime.x = 0.f;
			m_fMaskSweep = 0.f;
		}
	}
}

void CTrail_Mesh::Root_Transform(_fmatrix WorldMatrix)
{

    XMStoreFloat4x4(&m_ComBindMatrix,
        m_pTransformCom->Get_WorldMatrix()
        * WorldMatrix);
}

void CTrail_Mesh::Update_Transform()
{
	if (m_pBoneMatrixPtr == nullptr)
		return;

	_matrix OffsetMatrix = m_OffsetMatrix;
	
	_float4x4 ObjectMatrix = *m_pObjectMatrixPtr;
	_float4x4 BoneMatrix = *m_pBoneMatrixPtr;
	
	_matrix SpawnMatrix = XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&ObjectMatrix);
	
	_vector vScale = {};
	_vector vPos = {};
	_vector vRot = {};
	XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);
	
	_matrix OffsetSpawnMatrix = XMMatrixRotationQuaternion(vRot) * XMMatrixTranslationFromVector(vPos);
	
	XMStoreFloat4x4(&m_ComBindMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		OffsetMatrix * OffsetSpawnMatrix);
}

HRESULT CTrail_Mesh::Ready_Components(TRAILMESH_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxTrailMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strColorTextureTag,
        TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
        return E_FAIL;

	if (Desc.IsDissolve)
	{
		if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strDissolveTextureTag,
			TEXT("Com_DissolveTexture"), reinterpret_cast<CComponent**>(&m_pDissolveTextureCom), nullptr)))
			return E_FAIL;
	}
	if (Desc.IsDistortion)
	{
		if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strDistortionTextureTag,
			TEXT("Com_DistortionTexture"), reinterpret_cast<CComponent**>(&m_pDistortionTextureCom), nullptr)))
			return E_FAIL;
	}

    return S_OK;
}

HRESULT CTrail_Mesh::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_ComBindMatrix)))
            return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pColorTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture", 0)))
        return E_FAIL;

	if (m_IsDissolve)
	{
		if (FAILED(m_pDissolveTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DissolveTexture", 0)))
			return E_FAIL;
	}

	if (m_IsDistortion)
	{
		if (FAILED(m_pDistortionTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DistortionTexture", 0)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Value("g_DistortionWeight", &m_fDistortionWeight, sizeof(_float))))
			return E_FAIL;

	}

    //셰이더에 바인딩 해주자.
    if (FAILED(m_pShaderCom->Bind_Value("g_Sweep", &m_fSweep, sizeof(_float))))
        return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Soft", &m_fSoft, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Alpha", &m_fAlpha, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_ColorGain", &m_fColorGain, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_ColorGamma", &m_fColorGamma, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskSweep", &m_fMaskSweep, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskSpeed", &m_fMaskSpeed, sizeof(_float))))
		return E_FAIL;

    if(FAILED(m_pShaderCom->Bind_Value("g_SweepWitdh", &m_fSweepWitdh, sizeof(_float))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_ColorSpeed", &m_fColorSweep, sizeof(_float))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_Dir", &m_iDirFalg, sizeof(_int))))
        return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskFlag", &m_iMaskFlag, sizeof(_int))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Time", &m_vLifeTime.x, sizeof(_float))))
		return E_FAIL;

    return S_OK;
}

CTrail_Mesh* CTrail_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTrail_Mesh* pInstance = new CTrail_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTrail_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTrail_Mesh::Clone(void* pArg)
{
    CTrail_Mesh* pInstance = new CTrail_Mesh(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CTrail_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTrail_Mesh::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pColorTextureCom);
	Safe_Release(m_pDissolveTextureCom);
	Safe_Release(m_pDistortionTextureCom);
}
