#include "EditorPch.h"
#include "Animator_UI.h"

CAnimator_UI::CAnimator_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
}

CAnimator_UI::CAnimator_UI(const CAnimator_UI& Prototype)
    : CComponent(Prototype)
{
}

HRESULT CAnimator_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimator_UI::Initialize_Clone(void* pArg)
{
    ASSERT_CRASH(pArg);

    ANIMATOR_UI_DESC* pDesc = static_cast<ANIMATOR_UI_DESC*>(pArg);
    m_pOwner = pDesc->pOwner;



	return S_OK;
}

void CAnimator_UI::Priority_Update(_float fTimeDelta)
{

}

void CAnimator_UI::Update(_float fTimeDelta)
{
    // 업데이트는 어떻게 돌림? 게임인스턴스에서 돌 매니저도 아니고
    // UI가 업데이트를 돌게 하고, UI Update에서 애니메이터를 Update 돌려야 하나

    Update_Animation(fTimeDelta);


    // m_pCurAnimDesc 갱신도 있어야 함

    m_fElapsedTime += fTimeDelta;
}

void CAnimator_UI::Late_Update(_float fTimeDelta)
{

}

HRESULT CAnimator_UI::Render()
{
	return S_OK;
}

HRESULT CAnimator_UI::Insert_Animation(CLevel_UI::UI_ANIM_DESC& Desc)
{
    if (Find_Animation(Desc.strAnimName))
        return E_FAIL;

    m_vecAnimationDescs.push_back(Desc);
    return S_OK;
}

HRESULT CAnimator_UI::Remove_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = nullptr;

    _uint iIndex = 0;
    for (auto& animDesc : m_vecAnimationDescs)
    {
        if (animDesc.strAnimName == strAnimName)
        {
            pDesc = &animDesc;
            break;
        }
        iIndex++;
    }

    if (m_pCurAnimDesc == pDesc)
        m_pCurAnimDesc = nullptr;

    if (!pDesc)
        return E_FAIL;

    m_vecAnimationDescs.erase(m_vecAnimationDescs.begin() + iIndex);
    return S_OK;
}

HRESULT CAnimator_UI::Clear_Animation()
{
    m_pCurAnimDesc = nullptr;
    m_vecAnimationDescs.clear();

    return S_OK;
}

HRESULT CAnimator_UI::Change_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = Find_Animation(strAnimName);

    if (!pDesc)
        return E_FAIL;
    
    m_pCurAnimDesc = pDesc;
    m_fElapsedTime = 0;

    return S_OK;
}

HRESULT CAnimator_UI::Change_Animation(_uint iAnimIndex)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = Find_Animation(iAnimIndex);

    if (!pDesc)
        return E_FAIL;

    m_pCurAnimDesc = pDesc;
    m_fElapsedTime = 0;

    return S_OK;
}

HRESULT CAnimator_UI::Deselect_Animation()
{
    m_pCurAnimDesc = nullptr;

    return S_OK;
}

CLevel_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = nullptr;

    for (auto& animDesc : m_vecAnimationDescs)
        if (animDesc.strAnimName == strAnimName)
        {
            pDesc = &animDesc;
            break;
        }
    
    return (pDesc) ? pDesc : nullptr;
}

CLevel_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_uint iAnimIndex)
{
    if (iAnimIndex >= m_vecAnimationDescs.size())
        return nullptr;

    return &m_vecAnimationDescs[iAnimIndex];
}

// Fix_LerpRatio, Calc_Lerp 두개 합쳐서
// 여러 점을 기준으로 위치가 부드럽게 보간되는 것도 고려..? 근데 의미가 있나
_float CAnimator_UI::Fix_LerpRatio(_float fIn, _uint iLerpType)
{
    switch (static_cast<UI_LERPTYPE>(iLerpType))
    {
    case UI_LERPTYPE::LINEAR:       return fIn;
    case UI_LERPTYPE::CUBIC:        return fIn * fIn * (3.0f - 2.0f * fIn);
    default:        break;
    }
}

_float CAnimator_UI::Calc_Lerp(_float fStart, _float fEnd, _float fRatio)
{
    if (fRatio > 1.f || fRatio < 0.f)
        CRASH();

    return (fStart * (1 - fRatio)) + (fEnd * fRatio);
}

void CAnimator_UI::Update_Animation(_float fTimeDelta)
{
    // 도출된 키프레임을 통해 현재 값이어야 하는 것 걸러내고 실제로 적용시켜야 함
    // 텍스쳐는 이전 값 그대로 사용해야 하고, Alpha, Transform과 같은 값은 LerpType 에 따른 보간을 이용해서 적용해야 함
    // LerpType 은 Linear, Cubic

    // 현재 방식은 단순하게 점을 따라 일직선상으로 이동하기만 하는데,
    // 필요에 따라서는 점 여러개를 인자로써 통하는 함수를 이용하여 자연스러운 곡선 움직임이 필요할수도?


    if (m_pCurAnimDesc == nullptr)
        return;


    // Calculate Frame..
    const _uint iKeyFrameRate = 60; // 기준 초당 프레임
    const _float fSingleFrameTime = 1.f / iKeyFrameRate;

    _float fCurFrame = m_fElapsedTime / fSingleFrameTime;                   // 현재 키프레임
    if (fCurFrame >= m_pCurAnimDesc->vecKeyFrames.back().iKeyframeIndex)
    {
        if (m_pCurAnimDesc->isLoop)
            m_fElapsedTime = 0.f;                                           // 루프 시, 범위 넘어가면 0으로
    }

    fCurFrame = m_fElapsedTime / fSingleFrameTime;                          // 최종 현재 키프레임
    _uint iCurFrame = static_cast<_uint>(fCurFrame);                        // 최종 현재 키프레임 (int로 내림)


    // ==============================
    // * Calculate Ratio..
    // ==============================
    _uint iFrame_LerpStart = {};        // 프레임 값
    _uint iFrame_LerpEnd = {};          // 프레임 값
    _uint iFrame_StartIndex = {};       // 순수 인덱스
    _uint iFrame_EndIndex = {};         // 순수 인덱스

    for (_uint i = 0; i < m_pCurAnimDesc->vecKeyFrames.size(); i++)
    {
        if (m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex <= iCurFrame)
        {
            iFrame_LerpStart = m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex;
            iFrame_StartIndex = i;

            if (m_pCurAnimDesc->vecKeyFrames.size() > (i + 1))
            {
                iFrame_LerpEnd = m_pCurAnimDesc->vecKeyFrames[i + 1].iKeyframeIndex;
                iFrame_EndIndex = i + 1;
            }
            else
            {
                iFrame_LerpEnd = m_pCurAnimDesc->vecKeyFrames[0].iKeyframeIndex;
                iFrame_EndIndex = 0;
            }
        }
        else
            break;
    }

    _float fRawLerpRatio = static_cast<_float>((fCurFrame - iFrame_LerpStart) / (iFrame_LerpEnd - iFrame_LerpStart));
    _float fFixedLerpRatio = Fix_LerpRatio(fRawLerpRatio, m_pCurAnimDesc->iLerpType); // 이전 키프레임와 현재 키프레임 간의 최종 보간 비율


    // ==============================
    // * Calculate Results..
    // ==============================
    _uint iResultTexIndex = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iTexIndex;
    _float fResultAlpha = Calc_Lerp(
        m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].fAlpha, 
        m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].fAlpha, 
        fFixedLerpRatio
    );
    _float3 vResultPos = {};
    XMStoreFloat3(&vResultPos, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vPos),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vPos),
        fFixedLerpRatio)
    );
    _float3 vResultRot = {};
    XMStoreFloat3(&vResultRot, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vRot),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vRot),
        fFixedLerpRatio)
    );
    _float3 vResultSca = {};
    XMStoreFloat3(&vResultSca, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vSca),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vSca),
        fFixedLerpRatio)
    );

    // ==============================
    // * Apply Results..
    // ==============================
    m_pOwner->Set_CurTexIndex(iResultTexIndex);
    CTransform* pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(L"Com_Transform"));
    
    _matrix matPos = XMMatrixTranslationFromVector(XMLoadFloat3(&vResultPos));
    _matrix matRot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&vResultRot));
    _matrix matSca = XMMatrixScalingFromVector(XMLoadFloat3(&vResultSca));

    _matrix matTransform = matSca * matRot * matPos;

    pOwnerTransformCom->Set_WorldMatrix(matTransform);
}

CAnimator_UI* CAnimator_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimator_UI* pInstance = new CAnimator_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimator_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CAnimator_UI::Clone(void* pArg)
{
    CAnimator_UI* pInstance = new CAnimator_UI(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CAnimator_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimator_UI::Free()
{
    __super::Free();
}
