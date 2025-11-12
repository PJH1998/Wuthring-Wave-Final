#include "Editorpch.h"
#include "Effect_Prefab.h"
#include "Particle.h"
#include "Effect_Mesh.h"
#include "Trail_Mesh.h"
#include "Effect_Rect.h"
#include "Effect_Decal.h"

CEffect_Prefab::CEffect_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Prefab::CEffect_Prefab(const CEffect_Prefab& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Prefab::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Prefab::Initialize_Clone(void* pArg)
{
    PREFAB_DESC* pDesc = static_cast<PREFAB_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_strMyTag = pDesc->strPrefabTag;

    //m_vLifeTime = pDesc->vLifeTime;
    //일단 프리팹 라이프타임 15초로
    m_vLifeTime.y = 10.f;
    m_vLifeTime.x = 0.f;

    m_isActivate = false;

    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());

    return S_OK;
}

void CEffect_Prefab::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_fCurrentTime += fTimeDelta;



	for (auto& Frame : m_vFrames)
	{
		if (Frame.fActivateTime <= m_fCurrentTime && !Frame.bActivated)
		{
			_bool IsActivated = true;
			//자식 활성화하기전에 오프셋 처리
			_matrix OffsetMatrix = {};

			if (!Frame.IsRoot)
			{
				Children_Offset(Frame, OffsetMatrix);
				Get_Children(Frame.strChildrenTag)->Reset(OffsetMatrix, &IsActivated);
			}
			else
			{
				//

				//
			}

			Frame.bActivated = true;
		}
	}


    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Priority_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if (m_vLifeTime.x >= m_vLifeTime.y)
    {
        m_isActivate = false;
        Reset_Prefab_Info();
    }
    else
        m_vLifeTime.x += fTimeDelta;

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Update(fTimeDelta);
    }
}

void CEffect_Prefab::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Late_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Render()
{

}

void CEffect_Prefab::Add_Children(void* pArg, EFFECT_TYPE eType)
{
    CGameObject* pChildren = {};
    _wstring strChildrenTag = {};
    
    //설정할 자식들 Desc 미리 선언
    CParticle::PARTICLE_DESC* pParticleDesc = {};
    CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc = {};
    CTrail_Mesh::TRAILMESH_DESC* pTrailDesc = {};
	CEffect_Rect::FXRECT_DESC* pRectDesc = {};
	CEffect_Decal::DECAL_DESC* pDecalDesc = {};

    //프리팹 프레임에 미리 추가.
    FRAME_DESC FrameDesc = {};

    switch (eType)
    {
    case EFFECT_TYPE::PARTICLE:
        pParticleDesc = static_cast<CParticle::PARTICLE_DESC*>(pArg);
        strChildrenTag = pParticleDesc->strMyTag;

        FrameDesc.strChildrenTag = pParticleDesc->strMyTag;
        FrameDesc.eChildrenType = eType;
        
        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::MESH:
        pMeshDesc = static_cast<CEffect_Mesh::EFFECTMESH_DESC*>(pArg);
        strChildrenTag = pMeshDesc->strMyTag;

        FrameDesc.strChildrenTag = pMeshDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::TRAIL:
        pTrailDesc = static_cast<CTrail_Mesh::TRAILMESH_DESC*>(pArg);
        strChildrenTag = pTrailDesc->strMyTag;

        FrameDesc.strChildrenTag = pTrailDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_TrailMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
	case EFFECT_TYPE::RECT:
		pRectDesc = static_cast<CEffect_Rect::FXRECT_DESC*>(pArg);
		strChildrenTag = pRectDesc->strMyTag;

		FrameDesc.strChildrenTag = pRectDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectRect"), PROTOTYPE::GAMEOBJECT, pArg));
		break;
	case EFFECT_TYPE::DECAL:
		pDecalDesc = static_cast<CEffect_Decal::DECAL_DESC*>(pArg);
		strChildrenTag = pDecalDesc->strMyTag;

		FrameDesc.strChildrenTag = pDecalDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectDecal"), PROTOTYPE::GAMEOBJECT, pArg));
		break;

    case EFFECT_TYPE::END:
        CRASH("Failed Children Desc");
        break;
    }

    if (pChildren == nullptr)
        return;

    //활성화 한번
    //Reset_Prefab_Info();
 /*   _matrix DefaultMat = {};
    pChildren->Reset(DefaultMat, nullptr);*/

    m_EffectChildren.emplace(strChildrenTag, pChildren);

    _bool bCheck = false;
    FrameDesc_Check(strChildrenTag, &bCheck);

    if (!bCheck)
        m_vFrames.push_back(FrameDesc);

}

void CEffect_Prefab::Remove_Children(_wstring& ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return;

    Safe_Release(iter->second);
    m_EffectChildren.erase(iter);
}

_int CEffect_Prefab::Get_Children_Count()
{
    if (m_EffectChildren.empty())
        return 0;

    return m_EffectChildren.size();
}

_wstring CEffect_Prefab::Get_Children_Tag(_int iIndex)
{
    _int iCheckIndex = 0;

    for (auto iter = m_EffectChildren.begin(); iter != m_EffectChildren.end();)
    {
        if (iCheckIndex == iIndex)
        {
            return iter->first;
        } 
        else
        {
            ++iter;
            ++iCheckIndex;
        }
    }
}

CGameObject* CEffect_Prefab::Get_Children(_wstring ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return nullptr;

    return iter->second;
}

void CEffect_Prefab::Bind_FrameDesc(PREFAB_DESC& PrefabDesc)
{
    for (size_t i = 0; i < PrefabDesc.ChildrenCount; i++)
    {
        FRAME_DESC FrameDesc = PrefabDesc.FrameDesc[i];

        m_vFrames.push_back(FrameDesc);
    }
}

void CEffect_Prefab::Set_FrameDesc(FRAME_DESC* pFrameDesc)
{
    for (auto& Frame : m_vFrames)
    {
        if (pFrameDesc->strChildrenTag == Frame.strChildrenTag)
        {
            Frame = *pFrameDesc;
			_bool IsActivated = true;
			_matrix OffsetMatrix = {};
			Children_Offset(Frame, OffsetMatrix);
			Get_Children(Frame.strChildrenTag)->Reset(OffsetMatrix, &IsActivated);

			m_vLifeTime.x = 0.f;
            return;
        }
    }
}


void CEffect_Prefab::Set_SpawnMatrix(_float4x4 SpawnMatrix)
{
    m_SpawnMatrix = SpawnMatrix;
}

void CEffect_Prefab::Reset_SpawnMatrix()
{
    for (auto& Children : m_EffectChildren)
        Children.second->SetActivate(false);

    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());
}

void CEffect_Prefab::Reset_Prefab_Info()
{
    for (auto& Frame : m_vFrames)
    {
        Frame.bActivated = false;
    }
    m_fCurrentTime = 0.f;

    m_vLifeTime.x = 0.f;


#ifdef _DEBUG
    OutPutDebugMatrix(TEXT("Spawn Matrix : "), m_SpawnMatrix);
#endif // DEBUG

    _matrix DefaultMat = XMLoadFloat4x4(&m_SpawnMatrix);

    //초기설정으로 되돌리기 처리만
    _bool Activate = false;
    for (auto& Children : m_EffectChildren)
        Children.second->Reset(DefaultMat, &Activate);
}

void CEffect_Prefab::Children_Offset(const FRAME_DESC& Desc, _matrix& OutMatrix)
{
    _matrix PositionMat = XMMatrixTranslationFromVector(XMVectorSet(Desc.vOffsetPos.x, Desc.vOffsetPos.y, Desc.vOffsetPos.z, 1.f));

    _matrix ScaleMat = XMMatrixScaling(Desc.vOffsetSize.x, Desc.vOffsetSize.y, Desc.vOffsetSize.z);

    _matrix RotMat = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(Desc.vOffsetRot.x),
        XMConvertToRadians(Desc.vOffsetRot.y),
        XMConvertToRadians(Desc.vOffsetRot.z));

    _matrix OffsetMatrix = ScaleMat * RotMat * PositionMat;

	//m_SpawnMatrix 크기 영향 죽이기
	_vector vScale = {};
	_vector vPos = {};
	_vector vRot = {};
	XMMatrixDecompose(&vScale, &vRot, &vPos, XMLoadFloat4x4(&m_SpawnMatrix));

	_matrix SpawnMatrix = XMMatrixRotationQuaternion(vRot) * XMMatrixTranslationFromVector(vPos);

	OutMatrix = OffsetMatrix * SpawnMatrix;
}

void CEffect_Prefab::FrameDesc_Check(_wstring& ChildrenTag, _bool* bCheck)
{
    //자식 추가했을 때, 이전에 있던 자식 태그와 같은 얘라면 FrameDesc 삭제x 급하게 대충 만듦
    for (auto iterFrame = m_vFrames.begin(); iterFrame != m_vFrames.end();)
    {
        if (iterFrame->strChildrenTag == ChildrenTag)
        {
            *bCheck = true;
            break;
        }
        else
            ++iterFrame;
    }

    if (!bCheck)
    {
        for (auto iterFrameDesc = m_vFrames.begin(); iterFrameDesc != m_vFrames.end(); )
        {
            if (iterFrameDesc->strChildrenTag == ChildrenTag)
            {
                iterFrameDesc = m_vFrames.erase(iterFrameDesc);
                break;
            }
            else
                ++iterFrameDesc;
        }
    }

}

CEffect_Prefab* CEffect_Prefab::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Prefab");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Prefab::Clone(void* pArg)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Prefab::Free()
{
    __super::Free();

    for (auto& Children : m_EffectChildren)
        Safe_Release(Children.second);

    m_EffectChildren.clear();
}
