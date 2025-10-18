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
    Update_Animation(fTimeDelta);

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

// ?뚯쟾 諛??ㅼ??쇱슜
_float CAnimator_UI::Fix_LerpRatio(_float fIn, _uint iLerpType)
{
    switch (static_cast<UI_LERPTYPE>(iLerpType))
    {
    default:        
    case UI_LERPTYPE::END:
    case UI_LERPTYPE::LINEAR:               return fIn;

    //case UI_LERPTYPE_SPEED::MT:                   return sqrt(1.0f - 4.0f * pow(fIn - 0.5f, 2.0f));
    //case UI_LERPTYPE_SPEED::MB:                   return 1.0f - sqrt(max(0.0f, 1.0f - 4.0f * pow(fIn - 0.5f, 2.0f)));
    case UI_LERPTYPE::LT:                   return sqrt(1.0f - pow(fIn - 1.0f, 2.0f));                          //
    //case UI_LERPTYPE_SPEED::LB:                   return 1.0f - sqrt(1.0f - pow(fIn - 1.0f, 2.0f));;
    //case UI_LERPTYPE_SPEED::RT:                   return sqrt(1.0f - pow(fIn, 2.0f));;
    case UI_LERPTYPE::RB:                   return 1.0f - sqrt(1.0f - pow(fIn, 2.0f));                          //

    case UI_LERPTYPE::CUBIC:                return fIn * fIn * (3.0f - 2.0f * fIn);
    //case UI_LERPTYPE_SPEED::CUBICR:
    }
}

_float CAnimator_UI::Calc_LerpRatio(_float fStart, _float fEnd, _float fRatio)
{
    if (fRatio > 1.f || fRatio < 0.f)
        CRASH("Unclamped Data. fRatio must between [0, 1]");

    return (fStart * (1 - fRatio)) + (fEnd * fRatio);
}

_float3 CAnimator_UI::Calc_Lerp_Position_CMR(_uint iKeyframe)
{
    if (!m_pCurAnimDesc)
        return _float3();

    _vector vPoses[4]               = {};

    _uint iKeyframeTimeStart        = UINT_MAX;
    _uint iKeyframeTimeEnd          = UINT_MAX;
    const _bool isLoop              = m_pCurAnimDesc->isLoop;
    const _uint iLastKeyframeIndex  = m_pCurAnimDesc->vecKeyFrames.size() - 1;
    _uint iKeyframeIndex            = 0;

    // ?꾩옱 ?ㅽ봽?덉엫??vector ???몃뜳?ㅻ? 寃??
    for (_uint i = 0; i < m_pCurAnimDesc->vecKeyFrames.size(); i++)
    {
        if (m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex > iKeyframe)
            break;
        iKeyframeIndex = i;
    }
    
    // lerp???ъ슜??媛믩뱾 ?좊떦
    for (_uint i = 0; i < 4; i++)
    {
        // 1, 2 ?ъ엲媛믪쓣 ?ъ슜??寃?
        // ?ㅻ쭔 ?몃뜳?ㅻ? 踰쀬뼱?섎뒗 寃쎌슦??????뺤쓽. ?대뒗 loop ?щ????곕씪 ?ㅻ쫫.
        _uint iIndex = iKeyframeIndex + i - 1;
        if (iIndex < 0)
            iIndex = (isLoop) ? iLastKeyframeIndex : 0;
        else if (iIndex > iLastKeyframeIndex)
            iIndex = (isLoop) ? 0 : iLastKeyframeIndex;

        vPoses[i] = XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iIndex].vPos);
        
        if (i == 1) iKeyframeTimeStart  = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (i == 2) iKeyframeTimeEnd    = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (iKeyframeTimeStart == iKeyframeTimeEnd) iKeyframeTimeEnd = m_pCurAnimDesc->vecKeyFrames[(iIndex + 1) % m_pCurAnimDesc->vecKeyFrames.size()].iKeyframeIndex;
    }

    // ?좊떦??媛믪쓣 ?댁슜?섏뿬 怨꾩궛, 諛섑솚
    // ?ㅽ봽?덉엫 李⑥뿉 ?곕Ⅸ 媛꾧꺽??怨좊젮?댁뿬 怨꾩궛?댁빞 ?? XMVectorCatmullRom ???ㅽ봽?덉엫 媛꾧꺽??媛숈쓬???꾩젣湲??뚮Ц.
    // 1, 2 ?ъ씠???ㅽ봽?덉엫??湲곗??쇰줈 ratio 怨꾩궛?섏뿬 ?몄옄瑜?二쇰㈃ ?좊벏?

    _float fKeyframeRatio = (_float)(iKeyframe - iKeyframeTimeStart) / (iKeyframeTimeEnd - iKeyframeTimeStart);

    _vector vResultPos = XMVectorCatmullRom(vPoses[0], vPoses[1], vPoses[2], vPoses[3], fKeyframeRatio);
    _float3 vResult = {};
    XMStoreFloat3(&vResult, vResultPos);

    return vResult;
}

void CAnimator_UI::Update_Animation(_float fTimeDelta)
{
    if (m_pCurAnimDesc == nullptr)
        return;

    // Calculate Frame..
    const _uint     iKeyFrameRate       = 60; // 湲곗? 珥덈떦 ?꾨젅??
    const _float    fSingleFrameTime    = 1.f / iKeyFrameRate;

    _float fCurFrame = m_fElapsedTime / fSingleFrameTime;                   // ?꾩옱 ?ㅽ봽?덉엫
    if (fCurFrame >= m_pCurAnimDesc->vecKeyFrames.back().iKeyframeIndex)
    {
        if (m_pCurAnimDesc->isLoop)
            m_fElapsedTime = 0.f;                                           // 猷⑦봽 ?? 踰붿쐞 ?섏뼱媛硫?0?쇰줈
    }

    fCurFrame = m_fElapsedTime / fSingleFrameTime;                          // 理쒖쥌 ?꾩옱 ?ㅽ봽?덉엫
    _uint iCurFrame = static_cast<_uint>(fCurFrame);                        // 理쒖쥌 ?꾩옱 ?ㅽ봽?덉엫 (int濡??대┝)


    // ==============================
    // * Calculate Ratio..
    // ==============================
    _uint iFrame_LerpStart = {};        // ?꾨젅??媛?
    _uint iFrame_LerpEnd = {};          // ?꾨젅??媛?
    _uint iFrame_StartIndex = {};       // ?쒖닔 ?몃뜳??
    _uint iFrame_EndIndex = {};         // ?쒖닔 ?몃뜳??

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
    _float fFixedLerpRatio = Fix_LerpRatio(fRawLerpRatio, m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iLerpType); // ?댁쟾 ?ㅽ봽?덉엫? ?꾩옱 ?ㅽ봽?덉엫 媛꾩쓽 理쒖쥌 蹂닿컙 鍮꾩쑉


    // ==============================
    // * Calculate Results..
    // ==============================
    _uint iResultTexIndex = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iTexIndex;
    _float fResultAlpha = Calc_LerpRatio(
        m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].fAlpha, 
        m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].fAlpha, 
        fFixedLerpRatio
    );
    //_float3 vResultPos = {};
    // ksta : cmr ?ｌ쑝硫??대?遺??쒓굅 諛?蹂寃??꾩슂
    //XMStoreFloat3(&vResultPos, XMVectorLerp(
    //    XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vPos),
    //    XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vPos),
    //    fFixedLerpRatio)
    //);

    _float3 vResultPos = Calc_Lerp_Position_CMR(iCurFrame);

    _float3 vResultRot = {};
    XMStoreFloat3(&vResultRot, XMVectorLerp(        // degree?쇱꽌 洹몃윴 寃?媛숈???. 
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vRot),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vRot),
        fFixedLerpRatio)
    );
    _float3 vResultRotRad = { DegreesToRadians(vResultRot.x), DegreesToRadians(vResultRot.y), DegreesToRadians(vResultRot.z) };
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
    CShader* pTargetShader = dynamic_cast<CShader*>(m_pOwner->Get_Component(L"Com_Shader"));
    pTargetShader->Bind_Value("g_AlphaStrength", &fResultAlpha, sizeof(fResultAlpha));

    CTransform* pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(L"Com_Transform"));
    
    _matrix matPos = XMMatrixTranslationFromVector(XMLoadFloat3(&vResultPos));
    _matrix matRot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&vResultRotRad));
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
