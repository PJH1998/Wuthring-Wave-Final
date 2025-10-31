#include "ClientPch.h"
#include "Factory.h"

#include "MonsterDummy.h"

CFactory::CFactory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance { CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CFactory::Initialize()
{
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MonsterDummy"),
		CMonsterDummy::Create(m_pDevice, m_pContext))))
		CRASH("Monster Dummy Prototype");

	return S_OK;
}

void CFactory::Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix)
{
	CMonsterDummy::MONSTER_DUMMY_DESC MonsterDummyDesc = {};
	MonsterDummyDesc.vPos = vPos;
	MonsterDummyDesc.PreTransformationMatrix = PreTransformationMatrix;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MonsterDummy"),
		ENUM_CLASS(eLayerLevel), TEXT("Layer_Dummy"), &MonsterDummyDesc)))
		CRASH("Monster Dummy Clone");
}

CFactory* CFactory::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFactory* pInstance = new CFactory(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
		CRASH("Factory");

	return pInstance;
}

void CFactory::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
