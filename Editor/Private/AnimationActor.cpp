#include "EditorPch.h"
#include "AnimationActor.h"
#include "Model.h"

#include "SpringCamera_Edit.h"

CAnimationActor::CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CAnimationActor::CAnimationActor(const CAnimationActor& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CAnimationActor::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAnimationActor::Initialize_Clone(void* pArg)
{
    ANIMATION_ACTOR_DESC* pDesc = static_cast<ANIMATION_ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eLevel;

    m_iShaderPath = pDesc->iShaderPath;

    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPostion), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
    
    _float3 vRadian = { 
        XMConvertToRadians(pDesc->vRotation.x),
        XMConvertToRadians(pDesc->vRotation.y),
        XMConvertToRadians(pDesc->vRotation.z) };
    m_pTransformCom->Rotation_Quaternion(vRadian);

    // Model의 Dat Folder Path
    m_strModelDatPath = pDesc->strModelDatPath;


    if (FAILED(Ready_Components(pDesc)))
    {
        CRASH("Failed Ready_Components");
        return E_FAIL;
    }

    // Default는 0번 애니메이션 실행.
#ifdef _DEBUG
    m_strCurrentAnimation = m_pModelCom->Get_AnimationNames()[0];
#endif // _DEBUG

    

    m_IsPlayAnimation = true;
	m_strCurrentAnimation = "Blend_BasePose";
    //m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, "Blend_BasePose", 0.f, &m_fTrackPosition, true, 0.01f);
    m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, "Blend_BasePose", 0.f, &m_fTrackPosition, true, 0.01f);

#ifdef _DEBUG
	XMStoreFloat4(&m_vInitPosition, m_pTransformCom->Get_State(STATE::POSITION));
#endif // _DEBUG

    

    //m_pTransformCom->Scale(pDesc->vScale);
    // Look 벡터 설정한 방향으로 잘갑니다 지금.
	if (FAILED(Ready_Camera()))
		CRASH("Camera");
	m_fOffsetY = 1.f;

    return S_OK;
}

void CAnimationActor::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);
}

void CAnimationActor::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);

    m_fTimeDelta = fTimeDelta;
   
    //if (m_IsPlayAnimation)
    //{
    //    m_pModelCom->Play_Animation_CPU(m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, false, true, 0.01f);
    //    _string strRibAnimation = "Rib_" + m_strCurrentAnimation;
    //    //m_pModelCom->Play_RibAnimation(strRibAnimation, fTimeDelta);
    //    m_pModelCom->Play_RibAnimation(strRibAnimation, m_fTrackPosition);
    //    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    //}
        

    /*if (m_IsPlayAnimation)
        m_pModelCom->Play_RibAnimation_GPU(strRibAnimation, fTimeDelta);*/


    _bool IsAnimationEnd = { false };
    if (m_IsPlayAnimation)
    {
        IsAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, true, true, true, 1.f);
        //IsAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, true, true, true, 1.f);

        m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    }

#ifdef _DEBUG
    // Jump Second F
    //const _float4x4* RootMatrix = m_pModelCom->Get_BoneMatrixPtr("Root");
    //const _float4x4* HairMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Hair001_M");
    //OutPutDebugMatrix(TEXT("Root"), *RootMatrix);
    //OutPutDebugMatrix(TEXT("Bone_Hair001_M"), *HairMatrix);
#endif // _DEBUG


    //if (IsAnimationEnd)
    //    m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&m_vInitPosition));
  

    //m_pModelCom->Sync_RootNode(m_pTransformCom, 0.f);
        
#ifdef _DEBUG
	_int iBoneIndex = 0;
    m_pModelCom->Bind_Bone_to_GUI(iBoneIndex, m_pTransformCom->Get_WorldMatrix());

    m_pModelCom->Render_Gizmo(m_pTransformCom->Get_WorldMatrix());
#endif // _DEBUG
	ImGui::Begin("Offset");
	ImGui::Text("OffsetY : ");
	ImGui::SameLine();
	ImGui::InputFloat("##", &m_fOffsetY);
	ImGui::End();

	m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), m_fOffsetY);
}

void CAnimationActor::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CAnimationActor::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

        //if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
        //    return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
    
}

void CAnimationActor::Render_Shadow()
{

}

#ifdef _DEBUG
const vector<_string>& CAnimationActor::Get_AnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_AnimationNames();
}

_float* CAnimationActor::Get_TrackPositionPtr(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_TrackPositionPtr(strAnimName);
}

_float CAnimationActor::Get_Duration(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(strAnimName);
}

// Notify에서 사용할 현재 선택된 애니메이션 이름
const _string& CAnimationActor::Get_CurrentAnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_strCurrentAnimation;
}

// Notify에서 사용할 현재 선택된 애니메이션의 최대 TrackPosition
const _float CAnimationActor::Get_CurrentAnimationDuration() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(m_strCurrentAnimation);
}

HRESULT CAnimationActor::Bind_Bone_to_GUI()
{
	_int iBoneIndex = 0;
    if(FAILED(m_pModelCom->Bind_Bone_to_GUI(iBoneIndex, m_pTransformCom->Get_WorldMatrix())))
        return E_FAIL;
    return S_OK;
}


// Notify에서 사용할 현재 선택된 애니메이션의 최대 프레임 정보?

void CAnimationActor::Set_TrackPosition(_float fTrackPosition)
{
    ASSERT_CRASH(m_pModelCom);
    // 어차피 현재거 설정하니까 매개변수로 가져올 필요가 없을 듯.
    m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, fTrackPosition);

    // TrackPosition을 설정하면서 만약 Stop인 경우에도 확인할 수 있게 Play Animation을 실행합니다.
   /* if (!m_IsPlayAnimation)
        m_pModelCom->Play_Animation(m_strCurrentAnimation, m_fTimeDelta, &m_fTrackPosition, false);*/

    if (!m_IsPlayAnimation)
    {
        // 3. m_fTrackPosition을 방금 설정한 값으로 업데이트합니다.
        //    (Play_Animation_GPU가 이 값을 참조하기 때문)
        m_fTrackPosition = fTrackPosition;

        // 4. fTimeDelta = 0.f로 GPU 업데이트를 1회 실행합니다.
        //    (기존 주석 코드를 GPU 버전으로 변경)
        m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom,
            m_strCurrentAnimation,
            0.f, // TimeDelta를 0으로 주어 시간이 흐르지 않게 함
            &m_fTrackPosition,
            true, 0.1f);

       // 5. 루트 모션도 멈춘 위치에서 동기화합니다.
       // m_pModelCom->Sync_RootNode(m_pTransformCom, 0.f);
    }

}
void CAnimationActor::Set_PlayAnimation(_bool IsPlay)
{
    m_IsPlayAnimation = IsPlay;
}

// 폴더에 존재하는 모든 애니메이션 json을 읽어와서 등록합니다.
void CAnimationActor::Register_AllNotifies(const _string& strFolderPath)
{
    //m_pModelCom->Register_Notify(strFilePath);

    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active); // 'this->'는 생략 가능
    };

    auto effectCallBack = [this](const _wstring& tag) {
        this->Effect_Active(tag);
    };

    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
    
}
void CAnimationActor::Collider_Active(const _wstring&, _bool)
{

}
void CAnimationActor::Effect_Active(const _wstring& tag)
{
    _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
    m_pGameInstance->Spawn_PoolingObject(tag, matWorld, m_pModelCom);
}

const _float4x4* CAnimationActor::Get_BoneMatrix(const _string& strBoneName)
{
    if (nullptr == m_pModelCom)
    {
        MSG_BOX("Model nullptr");
        return nullptr;
    }
        
    const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());

    if (nullptr == pBoneMatrix)
    {
        MSG_BOX("Bone Name Error");
        return nullptr;
    }

    return pBoneMatrix;
}
const _float4x4* CAnimationActor::Get_WorldMatrixPtr()
{
    return m_pTransformCom->Get_WorldMatrixPtr();
}

#endif

// 1. 행렬 
void CAnimationActor::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

HRESULT CAnimationActor::Ready_Components(const ANIMATION_ACTOR_DESC* pDesc)
{
    // Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strComputeShaderTag,
        TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    // Model
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
    {
        CRASH("Failed Ready Com_Model");
        return E_FAIL;
    }
        

    return S_OK;
}

HRESULT CAnimationActor::Ready_Camera()
{
	m_pSpringCamera = CSpringCamera_Edit::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSpringCamera);

	CSpringCamera_Edit::CAMERA_DESC CameraDesc = {};
	CameraDesc.fSpeedPerSec = 100.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 5000.f;
	CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
	CameraDesc.fMouseSensor = 0.004f;

	m_pSpringCamera->Initialize_Clone(&CameraDesc);
	
	m_pGameInstance->Add_Camera(ENUM_CLASS(m_eCurLevel), TEXT("Camera_Spring"), m_pSpringCamera);
	Safe_AddRef(m_pSpringCamera);

	m_pGameInstance->Change_MainCamera(ENUM_CLASS(m_eCurLevel), TEXT("Camera_Spring"));

	return S_OK;
}

CGameObject* CAnimationActor::Clone(void* pArg)
{
    CAnimationActor* pInstance = new CAnimationActor(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CAnimationActor* CAnimationActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimationActor* pInstance = new CAnimationActor(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationActor::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);

	Safe_Release(m_pSpringCamera);

}
