#include "ClientPch.h"
#include "Effect_Prefab.h"
#include "Particle.h"
//#include "Effect_Mesh.h"
#include "Trail_Mesh.h"

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
    m_strBoneTag = pDesc->strBoneTag;

    for (size_t i = 0; i < pDesc->ChildrenCount; i++)
    {
       _wstring strChildrenTag = pDesc->FrameDesc[i].strChildrenTag;
       EFFECT_TYPE eType = pDesc->FrameDesc[i].eChildrenType;

       Add_Children(strChildrenTag, eType, pDesc->CurrentLevel);

       m_vFrames.push_back(pDesc->FrameDesc[i]);
    }
    //m_vLifeTime = pDesc->vLifeTime;
    //프리팹 라이프 타임 필요할까 ?

    m_vLifeTime.y = 15.f;
    m_vLifeTime.x = 0.f;
	m_IsLoop = pDesc->IsLoop;
	
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
			EFFECT_INFO InfoDesc = {};

			InfoDesc.pBoneMatrixPtr = m_pBoneMatrixPtr;
			InfoDesc.pObjectMatrixPtr = m_pObjectMatrixPtr;
			InfoDesc.IsActive = true;

			_matrix OffsetMatrix = {};

			Children_Offset(Frame, OffsetMatrix, InfoDesc);
			Get_Children(Frame.strChildrenTag)->Reset(OffsetMatrix, &InfoDesc);

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

	if (!m_IsLoop)
	{
		if (m_vLifeTime.x >= m_vLifeTime.y)
		{
			m_isActivate = false;
			Reset_Prefab_Info();
		}
		else
			m_vLifeTime.x += fTimeDelta;
	}

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

void CEffect_Prefab::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	
	PREFAB_INFO* pDesc = static_cast<PREFAB_INFO*>(pArg);

	_float4x4 PlayerMatrix = {};
	_float4x4 BoneMatrix = {};

	if (pDesc->pModelPtr != nullptr)
	{
		//프리팹 안에 뼈에 붙어야 할 자식과 안붙어야 할 자식이 같이 있을 수 있음.
		//그러니 프리팹에 기존 처리 + 만약 뼈에 붙어야할 얘가 있다면 추가적인 정보를 필요로 함 (BonePtr과 ObjectPtr필요)
		//기존 처리 할 얘들을 위한 정보 + PTR이 필요하니 기존 처리도 해주고 뼈 정보도 저장해주는게 맞는거 같음. 일단은 그렇게 생각중.
		
		Reset_SpawnMatrix();
		Reset_Prefab_Info();

		XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

		if (m_strBoneTag == "")
			XMStoreFloat4x4(&BoneMatrix, XMMatrixIdentity());
		else
			BoneMatrix = *pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());

		//위에서 꺼낸 본 매트릭스 그때 위치 갱신정보와 모델의 월드매트릭스 전달.
		Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

		m_pBoneMatrixPtr = pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());
		m_pObjectMatrixPtr = pDesc->pMatrixPtr;

		m_isActivate = true;
	}
	else if (pDesc->pModelPtr == nullptr)
	{
		//단순 오브젝트가 호출할경우 모델주소 비어있음.

		Reset_SpawnMatrix();
		Reset_Prefab_Info();

		_float4x4 PlayerMatrix = {};
		XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

		XMStoreFloat4x4(&BoneMatrix, XMMatrixIdentity());

		Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

		m_pObjectMatrixPtr = pDesc->pMatrixPtr;
		m_pBoneMatrixPtr = nullptr;

		m_isActivate = true;
	}
}

void CEffect_Prefab::Add_Children(const _wstring& ChildrenTag, EFFECT_TYPE eType, _uint CurrentLevel)
{
    CGameObject* pChildren = {};
    _wstring strChildrenProtoTag = TEXT("Prototype_GameObject_");
    _wstring strChildrenNameTag = ChildrenTag;

    switch (eType)
    {
    case EFFECT_TYPE::PARTICLE:

        strChildrenProtoTag += TEXT("Particle_");
        strChildrenProtoTag += strChildrenNameTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
        break;

   /* case EFFECT_TYPE::MESH:
        pMeshDesc = static_cast<CEffect_Mesh::EFFECTMESH_DESC*>(pArg);
        strChildrenTag = pMeshDesc->strMyTag;

        FrameDesc.strChildrenTag = pMeshDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

         strDefaultTag += TEXT("FXMesh_");
        strDefaultTag += strChildrenTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;*/

    case EFFECT_TYPE::TRAIL:
        strChildrenProtoTag += TEXT("TrailMesh_");
        strChildrenProtoTag += strChildrenNameTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
        break;

	case EFFECT_TYPE::RECT:
		strChildrenProtoTag += TEXT("FXRect_");
		strChildrenProtoTag += strChildrenNameTag;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
		break;

	case EFFECT_TYPE::DECAL:
		strChildrenProtoTag += TEXT("FXDecal_");
		strChildrenProtoTag += strChildrenNameTag;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
		break;

	case EFFECT_TYPE::RADIAL:
		strChildrenProtoTag += TEXT("FXRadial_");
		strChildrenProtoTag += strChildrenNameTag;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
		break;

    case EFFECT_TYPE::END:
        CRASH("Failed Children Desc");
        break;
    }

    if (pChildren == nullptr)
        return;

    m_EffectChildren.emplace(strChildrenNameTag, pChildren);
}

void CEffect_Prefab::Children_Offset(const FRAME_DESC& Desc, _matrix& OutMatrix, EFFECT_INFO& Info)
{
	_matrix PositionMat = XMMatrixTranslationFromVector(XMVectorSet(Desc.vOffsetPos.x, Desc.vOffsetPos.y, Desc.vOffsetPos.z, 1.f));

	_matrix ScaleMat = XMMatrixScaling(Desc.vOffsetSize.x, Desc.vOffsetSize.y, Desc.vOffsetSize.z);

	_matrix RotMat = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(Desc.vOffsetRot.x),
		XMConvertToRadians(Desc.vOffsetRot.y),
		XMConvertToRadians(Desc.vOffsetRot.z));

	_matrix OffsetMatrix = ScaleMat * RotMat * PositionMat;

	//뼈에 붙어야하는 자식인경우 프리팹이 가지고 있는 오프셋 매트릭스 따로 사용할 수 있게 저장해줘야함.
	Info.OffsetMatrix = OffsetMatrix;		

	//m_SpawnMatrix 크기 영향 죽이기
	_vector vScale = {};
	_vector vPos = {};
	_vector vRot = {};
	XMMatrixDecompose(&vScale, &vRot, &vPos, XMLoadFloat4x4(&m_SpawnMatrix));

	_matrix SpawnMatrix = XMMatrixRotationQuaternion(vRot) * XMMatrixTranslationFromVector(vPos);

	OutMatrix = OffsetMatrix * SpawnMatrix;
}

CGameObject* CEffect_Prefab::Get_Children(_wstring ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return nullptr;

    return iter->second;
}

void CEffect_Prefab::Set_SpawnMatrix(_float4x4 PlayerMatrix, _float4x4 BoneMatrix)
{
    XMStoreFloat4x4(&m_SpawnMatrix,
      XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&PlayerMatrix));
}

void CEffect_Prefab::Reset_SpawnMatrix()
{
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
