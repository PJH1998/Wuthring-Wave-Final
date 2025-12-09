#include "ClientPch.h"
#include "CoroProduction.h"
#include "MapObject_Meteo.h"
#include "GameSystem.h"

CCoroProduction::CCoroProduction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CCoroProduction::CCoroProduction(const CCoroProduction& Prototype)
    : CGameObject(Prototype)
	,m_pGameSystem {CGameSystem::GetInstance()}
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CCoroProduction::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCoroProduction::Initialize_Clone(void* pArg)
{
    COROPROD_DESC* pDesc = static_cast<COROPROD_DESC*>(pArg);
    if (FAILED(__super::Initialize_Clone(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components(pDesc)))
    {
        CRASH("Failed Ready_Components");
        return E_FAIL;
    }
	Register_AllNotifies(pDesc->strFolderPath);
    m_IsPlayAnimation = false;
	m_Tracks.emplace("Attack4", make_pair(43.f, 73.f));
	m_Tracks.emplace("Attack12", make_pair(5.f, 80.f));
	m_Tracks.emplace("PatrolToFight", make_pair(10.f, 139.f));
	m_isActivate = false;
	m_pGameSystem->TriggerRegister(31, [this](void* pArg) {
		Action2();
		});
	m_pGameSystem->TriggerRegister(34, [this](void* pArg) {
		Action3();
		});
	m_pGameSystem->TriggerRegister(30, [this](void* pArg) {
		Action1();
		});
	_float temp{};
	m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, "PatrolToFight", 0.f, &temp);
    return S_OK;
}

void CCoroProduction::Priority_Update(_float fTimeDelta)
{
    // 0. Transform의 Previous Position을 저장해둔다.
    m_pTransformCom->Save_PreviousPosition();
}

void CCoroProduction::Update(_float fTimeDelta)
{
	_float fTrackPosition{};
	m_IsPlayAnimation = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, fTimeDelta, &fTrackPosition, m_IsRootMotion, m_IsRootMotionRotate, m_IsRootMotionTranslate, 1.f);
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	if (fTrackPosition > m_Tracks[m_strCurrentAnimation].second)
	{
		m_isActivate = false;
	}

}

void CCoroProduction::Late_Update(_float fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CCoroProduction::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");
		_bool HasNormal{ false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;
		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_ShaderIndices[i])))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");

		m_pShaderCom->UndBind_All_VS_SRV();
    }
    
}

void CCoroProduction::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

void CCoroProduction::Register_AllNotifies(const _string& strFolderPath)
{
    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active);
    };

    auto effectCallBack = [this](const _wstring& tag) {
        this->Effect_Active(tag);
    };
	auto objectCallBack = [this](const _wstring& tag) {
		this->Object_Func(tag);
	};
    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack, objectCallBack);
    
}
void CCoroProduction::Collider_Active(const _wstring&, _bool)
{

}
void CCoroProduction::Effect_Active(const _wstring& wStrEffectTag)
{
}
void CCoroProduction::Object_Func(const _wstring& wStrObjectTag)
{
	if (wStrObjectTag == TEXT("Meteo"))
	{
		m_pGameSystem->OnTriggerActivate(50);
		
	}
	//else if (wStrObjectTag == TEXT("Action40"))
	//{
	//	m_pGameSystem->OnTriggerActivate(40);
	//}
}

void CCoroProduction::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

HRESULT CCoroProduction::Ready_Components(const COROPROD_DESC* pDesc)
{
    // Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first), pDesc->computeShaderData.second,
        TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    // Model
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
    {
        CRASH("Failed Ready Com_Model");
        return E_FAIL;
    }
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));

	// Meteo
	_vector vSrc = XMVectorSet(3449.22f, 323.1f, 1964.74f, 1.f);
	_vector vDst = XMVectorSet(3440.1f, 321.5f, 1966.f, 1.f);
	_uint iCurLevel = m_pGameInstance->Get_CurrentLevel();
	CMapObject_Meteo::MAP_LOAD Meteo{};
	Meteo.fArchY = 0.5f;
	Meteo.iLevel = iCurLevel;
	Meteo.fDuration = 1.f;
	XMStoreFloat4(&Meteo.vSourPos, vSrc);
	XMStoreFloat4(&Meteo.vDestPos, vDst);
	Meteo.iShaderPassIndex = 0;
	Meteo.TriggerIndex = 50;
	Meteo.TriggerActiveIndex = 40;
	strcpy_s(Meteo.ModelName, "SM_Com2_Roc_APD_39AX_LOD0");
	XMStoreFloat4x4(&Meteo.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f));
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iCurLevel, TEXT("Prototype_GameObject_MapObject_Meteo"), iCurLevel, TEXT("Layer_Meteo"), &Meteo)))
		return E_FAIL;


    return S_OK;
}

void CCoroProduction::Action1()
{
	if (m_isActionEnd[0])
		return;
	m_strCurrentAnimation = "PatrolToFight";
	m_isActivate = true;
	_vector vScale = XMVectorSet(1.5f, 1.5f, 1.5f, 0.f);
	_vector vPos = XMVectorSet(3455.f, 321.1f, 1963.7f, 1.f);
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(-90.f), XMConvertToRadians(0.f));
	//m_pTransformCom->Set_State(STATE::POSITION, vPos);
	//m_pTransformCom->Rotation_Quaternion(vQuat);
	m_pTransformCom->Set_WorldMatrix(XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuat, vPos));
	m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, m_Tracks[m_strCurrentAnimation].first);
	m_isActionEnd[0] = true;
}

void CCoroProduction::Action2()
{
	if (m_isActionEnd[1])
		return;
	m_strCurrentAnimation = "Attack4";
	m_isActivate = true;
	_vector vScale = XMVectorSet(1.3f, 1.3f, 1.3f, 0.f);
	_vector vPos = XMVectorSet(3393.292f, 312.548f, 1960.47f, 1.f);
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

	m_pTransformCom->Set_WorldMatrix(XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuat, vPos));
	_float fTrackPos{};
	m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, 0.f, &fTrackPos);
	m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, m_Tracks[m_strCurrentAnimation].first);
	m_IsRootMotion = true;
	m_isActionEnd[1] = true;
}

void CCoroProduction::Action3()
{
	if (m_isActionEnd[2])
		return;
	m_strCurrentAnimation = "Attack12";
	m_isActivate = true;
	_vector vScale = XMVectorSet(2.f, 2.f, 2.f, 0.f);
	_vector vPos = XMVectorSet(3388.8f, 290.f, 2013.5f, 1.f);
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(-90.f), XMConvertToRadians(0.f));
	
	m_pTransformCom->Set_WorldMatrix(XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuat, vPos));
	_float fTrackPos{};
	m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, 0.f, &fTrackPos);
	m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, m_Tracks[m_strCurrentAnimation].first);
	m_IsRootMotion = false;
	m_isActionEnd[2] = true;
}

CGameObject* CCoroProduction::Clone(void* pArg)
{
    CCoroProduction* pInstance = new CCoroProduction(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CCoroProduction");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CCoroProduction* CCoroProduction::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCoroProduction* pInstance = new CCoroProduction(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CCoroProduction");
        Safe_Release(pInstance);
    }
    
    return pInstance;
}

void CCoroProduction::Free()
{
    __super::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
	Safe_Release(m_pGameSystem);

}
