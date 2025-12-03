#include "ClientPch.h"
#include "MotionTrail.h"

CMotionTrail::CMotionTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMotionTrail::CMotionTrail(const CMotionTrail& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CMotionTrail::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMotionTrail::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_isActivate = false;

	m_iNumTrails = 0;
	
	return S_OK;
}

void CMotionTrail::Priority_Update(_float fTimeDelta)
{
}

void CMotionTrail::Update(_float fTimeDelta)
{
	m_fCurDuration += fTimeDelta;

	if (m_fCurDuration >= m_fDuration && m_Datas.empty())
	{
		m_isActivate = false;

		Safe_Release(m_pOwnerModel);
		Safe_Release(m_pOwnerTransform);

		return;
	}

	m_fCurInterval += fTimeDelta;

	if (m_fCurInterval >= m_fInterval && m_fCurDuration < m_fDuration)
	{
		Add_MotionTrail();
		m_fCurInterval -= m_fInterval;
	}

	if(false == m_Datas.empty())
		Update_Datas(fTimeDelta);
}

void CMotionTrail::Late_Update(_float fTimeDelta)
{
	if (false == m_isActivate)
		return;

	m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this);
}

void CMotionTrail::Render_OutLine()
{
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed to Bind View Matrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed to Bind Proj Matrix");

	for (_uint i = 0; i < m_iNumTrails; ++i)
	{
		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_Datas[i].WorldMatrix)))
			CRASH("Failed to Bind MotionTrail World Matrix");

		if (FAILED(m_pShader->Bind_Value("g_vLifeTime", &m_Datas[i].vMotionLifeTime, sizeof(_float2))))
			CRASH("Failed to Bind MotionTrail LifeTime");
			
		if (FAILED(m_pShader->Bind_Value("g_vTrailColor", &m_Datas[i].vColor, sizeof(_float4))))
			CRASH("Failed to Bind MotionTrail Color");

		for (_uint j = 0; j < m_iNumMeshes; j++)
		{
			if (FAILED(Bind_BoneMatrices(i, j)))
				return;

			m_pShader->Begin(m_iShaderPassIndex);

			m_pOwnerModel->Render(j);
		}
	}
}

void CMotionTrail::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;

	m_fCurDuration = 0.f;
	m_fCurInterval = 0.f;

	MOTION_TRAIL_DESC* pDesc = static_cast<MOTION_TRAIL_DESC*>(pArg);
	m_fInterval = pDesc->fInterval;
	m_fDuration = pDesc->fDuration;
	m_fMotionLifeTime = pDesc->fMotionLifeTime;
	m_vColor = pDesc->vColor;

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	m_pOwnerModel = pDesc->pModel;
	Safe_AddRef(m_pOwnerModel);

	m_pOwnerTransform = pDesc->pTransform;
	Safe_AddRef(m_pOwnerTransform);

	m_iNumMeshes = m_pOwnerModel->Get_NumMesh();

	m_iNumBones.resize(m_iNumMeshes);

	for (_uint i = 0; i < m_iNumMeshes; ++i)
		m_iNumBones[i] = m_pOwnerModel->Get_NumBones(i);

	Add_MotionTrail();
}

HRESULT CMotionTrail::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MotionTrail"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

void CMotionTrail::Add_MotionTrail()
{
	MOTION_TRAIL_DATA Data = {};

	Data.BoneMatrices.resize(m_iNumMeshes);

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		Data.BoneMatrices[i].resize(m_iNumBones[i]);

		m_pOwnerModel->Copy_BoneMatrices(Data.BoneMatrices[i].data(), i);
	}

	XMStoreFloat4x4(&Data.WorldMatrix, m_pOwnerTransform->Get_WorldMatrix());

	Data.vMotionLifeTime = _float2(0.f, m_fMotionLifeTime);
	Data.vColor = m_vColor;

	m_Datas.push_back(Data);

	m_iNumTrails = m_Datas.size();
}

void CMotionTrail::Update_Datas(_float fTimeDelta)
{
	for (auto& Trail : m_Datas)
		Trail.vMotionLifeTime.x += fTimeDelta;

	while (m_Datas.front().vMotionLifeTime.x >= m_Datas.front().vMotionLifeTime.y)
	{
		m_Datas.pop_front();

		if (m_Datas.empty())
			break;
	}

	m_iNumTrails = m_Datas.size();
}

HRESULT CMotionTrail::Bind_BoneMatrices(_uint iTrailIndex, _uint iMeshIndex)
{
	if (FAILED(m_pShader->Bind_Matrices("g_BoneMatrices", m_Datas[iTrailIndex].BoneMatrices[iMeshIndex].data(), m_iNumBones[iMeshIndex])))
		CRASH("Failed to Bind MotionTrail Bone Matrices");

	return S_OK;
}

CMotionTrail* CMotionTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMotionTrail* pInstance = new CMotionTrail(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMotionTrail");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMotionTrail::Clone(void* pArg)
{
	CMotionTrail* pInstance = new CMotionTrail(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMotionTrail");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMotionTrail::Free()
{
	__super::Free();

	Safe_Release(m_pShader);
	Safe_Release(m_pOwnerModel);
	Safe_Release(m_pOwnerTransform);
}
