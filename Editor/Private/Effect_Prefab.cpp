#include "Editorpch.h"
#include "Effect_Prefab.h"
#include "GameInstance.h"

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
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    return S_OK;
}

void CEffect_Prefab::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Prefab::Update(_float fTimeDelta)
{
}

void CEffect_Prefab::Late_Update(_float fTimeDelta)
{
}

void CEffect_Prefab::Render()
{
}

void CEffect_Prefab::Add_Child(void* pArg)
{
    //자식들 추가 (파티클, 파티클 Desc필요)

}

CGameObject* CEffect_Prefab::Clone(void* pArg)
{
    return nullptr;
}

void CEffect_Prefab::Free()
{
}
