#include "ClientPch.h"
#include "Loader_Test.h"

#include "Dummy.h"
#include "MapObject.h"
#include "AnimationDummy.h"
#include "MonsterTest.h"

#include "StateMachine.h"

#include "Augusta.h"
#include "Player.h"



#pragma region BehaviorTree
#include "BT_Action.h"
#include "BT_Selector.h"
#include "BT_Sequence.h"
#pragma endregion
#include"GameSystem.h"

CLoader_Test::CLoader_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Test::Initialize()
{
	CoInitializeEx(nullptr, 0);

	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

    m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });

    m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
    
    

    m_pGameInstance->Wait_Thread_End();
    return S_OK;
}

HRESULT CLoader_Test::Load_Texture()
{
	cout << "Texture" << endl;
    
    return S_OK;
}

HRESULT CLoader_Test::Load_Model()
{
    m_pGameSystem->Create_Map_Model("../Bin/Resource/Map/MapData/PLAYER_TEST/", m_eCurLevel);
    //m_pGameSystem->Create_Map_Model("../Bin/Resource/Map/MapData/Kings_Load_1026_First/", m_eCurLevel);

    // Prototype_Component_Model_FalseSoverign
    //_fmatrix PreMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
    //if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_FalseSoverign"),
    //    CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreMatrix, "../Bin/Resource/Model/Player/FalseSovereign/False_SovereignTest1.dat"))))
    //    return E_FAIL;

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Test::Load_Shader()
{
	cout << "Shader" << endl;

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }

    

    return S_OK;
}

HRESULT CLoader_Test::Load_Object()
{
    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
        CMapObject::Create(m_pDevice, m_pContext));


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Dummy"),
		CDummy::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	//if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MonsterTest"), CMonsterTest::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	cout << "Object" << endl;

    return S_OK;
}

HRESULT CLoader_Test::Load_Component()
{
    cout << "Component" << endl;

	//if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_BehaviorTree_Test"),
	//	CBehavior_Tree::Create(m_pDevice, m_pContext, pRoot))))
	//	return E_FAIL;

    return S_OK;
}

HRESULT CLoader_Test::Load_Player()
{

    // Controller 초기화
    _wstring wstrControllerTag = L"Prototype_Component_PlayerController";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wstrControllerTag,
        CInputController::Create(m_pDevice, m_pContext))))
        CRASH("PlayerInput Controller");
    
    _wstring wStrControllerTag = TEXT("Prototype_GameObject_Player");
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrControllerTag
        , CPlayer::Create(m_pDevice, m_pContext))))
        CRASH("Prototype Create Failed");

    return S_OK;
}

HRESULT CLoader_Test::Load_Augusta()
{
    _wstring wStrModelTag = L"Prototype_Component_Model_Augusta";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Augusta.dat";
    _matrix		PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.01f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(XM_PI));

    // 1. 모델 초기화.
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
        CRASH("Prototype Create Failed");


    // 2. StateMachine 초기화
    _wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_Augusta";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
        CStateMachine::Create(m_pDevice, m_pContext))))
        CRASH("PlayerState Machine");

   



    _wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Augusta");

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrActorTag
        , CAugusta::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

    
    /*if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , TEXT("Prototype_GameObject_Dummy_Augusta")
        , CAnimationDummy::Create(m_pDevice, m_pContext))))
        CRASH("Prototype Create Failed");*/


    return S_OK;
}

CLoader_Test* CLoader_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Test* pInstance = new CLoader_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Test::Free()
{
    __super::Free();
}
