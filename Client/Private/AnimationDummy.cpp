#include "ClientPch.h"
#include "AnimationDummy.h"
#include "Model.h"

CAnimationDummy::CAnimationDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CAnimationDummy::CAnimationDummy(const CAnimationDummy& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CAnimationDummy::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAnimationDummy::Initialize_Clone(void* pArg)
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

    m_strModelDatPath = pDesc->strModelDatPath;


    if (FAILED(Ready_Components(pDesc)))
    {
        CRASH("Failed Ready_Components");
        return E_FAIL;
    }

#ifdef _DEBUG
    m_strCurrentAnimation = m_pModelCom->Get_AnimationNames()[0];
#endif // _DEBUG

    m_IsPlayAnimation = true;
	m_strCurrentAnimation = "Pose";
	ANIMATION_PLAY_DESC playDesc{};
	playDesc.strAnimationName = m_strCurrentAnimation;
	playDesc.fTimeDelta = 0.f;
	playDesc.pTrackPosition = &m_fTrackPosition;
	playDesc.isFacial = false;

	ROOTMOTION_DESC rootMotionDesc{};
	rootMotionDesc.fRate = 1.f;
	rootMotionDesc.isEnable = true;
	rootMotionDesc.isRotate = true;
	rootMotionDesc.isTranslate = true;
    m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, playDesc, rootMotionDesc);

    // Look 벡터 설정한 방향으로 잘갑니다 지금.
    

    return S_OK;
}

void CAnimationDummy::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);

    // 0. Transform의 Previous Position을 저장해둔다.
    m_pTransformCom->Save_PreviousPosition();
}

void CAnimationDummy::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);

    m_fTimeDelta = fTimeDelta;
  
    m_strPreAnimation = m_strCurrentAnimation;

    // 1. 무조건 처음 해줘야하는거
    if (m_IsPlayAnimation)
    {
		ANIMATION_PLAY_DESC playDesc{};
		playDesc.strAnimationName = m_strCurrentAnimation;
		playDesc.fTimeDelta = 0.f;
		playDesc.pTrackPosition = &m_fTrackPosition;
		playDesc.isFacial = false;

		ROOTMOTION_DESC rootMotionDesc{};
		rootMotionDesc.fRate = 1.f;
		rootMotionDesc.isEnable = true;
		rootMotionDesc.isRotate = true;
		rootMotionDesc.isTranslate = true;

        m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, playDesc, rootMotionDesc);
        m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    }

    // 2. 추가 이동량을 Transform에 적용해 + 중력
    Calc_Translate();
    // 3. 이전 프레임 포지션을 가지고 와서 현재 트랜스폼 위치에서 뺀다음에 그 이동량을
    // Collider Update에 던진다.


    if (m_strPreAnimation != m_strCurrentAnimation)
        m_fTrackPosition = 0.f;
        
}

void CAnimationDummy::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CAnimationDummy::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

        m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0);

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
    
}

void CAnimationDummy::Render_Shadow()
{

}

#ifdef _DEBUG
const vector<_string>& CAnimationDummy::Get_AnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_AnimationNames();
}

_float* CAnimationDummy::Get_TrackPositionPtr(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_TrackPositionPtr(strAnimName);
}

_float CAnimationDummy::Get_Duration(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(strAnimName);
}

const _string& CAnimationDummy::Get_CurrentAnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_strCurrentAnimation;
}

const _float CAnimationDummy::Get_CurrentAnimationDuration() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(m_strCurrentAnimation);
}



void CAnimationDummy::Set_TrackPosition(_float fTrackPosition)
{
    ASSERT_CRASH(m_pModelCom);
    m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, fTrackPosition);

    /*if (!m_IsPlayAnimation)
        m_pModelCom->Play_Animation(m_strCurrentAnimation, m_fTimeDelta, &m_fTrackPosition, false);*/

    if (!m_IsPlayAnimation)
    {
        // 3. m_fTrackPosition을 방금 설정한 값으로 업데이트합니다.
        //    (Play_Animation_GPU가 이 값을 참조하기 때문)
        m_fTrackPosition = fTrackPosition;

        // 4. fTimeDelta = 0.f로 GPU 업데이트를 1회 실행합니다.
        //    (기존 주석 코드를 GPU 버전으로 변경)
		ANIMATION_PLAY_DESC playDesc{};
		playDesc.strAnimationName = m_strCurrentAnimation;
		playDesc.fTimeDelta = 0.f;
		playDesc.pTrackPosition = &m_fTrackPosition;
		playDesc.isFacial = false;

		ROOTMOTION_DESC rootMotionDesc{};
		rootMotionDesc.fRate = 1.f;
		rootMotionDesc.isEnable = true;
		rootMotionDesc.isRotate = true;
		rootMotionDesc.isTranslate = true;

		m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, playDesc, rootMotionDesc);

       // 5. 루트 모션도 멈춘 위치에서 동기화합니다.
       // m_pModelCom->Sync_RootNode(m_pTransformCom, 0.f);
    }

}
void CAnimationDummy::Set_PlayAnimation(_bool IsPlay)
{
    m_IsPlayAnimation = IsPlay;
}

void CAnimationDummy::Register_AllNotifies(const _string& strFolderPath)
{
    //m_pModelCom->Register_Notify(strFilePath);

    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active); // 'this->'?? ???? ????
    };

    auto effectCallBack = [this](const _wstring& tag) {
        this->Effect_Active();
    };
	auto objectCallBack = [this](const _wstring& tag) {
		};
    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack, objectCallBack);
    
}
void CAnimationDummy::Collider_Active(const _wstring&, _bool)
{

}
void CAnimationDummy::Effect_Active()
{
}

#endif

void CAnimationDummy::Calc_Translate()
{
    _vector vTranslate = {};
    
    if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_F";
        vTranslate = m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_S) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_B";
        vTranslate = m_pTransformCom->Get_State(STATE::LOOK);
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_A) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_LF";
        vTranslate = m_pTransformCom->Get_State(STATE::RIGHT);
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_D) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_RF";
        vTranslate = m_pTransformCom->Get_State(STATE::RIGHT) * -1.f;
    }


    if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::UP)
        m_strCurrentAnimation = "Move_F";

    if (m_pGameInstance->Get_DIKeyState(DIK_SPACE) == KEYSTATE::PRESS)
        m_strCurrentAnimation = "Jump_Walk_LF";


    m_pTransformCom->Go_Force(XMVector3Normalize(vTranslate), m_fTimeDelta * 100.f);

    
}

void CAnimationDummy::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

HRESULT CAnimationDummy::Ready_Components(const ANIMATION_ACTOR_DESC* pDesc)
{
    // Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), pDesc->strShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), pDesc->strComputeShaderTag,
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

CGameObject* CAnimationDummy::Clone(void* pArg)
{
    CAnimationDummy* pInstance = new CAnimationDummy(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAnimationDummy");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CAnimationDummy* CAnimationDummy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimationDummy* pInstance = new CAnimationDummy(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAnimationDummy");
        Safe_Release(pInstance);
    }
    
    return pInstance;
}

void CAnimationDummy::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);

}
