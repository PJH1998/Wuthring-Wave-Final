#include"ClientPch.h"
#include "MapObject_Meteo.h"
#include"GameSystem.h"

CMapObject_Meteo::CMapObject_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_Meteo::CMapObject_Meteo(const CMapObject_Meteo& Prototype)
	:CGameObject(Prototype), m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Meteo::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Meteo::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Components(pArg);
	m_pGameSystem->TriggerRegister(m_iTriggerIndex,[this](void* pArg) {
		m_IsTriggerd = true;
	});
    return S_OK;
}

void CMapObject_Meteo::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Meteo::Update(_float fTimeDelta)
{
	if (m_IsTriggerd)
		LerpPos(fTimeDelta);
}

void CMapObject_Meteo::Late_Update(_float fTimeDelta)
{
	if (m_IsTriggerd)
		//if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this)))
	
	//if (m_IsTriggerd)
	//	if (FAILED(m_pGameInstance->Add_Render_StaticObject(this, 0)))

			return;
}

void CMapObject_Meteo::Render()
{
	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CMapObject_Meteo::LerpPos(_float fTimeDelta)
{
	PREFAB_INFO Info{};
	Info.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	if (m_iEffectFrame >= 3)
	{
		m_pGameInstance->Spawn_PoolingObject(TEXT("Smoke"), m_pTransformCom->Get_WorldMatrix(), &Info);
		m_iEffectFrame = 0;
	}
	m_iEffectFrame++;
	m_fFall += fTimeDelta;
	_float Time = m_fFall / m_fDuration;
	_vector current_xz = XMVectorLerp(XMLoadFloat4(&m_vSourPos), XMLoadFloat4(&m_vDestPos), Time);

	_float y_arc = sin(Time * XM_PI) * m_farchY + current_xz.m128_f32[1];
	_vector CurrentPos = XMVectorSetY(current_xz, y_arc);

	m_pTransformCom->Set_State(STATE::POSITION, CurrentPos);
	if (Time >= 1.f)
	{
		m_pGameInstance->Spawn_PoolingObject(TEXT("Explosion"), m_pTransformCom->Get_WorldMatrix(), &Info);
		//이펙트들 터트리기.
		m_IsTriggerd = false;
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&m_vSourPos));
		m_fFall = 0.f;
		m_isActivate = false;

		if (m_iTriggerActiveIndex != -1)
		{
			m_pGameSystem->OnTriggerActivate(m_iTriggerActiveIndex);

			if (m_pTempPtr)
				m_pGameSystem->Toggle_GrapplePoint(m_pTempPtr, true);

			if (m_pSecondTempPtr)
				m_pGameSystem->Toggle_GrapplePoint(m_pSecondTempPtr, true);
		}
	}
}

void CMapObject_Meteo::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));

	m_farchY = pDesc->fArchY;
	m_fDuration = pDesc->fDuration;
	m_vDestPos = pDesc->vDestPos;
	m_vSourPos = pDesc->vSourPos;
	m_iTriggerIndex = pDesc->TriggerIndex;
	m_iTriggerActiveIndex = pDesc->TriggerActiveIndex;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	_wstring WModelName = Model;
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), WModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return;

	switch ((m_iTriggerActiveIndex))
	{
	case 11:
		m_pTempPtr = m_pGameSystem->Create_GrapplePoint(_float3(3458.7f, 334.9, 1787.2f), UI_GRAPPLE_TYPE::ANCHOR);
		m_pSecondTempPtr = m_pGameSystem->Create_GrapplePoint(_float3(3456.3f, 341.5f, 1769.5f), UI_GRAPPLE_TYPE::ANCHOR);
		m_pGameSystem->Toggle_GrapplePoint(m_pTempPtr, false);
		m_pGameSystem->Toggle_GrapplePoint(m_pSecondTempPtr, false);
		break;
	}
}

CMapObject_Meteo* CMapObject_Meteo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Meteo* pInstance = new CMapObject_Meteo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Meteo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Meteo::Clone(void* pArg)
{
	CMapObject_Meteo* pInstance = new CMapObject_Meteo(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Meteo (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_Meteo::Free()
{
	__super::Free();
	m_pTempPtr = nullptr;
	m_pSecondTempPtr = nullptr;
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pModelCom);

}
