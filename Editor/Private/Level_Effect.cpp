#include "EditorPch.h"
#include "Level_Effect.h"
#include "Event_Level.h"
#include "Effect_Controller.h"
#include "Particle.h"
#include "ComputeShader.h"

#include "AnimationTool.h"

CLevel_Effect::CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Effect::Initialize()
{
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Prefab"),
        CEffect_Prefab::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"),
        CParticle::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements));


    // hlsl 怨?留욎텣?? => ??媛믪? 堉?媛쒖닔? ?곴??놁씠 嫄곗쓽 怨좎젙
// ??踰덉뿉 ?묒뾽??泥섎━????????ㅻ젅?쒓? 紐뉖챸?멸?瑜??뺤쓽.
    SHADER_MACRO eShaderMacro = {
        {"THREAD_X", "64" }
        ,{"THREAD_Y", "1" }
        ,{"THREAD_Z", "1" }
        , { NULL, NULL }
    };

    string strEntryPoint = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_Particle"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ParticleUpdate_CS.hlsl"), eShaderMacro, strEntryPoint));

    //m_pParticle_Controller = CParticle_Controller::Create(m_pDevice, m_pContext);
    m_pEffect_Controller = CEffect_Controller::Create(m_pDevice, m_pContext);

    //?뚰떚???吏곸엫 諛??꾩튂媛숈? ?ㅼ젙??蹂닿린 ?꾪빐 ?뚮젅?댁뼱 ?꾩슱?ㅺ퀬 異붽???
    m_pAnimation_Tool = CAnimationTool::Create(m_pDevice, m_pContext, LEVEL::EFFECT);

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }

    return S_OK;
}

void CLevel_Effect::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Effect"));

    m_pEffect_Controller->Update();

   
}

void CLevel_Effect::Render()
{
    m_pAnimation_Tool->Render();
}


CLevel_Effect* CLevel_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Effect* pInstance = new CLevel_Effect(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Effect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Effect::Free()
{
    __super::Free();

    Safe_Release(m_pEffect_Controller);
    Safe_Release(m_pAnimation_Tool);
}
