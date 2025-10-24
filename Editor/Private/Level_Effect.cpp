#include "EditorPch.h"
#include "Level_Effect.h"
#include "Event_Level.h"
#include "Effect_Controller.h"
#include "Particle.h"
#include "Effect_Mesh.h"
#include "ComputeShader.h"
#include "AnimationTool.h"
#include "Trail_Mesh.h"

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

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"),
        CEffect_Mesh::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_TrailMesh"),
        CTrail_Mesh::Create(m_pDevice, m_pContext));

    //파티클 그리기용 셰이더
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements));

    //매쉬 그리기용 셰이더
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_FXMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxFXMesh_Instance.hlsl"), VTXFXMESHINSTANCE::Elements, VTXFXMESHINSTANCE::iNumElements));

    //일반 매쉬 그리기용 셰이더 추가해줘야함.
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxTrailMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxTrailMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));
        
    //파티클 연산용 셰이더
    SHADER_MACRO eShaderMacro = {
        {"THREAD_X", "64" }
        ,{"THREAD_Y", "1" }
        ,{"THREAD_Z", "1" }
        , { NULL, NULL }
    };

    string strEntryPoint = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_Particle"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ParticleUpdate_CS.hlsl"), eShaderMacro, strEntryPoint));


    //FX매쉬 연산용 셰이더

    SHADER_MACRO eShaderMacroMesh = {
      {"THREAD_X", "64" }
      ,{"THREAD_Y", "1" }
      ,{"THREAD_Z", "1" }
      , { NULL, NULL }
    };

    string strEntryPointMesh = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_FXMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_FXMeshUpdate_CS.hlsl"), eShaderMacroMesh, strEntryPointMesh));

    //이펙트 툴
    m_pEffect_Controller = CEffect_Controller::Create(m_pDevice, m_pContext);


    //파티클 움직임 및 위치같은 설정들 보기 위해 플레이어 띄울려고 추가함. 영훈오빠가 만든 애니메이션 툴
    m_pAnimation_Tool = CAnimationTool::Create(m_pDevice, m_pContext, LEVEL::EFFECT);

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }

    //애니메이션 연산용 셰이더
    SHADER_MACRO eShaderMacroB = {
    {"THREAD_X", "64" }
    ,{"THREAD_Y", "1" }
    ,{"THREAD_Z", "1" }
    , { NULL, NULL }
    };

    string strEntryPointB = "CSMain";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
            , eShaderMacroB, strEntryPointB))))
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
