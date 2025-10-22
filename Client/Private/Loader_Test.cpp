#include "ClientPch.h"
#include "Loader_Test.h"

#include "Dummy.h"
#include "MapObject.h"
#include "AnimationDummy.h"
#include "MonsterTest.h"

#include "PlayerAugusta.h"
#include "PlayerParty.h"


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

    m_pGameInstance->Add_Work([this]() {Load_PlayerController(); Complete_Load(); });
    
    

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

    m_pGameSystem->Create_Map_Model("../Bin/Resource/Map/MapData/Client_ShadowTest_NonInteraction.dat", m_eCurLevel);

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

	//CBT_Selector* pRoot = CBT_Selector::Create();

 //   //is Dead
 //   CBT_Action* pAction = CBT_Action::Create([](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{

 //       _bool* isAnimationFinished = static_cast<_bool*>(pBlackBoard->Get_Data("isAnimationFinished"));
	//	if(*isAnimationFinished)
	//		return CBT_Node::BT_STATE::RUNNING;
 //       
 //       _uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
 //       if(*pState & ENUM_CLASS(TEST_STATE::DEAD))
 //       {
 //           return CBT_Node::BT_STATE::SUCCESS;
 //       }

 //       return  CBT_Node::BT_STATE::FAILURE;
 //       });

	//pRoot->Add_Child(pAction);
 //   pAction = nullptr;

 //   //Attack Sequence
	//CBT_Sequence* pSequence = CBT_Sequence::Create();

 //   //      Check Enable
	//pAction = CBT_Action::Create([](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{

 //       _bool* isAnimationFinished = static_cast<_bool*>(pBlackBoard->Get_Data("isAnimationFinished"));
 //       if(!(*isAnimationFinished))
 //           return CBT_Node::BT_STATE::RUNNING;

	//	_uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
	//	if(*pState & ENUM_CLASS(TEST_STATE::DEAD))
	//		return CBT_Node::BT_STATE::FAILURE;
	//	return  CBT_Node::BT_STATE::FAILURE;
	//	});
	//pSequence->Add_Child(pAction);
	//pAction = nullptr;

	////      Attack Selector
	//CBT_Selector* pSelector = CBT_Selector::Create();

	////              Attack1
	//pAction = CBT_Action::Create([](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{
	//	_bool* isAnimationFinished = static_cast<_bool*>(pBlackBoard->Get_Data("isAnimationFinished"));
 //       if(!(*isAnimationFinished))
	//		return CBT_Node::BT_STATE::RUNNING;

 //       if(pBlackBoard->Get_Checker("Attack1_Enable") > 0)
 //       {
 //           _uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
 //           *pState |= ENUM_CLASS(TEST_STATE::ATTACK_1);
 //           return  CBT_Node::BT_STATE::SUCCESS;
 //       }
 //       else
	//		return CBT_Node::BT_STATE::FAILURE;
	//	});
 //   pSelector->Add_Child(pAction);
 //   pAction = nullptr;

 //   pSequence->Add_Child(pSelector);
 //   pSelector = nullptr;
 //   pRoot->Add_Child(pSequence);
 //   pSequence = nullptr;

 //   // Idle
	//pAction = CBT_Action::Create([](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{
	//	_uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
	//	*pState = ENUM_CLASS(TEST_STATE::NONE);
	//	return  CBT_Node::BT_STATE::SUCCESS;
	//	});
 //   pRoot->Add_Child(pAction);
 //   pAction = nullptr;

	//if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_Component_BehaviorTree_Test"),
	//	CBehavior_Tree::Create(m_pDevice, m_pContext, pRoot))))
	//	return E_FAIL;

    return S_OK;
}

HRESULT CLoader_Test::Load_PlayerController()
{
    
    _wstring wStrControllerTag = TEXT("Prototype_GameObject_PlayerParty");
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrControllerTag
        , CPlayerParty::Create(m_pDevice, m_pContext))))
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

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
        CRASH("Prototype Create Failed");


    _wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Augusta");

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrActorTag
        , CPlayerAugusta::Create(m_pDevice, m_pContext))))
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
