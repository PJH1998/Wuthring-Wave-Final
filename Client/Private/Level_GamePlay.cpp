#include "ClientPch.h"
#include "Level_GamePlay.h"
#include "MonsterTest.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext)
{
}

HRESULT CLevel_GamePlay::Initialize()
{
    return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("GamePlay"));
}

void CLevel_GamePlay::Render()
{
}

void CLevel_GamePlay::Ready_MonsterTest()
{
	CMonsterTest::MONSTERTEST_DESC MobDesc{};
	MobDesc.eCurLevel = LEVEL::GAMEPLAY;
	MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	MobDesc.modelData = make_pair(LEVEL::GAMEPLAY, TEXT("Prototype_Component_Model_FalseSovereign"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(0.f, -8.f, 4.f);
	MobDesc.pAnimationTag = "Born1";
	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_MonsterTest"), &MobDesc)))
		CRASH("Failed Ready MonsterTest");
}

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_GamePlay::Free()
{
    __super::Free();
}
