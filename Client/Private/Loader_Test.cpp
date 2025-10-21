#include "ClientPch.h"
#include "Loader_Test.h"

#include "Dummy.h"
#include "MapObject.h"
#include "AnimationDummy.h"
#include "MonsterTest.h"

#pragma region BehaviorTree
#include "BT_Action.h"
#include "BT_Selector.h"
#include "BT_Sequence.h"
#pragma endregion

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
	//m_pGameInstance->Add_Work([this]() {Ready_OctoTree(); Complete_Load(); });

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
    _matrix PreTransformMatrix;
    _float fSize = 0.1f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    string FolderPath = "../../Client/Bin/Resource/Map/";

    _int version = {};
    _int Lastversion = {};
    _wstring LastVersionName;
    _string LastVersionPath;

    for (const auto& entry : filesystem::recursive_directory_iterator(FolderPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().string().find("MapData") != std::string::npos)
                continue;

            if (entry.path().extension() == ".dat") {

                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                _wstring baseName = StringToWString(FileName);

                // LOD 마지막에 붙은 숫자 추출
                size_t pos = baseName.find_last_not_of(L"0123456789");
                _wstring namePart = baseName.substr(0, pos + 1);
                _wstring numberPart = baseName.substr(pos + 1);
                version = stoi(numberPart);

                _wstring key = L"Prototype_Component_Model_" + namePart;
                _string VersionPath = FileDir;
                VersionPath += FileName;
                VersionPath += ".dat";

                _wstring PrototypeName = L"Prototype_Component_Model_";
                PrototypeName += StringToWString(FileName);

                m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, FileDir = VersionPath]() {

                    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), PrototypeName,
                        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, VersionPath.c_str()))))
                        CRASH("Prototype Create Failed");
                    });
            }
        }
    }

    // Prototype_Component_Model_Augusta
    _fmatrix PreMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
    if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Augusta"),
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreMatrix, "../Bin/Resource/Model/Player/Augusta/Augusta.dat"))))
        return E_FAIL;

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

    SHADER_MACRO eShaderMacro = {
        {"THREAD_X", "64" }
        ,{"THREAD_Y", "1" }
        ,{"THREAD_Z", "1" }
        , { NULL, NULL }
    };

    string strEntryPoint = "CSMain";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
            , eShaderMacro, strEntryPoint))))
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

HRESULT CLoader_Test::Load_Augusta()
{
    _wstring wStrModelTag = L"Prototype_Component_Model_Augusta";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Augusta.dat";
    _matrix		PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.01f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(XM_PI));

	m_pGameInstance->Add_Work([=]() {
			if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
				CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
				CRASH("Prototype Create Failed");
		});

	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Augusta");
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrActorTag
        , CAnimationDummy::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

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
