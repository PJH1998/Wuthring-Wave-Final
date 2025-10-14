#include "ClientPch.h"
#include "Loader.h"

#include "Parser.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance { CGameInstance::GetInstance() },
	m_pParser { CParser::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pParser);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CLoader::Complete_Load()
{
	lock_guard<mutex> lock(m_Mutex);
	m_fProgress += 25.f;
}

void CLoader::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
