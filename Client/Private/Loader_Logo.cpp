#include "ClientPch.h"
#include "Loader_Logo.h"

#include "Dummy.h"
#include "ShadowDummy.h"
#include"Parser.h"
#include "GameSystem.h"
#include "LogoMaleRover.h"
#include "LogoFemaleRover.h"

CLoader_Logo::CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Logo::Initialize()
{
	m_iNumLoadingThread = 7;
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTable(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_LogoMaleRover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_LogoFeMaleRover(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_Logo::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Model()
{
	//m_pParser->Ready_Prototype_Map(m_pDevice, m_pContext, "../Bin/Resource/Map/MapData/Client_Test3_NonInteraction.dat", LEVEL::LOGO);

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Object()
{
	cout << "Object" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_LogoMaleRover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_MaleRover";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Male/LogoMaleRover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_MaleRover";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoMaleRover");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CLogoMaleRover::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	cout << "Logo Male Rover" << endl;
	return S_OK;
}

HRESULT CLoader_Logo::Load_LogoFeMaleRover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_FemaleRover";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Female/LogoFemaleRover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationZ(XMConvertToRadians(90.f));// *  XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_FemaleRover";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoFemaleRover");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CLogoFemaleRover::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	cout << "Logo FeMale Rover" << endl;

	return S_OK;
}
HRESULT CLoader_Logo::Load_MonsterTable()
{
	if (FAILED(CGameSystem::GetInstance()->LoadMonsterTable("../Bin/Resource/Data/MonsterTable.csv")))
		return E_FAIL;

	cout << "Monster Table" << endl;

	return S_OK;
}

CLoader_Logo* CLoader_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Logo* pInstance = new CLoader_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Logo::Free()
{
    __super::Free();
}
