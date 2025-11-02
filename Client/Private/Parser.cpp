#include "ClientPch.h"
#include "Parser.h"
#include	"MapObject.h"

CParser::CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance{ CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CParser::Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel)
{
    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    _string PasingDir = FileDir;

    Read_Map_Prototype(PasingDir, eLevel);
	m_LoadingMap[eLevel].push_back(pFilePath);
}

void CParser::Read_Map_Prototype(const _string pFilePath, LEVEL eLevel)
{
	//넘어오는 건 폴더 경로.
	_string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
	ProjectPath += "/Client/Bin/Resource/Map";
	_float fSize = 0.01f;
	_matrix PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	_wstring PrototypeName = L"Prototype_Component_Model_";
	_wstring InstancePrototypeName = L"Prototype_Component_Model_Instance_";


	for (const auto& entry : filesystem::directory_iterator(pFilePath)) {
		if (!entry.is_regular_file())
			continue;
		if (entry.path().string().find("Prototype") == std::string::npos)
			continue;

		_string strFilePath = entry.path().string();
		ifstream File(strFilePath, ios::binary);

		_uint NameLength = {};

		_char Name[MAX_PATH] = {};
		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Name, 0, sizeof(Name));
			File.read(reinterpret_cast<_char*>(&Name), NameLength);
			//여기서 프로토타입 생성.
			_uint ProtoMax = Name[strlen(Name) - 1] - '0' + 1;
			_string ModelName = Name;
			ModelName.pop_back();

			for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
				if (entry2.path().string().find("MapData") != std::string::npos)
					continue;

				if (entry2.path().string().find("Test") != std::string::npos)
					continue;

				if (entry2.path().string().find(ModelName) == std::string::npos)
					continue;

				if (entry2.path().extension() != ".dat")
					continue;

				if (entry2.path().string().find("Anim") != std::string::npos)
					continue;

				_string Path = entry2.path().string();
				_string Prototype = entry2.path().stem().string();
				//파서 수정중
				if (entry2.path().string().find("_Bone") != std::string::npos)
				{
					m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(Prototype), ModelPath = Path]() {
						if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName + StringToWString(Prototype),
							CModel::Create(m_pDevice, m_pContext, MODELTYPE::ECO, PreTransformMatrix, ModelPath.c_str()))))
							CRASH("Prototype Create Failed");
						});
				}
				else if (entry2.path().string().find("Instance") != std::string::npos)
				{
					m_pGameInstance->Add_Work([=, Model = InstancePrototypeName + StringToWString(Prototype), ModelPath = Path]() {
						if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), Model,
							CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, ModelPath.c_str()))))
							CRASH("Prototype Create Failed");
						});
				}
				else
					//if (entry2.path().string().find("Instance") == std::string::npos)
				{
					m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(Prototype), ModelPath = Path]() {
						if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName + StringToWString(Prototype),
							CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, ModelPath.c_str()))))
							CRASH("Prototype Create Failed");
						});
				}
				//프로토타입 생성
			}
		}
		File.close();
	}
}

void CParser::Clone_MapObjects(LEVEL eLevel, _uint iIndex)
{
	if (!m_LoadingMap[eLevel][iIndex])
		CRASH("Failed");

	_char FileDrive[MAX_PATH] = {};
	_char FileDir[MAX_PATH] = {};
	_char FileName[MAX_PATH] = {};
	_char FileExt[MAX_PATH] = {};

	_splitpath_s(m_LoadingMap[eLevel][iIndex], FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

	for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".dat")
			continue;

		if (entry.path().string().find("Prototype") != std::string::npos)
			continue;


		_string strFilePath = entry.path().string();

		Read_Map_Dat(eLevel, strFilePath);
	}
}

void CParser::Read_Map_Dat(LEVEL eLevel, const _string pFilePath)
{
	ifstream File(pFilePath, ios::binary);

	if (!File.is_open())
	{
		MSG_BOX("Load Failed");
	}

		_uint NameLength;

		_matrix PreTransformMatrix = XMMatrixIdentity();
		_float fSize = 0.01f;
		PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	if (pFilePath.find("Instance") != std::string::npos)
	{
		return;
	}
	else if (pFilePath.find("Destruction") != std::string::npos)
	{
		return;
	//	_uint NameLength;

	//	_matrix PreTransformMatrix = XMMatrixIdentity();
	//	_float fSize = 0.01f;
	//	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	//	CMapObject_Destruction::MAP_LOAD Desc{};

	//	while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
	//	{
	//		memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
	//		File.read(Desc.ModelName, NameLength);
	//		_string Name = Desc.ModelName;

	//		File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
	//		File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
	//		_float4x4 Matrix = {};
	//		File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
	//		Desc.WorldMatrix = &Matrix;
	//		Desc.iLevel = m_iLevel;

	//		File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePos), sizeof(_float3));
	//		File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePower), sizeof(_float3));

	//		m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex,
	//			eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix,
	//			vImpulsePos = Desc.m_vImpulsePos, vImpulsePower = Desc.m_vImpulsePower]() mutable {
	//			CMapObject_Destruction::MAP_LOAD pDesc{};
	//			strcpy_s(pDesc.ModelName, ModelName.c_str());
	//			pDesc.iShaderPassIndex = ShaderPass;
	//			pDesc.eObjectType = eObjectType;
	//			pDesc.WorldMatrix = &Matrix;
	//			pDesc.iLevel = ENUM_CLASS(eLevel);
	//			pDesc.m_vImpulsePos = vImpulsePos;
	//			pDesc.m_vImpulsePower = vImpulsePower;
	//			m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_Destruction")
	//				, PROTOTYPE::GAMEOBJECT, &pDesc);
	//			});
	//	}
	}
	else
	{
		CMapObject::MAP_LOAD Desc{};

		_wstring PrototypeName = TEXT("Prototype_Component_Model_");

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);

			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;

			//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
			_wstring ModelName = StringToWString(Desc.ModelName);

			m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
				CMapObject::MAP_LOAD pDesc{};
				strcpy_s(pDesc.ModelName, ModelName.c_str());
				pDesc.iShaderPassIndex = ShaderPass;
				pDesc.eObjectType = eObjectType;
				pDesc.WorldMatrix = &Matrix;
				pDesc.iLevel = ENUM_CLASS(eLevel);
				m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject")
					, PROTOTYPE::GAMEOBJECT, &pDesc);
				});
		}
	}
	m_pGameInstance->Wait_Thread_End();
	File.close();
}

const vector<vector<_string>>& CParser::Load_CSV(const _char* pFilePath)
{
	ifstream InputFile(pFilePath);

	_string strLine;

	while (getline(InputFile, strLine))
	{
		stringstream ss(strLine);
		_string strCell;
		vector<_string> row;

		while (getline(ss, strCell, ','))
			row.push_back(strCell);

		m_Data.push_back(row);
	}

	InputFile.close();

	return m_Data;
}

HRESULT CParser::Initialize()
{
	return S_OK;
}

CParser* CParser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParser* pInstance = new CParser(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Parser");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParser::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
