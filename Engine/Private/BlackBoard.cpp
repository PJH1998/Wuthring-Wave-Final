#include "EnginePch.h"
#include "BlackBoard.h"

CBlackBoard::CBlackBoard()
{
}

HRESULT CBlackBoard::Add_Data(const _string& strDataTag, DATA_TYPE eType, void* pValue)
{
	if(!Find_Data(strDataTag))
	{
		if(eType >= ENUM_CLASS(DATA_TYPE::DATA_END))
			return E_FAIL;
		pair<DATA_TYPE, void*> pair = {eType, pValue};
		m_Datas.emplace(make_pair(strDataTag, pair));

		return S_OK;
	}
	else
	{
		// 이미 존재하는 데이터 키
		return E_FAIL;
	}
}

void* CBlackBoard::Get_Data(const _string& strDataTag)
{
	if(Find_Data(strDataTag))
		return m_Datas[strDataTag].second;
	else
		return nullptr;
}

HRESULT CBlackBoard::Add_Condition(const _string& strDataTag, function<_int()> Condition)
{
	m_Conditions.emplace(make_pair(strDataTag, Condition)); return S_OK;
}

_int CBlackBoard::Get_Condition(const _string& strFuncTag)
{
	if(m_Conditions.find(strFuncTag) == m_Conditions.end())
		return -1;

	return m_Conditions[strFuncTag]();
}

#ifdef _DEBUG
void CBlackBoard::Bind_Data_to_GUI()
{
	ImGui::Begin("BlackBoard");
	for(auto& Pair : m_Datas)
	{
		const char* strKey = Pair.first.c_str();
		DATA_TYPE eType = Pair.second.first;
		void* pValue = Pair.second.second;
		switch(eType)
		{
		case INT:
			ImGui::InputInt(strKey, static_cast<_int*>(pValue));
			break;
		case FLOAT:
			ImGui::InputFloat(strKey, static_cast<_float*>(pValue));
			break;
		case STRING:
		{
			char strBuffer[MAX_PATH] = {};
			strcpy_s(strBuffer, MAX_PATH, strKey);
			strcat_s(strBuffer, MAX_PATH, ": ");
			strcat_s(strBuffer, MAX_PATH, static_cast<_char*>(pValue));
			ImGui::Text(strBuffer);
			break;
		}
		case BOOL:
			ImGui::Checkbox(strKey, static_cast<_bool*>(pValue));
			break;
		case VECTOR3:
			ImGui::InputFloat3(strKey, static_cast<_float*>(pValue));
			break;
		case VECTOR4:
			ImGui::InputFloat4(strKey, static_cast<_float*>(pValue));
			break;
		default:
			break;
		}
	}
	ImGui::End();
}
void CBlackBoard::Clear_Data()
{
	m_Datas.clear();
}
#endif // _DEBUG

_bool CBlackBoard::Find_Data(const _string& strDataTag)
{
	return m_Datas.find(strDataTag) != m_Datas.end();
}

CBlackBoard* CBlackBoard::Create()
{
	CBlackBoard* pInstance = new CBlackBoard();
	return pInstance;
}

void CBlackBoard::Free()
{
	__super::Free();

#ifdef _DEBUG
	Clear_Data();
#endif // _DEBUG


}
