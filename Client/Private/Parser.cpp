#include "ClientPch.h"
#include "Parser.h"

IMPLEMENT_SINGLETON(CParser)

CParser::CParser()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CParser::Create_Map_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, LEVEL eLevel)
{

}

void CParser::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
