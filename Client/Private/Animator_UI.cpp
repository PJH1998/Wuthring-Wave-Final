#include "ClientPch.h"
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

	if (m_pOwner)
	{
		m_pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(L"Com_Transform"));
		m_pOwnerShaderCom = dynamic_cast<CShader*>(m_pOwner->Get_Component(L"Com_Shader"));
	}


    return S_OK;
}

void CAnimator_UI::Priority_Update(_float fTimeDelta)
{

}

void CAnimator_UI::Update(_float fTimeDelta)
{ 
    if (m_pCurAnimDesc)
        m_fElapsedTime += fTimeDelta;
    else
        m_fElapsedTime = 0.f;

	Update_Animation_Calculate();
}

void CAnimator_UI::Late_Update(_float fTimeDelta)
{

}

HRESULT CAnimator_UI::Render()
{
	Update_Animation_BindShader();
    return S_OK;
}

HRESULT CAnimator_UI::Insert_Animation(UI_ANIM_DESC& Desc)
{
    if (Find_Animation(Desc.strAnimName))
        return E_FAIL;

    m_vecAnimationDescs.push_back(Desc);
    return S_OK;
}

HRESULT CAnimator_UI::Remove_Animation(_wstring strAnimName)
{
    UI_ANIM_DESC* pDesc = nullptr;

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

HRESULT CAnimator_UI::Change_Animation(_wstring strAnimName, _bool isForceRestart)
{
    UI_ANIM_DESC* pDesc = Find_Animation(strAnimName);

    if (!pDesc)
        return E_FAIL;

	if (!(pDesc == m_pCurAnimDesc && !isForceRestart))
		m_fElapsedTime = 0;
		
    m_pCurAnimDesc = pDesc;

    return S_OK;
}

HRESULT CAnimator_UI::Change_Animation(_uint iAnimIndex, _bool isForceRestart)
{
    UI_ANIM_DESC* pDesc = Find_Animation(iAnimIndex);

    if (!pDesc)
        return E_FAIL;

	if (!(pDesc == m_pCurAnimDesc && !isForceRestart))
		m_fElapsedTime = 0;

    m_pCurAnimDesc = pDesc;
    m_fElapsedTime = 0;

    return S_OK;
}

HRESULT CAnimator_UI::Deselect_Animation()
{
    m_pCurAnimDesc = nullptr;

    return S_OK;
}

CAnimator_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_wstring strAnimName)
{
    UI_ANIM_DESC* pDesc = nullptr;

    for (auto& animDesc : m_vecAnimationDescs)
        if (animDesc.strAnimName == strAnimName)
        {
            pDesc = &animDesc;
            break;
        }

    return (pDesc) ? pDesc : nullptr;
}

CAnimator_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_uint iAnimIndex)
{
    if (iAnimIndex >= m_vecAnimationDescs.size())
        return nullptr;

    return &m_vecAnimationDescs[iAnimIndex];
}

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

    _vector     vPoses[4] = {};

    _uint       iKeyframeTimeStart = UINT_MAX;
    _uint       iKeyframeTimeEnd = UINT_MAX;
    const _bool isLoop = m_pCurAnimDesc->isLoop;
    const _uint iLastKeyframeIndex = (_uint)(m_pCurAnimDesc->vecKeyFrames.size() - 1);
    _uint       iKeyframeIndex = 0;

    // ���� Ű�������� vector �� �ε����� �˻�
    for (_uint i = 0; i < m_pCurAnimDesc->vecKeyFrames.size(); i++)
    {
        if (m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex > iKeyframe)
            break;
        iKeyframeIndex = i;
    }
	m_pCurKeyFrameDesc = &m_pCurAnimDesc->vecKeyFrames[iKeyframeIndex];

    // lerp�� ����� ���� �Ҵ�
    for (_uint i = 0; i < 4; i++)
    {
        // 1, 2 ���հ��� ����� ��.
        // �ٸ� �ε����� ����� ��쿡 ���� ����. �̴� loop ���ο� ���� �ٸ�.
        _uint iIndex = iKeyframeIndex + i - 1;
        if (iIndex < 0)
            iIndex = (isLoop) ? iLastKeyframeIndex : 0;
        else if (iIndex > iLastKeyframeIndex)
            iIndex = (isLoop) ? 0 : iLastKeyframeIndex;

        vPoses[i] = XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iIndex].vPos);

        if (i == 1) iKeyframeTimeStart = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (i == 2) iKeyframeTimeEnd = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (iKeyframeTimeStart == iKeyframeTimeEnd) iKeyframeTimeEnd = m_pCurAnimDesc->vecKeyFrames[(iIndex + 1) % m_pCurAnimDesc->vecKeyFrames.size()].iKeyframeIndex;
    }

    _float fKeyframeRatio = {};

    if ((iKeyframeTimeEnd - iKeyframeTimeStart) == 0)       fKeyframeRatio = 0;
    else        fKeyframeRatio = (_float)(iKeyframe - iKeyframeTimeStart) / (iKeyframeTimeEnd - iKeyframeTimeStart);

    _vector vResultPos = XMVectorCatmullRom(vPoses[0], vPoses[1], vPoses[2], vPoses[3], fKeyframeRatio);
    _float3 vResult = {};
    XMStoreFloat3(&vResult, vResultPos);

    return vResult;
}	

void CAnimator_UI::Update_Animation_Calculate()
{
    // if target doesnt have selected animation, binds default value to shader.
    // if not, it will be affected by pre-played animations.

    if (!(m_pOwner && m_pOwnerShaderCom))		// rootUI doesnt need animator.
        return;

	UI_ANIM_KEYFRAME_DESC tDesc = {};
    CShader* pTargetShader = m_pOwnerShaderCom;

    if (m_pCurAnimDesc == nullptr)
    {
		m_tCalcedKeyFrameDesc = UI_ANIM_KEYFRAME_DESC{};
		return;
    }



    // Calculate Frame..
    const _uint     iKeyFrameRate = 60; // ���� �ʴ� ������
    const _float    fSingleFrameTime = 1.f / iKeyFrameRate;

    _float fCurFrame = m_fElapsedTime / fSingleFrameTime;                   // ���� Ű������
    if (fCurFrame >= m_pCurAnimDesc->vecKeyFrames.back().iKeyframeIndex)
    {
        if (m_pCurAnimDesc->isLoop)
            m_fElapsedTime = 0.f;                                           // ���� ��, ���� �Ѿ�� 0����
    }

    fCurFrame = m_fElapsedTime / fSingleFrameTime;                          // ���� ���� Ű������
    _uint iCurFrame = static_cast<_uint>(fCurFrame);                        // ���� ���� Ű������ (int�� ����)


    // ==============================
    // * Calculate Ratio..
    // ==============================
    _uint iFrame_LerpStart = {};        // ������ ��
    _uint iFrame_LerpEnd = {};          // ������ ��
    _uint iFrame_StartIndex = {};       // ���� �ε���
    _uint iFrame_EndIndex = {};         // ���� �ε���

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
    _float fFixedLerpRatio = Fix_LerpRatio(fRawLerpRatio, m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iLerpType); // ���� Ű�����ӿ� ���� Ű������ ���� ���� ���� ����


    // ==============================
    // * Calculate Results..
    // ==============================
    _uint iResultTexIndex = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iTexIndex;
    _float fResultAlpha = Calc_LerpRatio(
        m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].fAlpha,
        m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].fAlpha,
        fFixedLerpRatio
    );

	_float3 vResultPos = Calc_Lerp_Position_CMR(iCurFrame);

    _float3 vResultRot = {};
    XMStoreFloat3(&vResultRot, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vRot),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vRot),
        fFixedLerpRatio)
    );
    _float3 vResultRotRad = { DegreesToRadians(vResultRot.x), DegreesToRadians(vResultRot.y), DegreesToRadians(vResultRot.z) };
    _float3 vResultSca = {};
    XMStoreFloat3(&vResultSca, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vSca),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vSca),
        fFixedLerpRatio
    ));


    _float2 vResultScreenLT = {};
    XMStoreFloat2(&vResultScreenLT, XMVectorLerp(
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vScreenLT),
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vScreenLT),
        fFixedLerpRatio
    ));

    _float2 vResultScreenRB = {};
    XMStoreFloat2(&vResultScreenRB, XMVectorLerp(
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vScreenRB),
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vScreenRB),
        fFixedLerpRatio
    ));

    _float4 vResultOuterWidth = {};
    XMStoreFloat4(&vResultOuterWidth, XMVectorLerp(
        XMLoadFloat4(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vBlendToOuterWidth),
        XMLoadFloat4(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vBlendToOuterWidth),
        fFixedLerpRatio
    ));

    // ==============================
    // * Apply Results..
    // ==============================


    CTransform* pOwnerTransformCom = m_pOwnerTransformCom;


    _matrix matPos = XMMatrixTranslationFromVector(XMLoadFloat3(&vResultPos));
    _matrix matRot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&vResultRotRad));
    _matrix matSca = XMMatrixScalingFromVector(XMLoadFloat3(&vResultSca));

	// 만약 비활성화된 요소가 있다면 해당하는 것은 반영X




	if (m_iDisableFlag)
	{
		_vector vPos, vRot, vSca;

		_float4x4 matOwner = {}; XMStoreFloat4x4(&matOwner, pOwnerTransformCom->Get_WorldMatrix());
		XMMatrixDecompose(&vSca, &vRot, &vPos, XMLoadFloat4x4(&matOwner));

		if (m_iDisableFlag & ENUM_CLASS(UI_ANIM_DISABLE::POS))
		{
			XMStoreFloat3(&vResultPos, vPos);
			matPos = XMMatrixTranslationFromVector(vPos);
		}
		if (m_iDisableFlag & ENUM_CLASS(UI_ANIM_DISABLE::ROT))
		{
			_float4x4 tmpMat = {}; XMStoreFloat4x4(&tmpMat, XMMatrixRotationQuaternion(vRot)) ;
			_float3 tmpRotRad = _float3{ RadiansToDegrees(asin(-tmpMat._32)), RadiansToDegrees(atan2(tmpMat._31, tmpMat._33)), RadiansToDegrees(atan2(tmpMat._12, tmpMat._22)) };
			vResultRotRad = tmpRotRad;
			matRot = XMMatrixRotationQuaternion(vRot);
		}
		if (m_iDisableFlag & ENUM_CLASS(UI_ANIM_DISABLE::SCA))
		{
			XMStoreFloat3(&vResultSca, vSca);
			matSca = XMMatrixScalingFromVector(vSca);
		}
		else if (m_iDisableFlag & ENUM_CLASS(UI_ANIM_DISABLE::SCA_BLEND))
		{
			// Anim 크기/원본 크기 * 카메라 거리따른 크기 적용된 현재 크기
			_float3 vScaBlended = { XMVectorGetX(vSca) * vResultSca.x, XMVectorGetY(vSca) * vResultSca.y, XMVectorGetZ(vSca) * vResultSca.z };
			//XMStoreFloat3(&vResultSca, vSca);
			vResultSca = vScaBlended;
			matSca = XMMatrixScalingFromVector(XMLoadFloat3(&vScaBlended));
		}
	}
		
    _matrix matTransform = matSca * matRot * matPos;

    m_pOwner->Set_CurTexIndex(iResultTexIndex);


	// 계산된 결과를 복사. 실시간 desc 정보를 저장하여 자식 계산에 사용하기 위함
	m_tCalcedKeyFrameDesc = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex];
	m_tCalcedKeyFrameDesc.vPos = vResultPos;
	m_tCalcedKeyFrameDesc.vRot = vResultRotRad;
	m_tCalcedKeyFrameDesc.vSca = vResultSca;
	m_tCalcedKeyFrameDesc.vScreenLT = vResultScreenLT;
	m_tCalcedKeyFrameDesc.vScreenRB = vResultScreenRB;
	m_tCalcedKeyFrameDesc.vBlendToOuterWidth = vResultOuterWidth;
	m_tCalcedKeyFrameDesc.fAlpha = fResultAlpha;
	
	//if (pParentCombinedDesc)
	//	m_tCombinedKeyFrameDesc.fAlpha = fResultAlpha * pParentCombinedDesc->fAlpha;		// 알파값만 부모 키프레임값을 가져와 계산

	//pTargetShader->Bind_Value("g_AlphaStrength", &m_tCombinedKeyFrameDesc.fAlpha, sizeof(m_tCombinedKeyFrameDesc.fAlpha));	//
	//pTargetShader->Bind_Value("g_ScreenLT", &vResultScreenLT, sizeof(vResultScreenLT));
	//pTargetShader->Bind_Value("g_ScreenRB", &vResultScreenRB, sizeof(vResultScreenRB));
	//pTargetShader->Bind_Value("g_BlendToOuterWidth", &vResultOuterWidth, sizeof(vResultOuterWidth));
	pOwnerTransformCom->Set_WorldMatrix(matTransform);
}

void CAnimator_UI::Update_Animation_BindShader()
{
	if (!(m_pOwner && m_pOwnerShaderCom))		// rootUI doesnt need animator.
		return;

	CShader* pTargetShader = m_pOwnerShaderCom;
	if (!pTargetShader)
		return;

	pTargetShader->Bind_Value("g_AlphaStrength", &m_tCombinedKeyFrameDesc.fAlpha, sizeof(m_tCombinedKeyFrameDesc.fAlpha));
	pTargetShader->Bind_Value("g_ScreenLT", &m_tCalcedKeyFrameDesc.vScreenLT, sizeof(m_tCalcedKeyFrameDesc.vScreenLT));
	pTargetShader->Bind_Value("g_ScreenRB", &m_tCalcedKeyFrameDesc.vScreenRB, sizeof(m_tCalcedKeyFrameDesc.vScreenRB));
	pTargetShader->Bind_Value("g_BlendToOuterWidth", &m_tCalcedKeyFrameDesc.vBlendToOuterWidth, sizeof(m_tCalcedKeyFrameDesc.vBlendToOuterWidth));
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
