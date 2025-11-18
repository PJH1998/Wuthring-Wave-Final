#include "ClientPch.h"
#include "Parser.h"
#include "MapObject.h"

#include "Trigger_Box.h"
#include "MapObject_Destruction.h"
#include"MapObject_Instance.h"
#include"MapObject_Meteo.h"
#include "Spawner.h"

#include "Effect_Prefab.h"
#include "Trail_Mesh.h"
#include "Particle.h"

#include "Effect_Rect.h"
#include "Effect_Decal.h"
#include<unordered_set>

#include "Sequence.h"
#include "Effect_Radial.h"

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
		if (entry.path().string().find("Prototype") == std::string::npos && entry.path().string().find("Instance") == std::string::npos
			 && entry.path().string().find("MonsterSpawnor") == std::string::npos)
			continue;

		_string strFilePath = entry.path().string();
		ifstream File(strFilePath, ios::binary);

		_uint NameLength = {};

		_char Name[MAX_PATH] = {};

		if (entry.path().string().find("Instance") != std::string::npos)
		{
			CMapObject_Instance::MAP_LOAD Desc{};
			unordered_set<_wstring> m_Names;
			while (File.read(reinterpret_cast<char*>(&Desc.iSaveIndex), sizeof(_uint)))
			{
				CMesh_Instance::MESH_INST_DESC MeshDesc{};

				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
				File.read(Desc.ModelName, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
				if (Desc.iShaderPassIndex == 2)
					File.read(reinterpret_cast<char*>(&Desc.vDiffuseColor), sizeof(_float4));

				File.read(reinterpret_cast<char*>(&MeshDesc.iNumInstance), sizeof(_uint));
				MeshDesc.pTransformMatrix = new _float4x4[MeshDesc.iNumInstance];

				File.read(reinterpret_cast<char*>(MeshDesc.pTransformMatrix), sizeof(_float4x4) * MeshDesc.iNumInstance);

				File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

				File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
				File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

				Desc.iLevel = ENUM_CLASS(eLevel);

				_string ModelOrigin = Desc.ModelName;
				ModelOrigin.pop_back();
				for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
					if (entry2.path().string().find("Foliage") == std::string::npos)
						continue;

					if (entry2.path().string().find("Test") != std::string::npos)
						continue;

					//지금 LOD단계 다 만드는 게 아니라 하나만 만드는 거 같음.
					if (entry2.path().string().find(ModelOrigin) == std::string::npos)
						continue;
					if (entry2.path().extension() != ".dat")
						continue;

					_char FileDrive[MAX_PATH] = {};
					_char FileDir[MAX_PATH] = {};
					_char FileName[MAX_PATH] = {};
					_char FileExt[MAX_PATH] = {};
					_splitpath_s(entry2.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
					//LOD단계별로 하고있어서 0, 1, 2  해야하는데 20,21,22, 이런 식으로 됨.
					_wstring PrototypeName = L"Prototype_Component_Model_Instance_";
					_wstring ModelName = StringToWString(FileName) + to_wstring(Desc.iSaveIndex);

					PrototypeName += ModelName;
					strcpy_s(Desc.ModelName, WStringToString(ModelName).c_str());
					_string VersionPath = FileDir;
					VersionPath += FileName;
					VersionPath += ".dat";

					if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName,
						CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, VersionPath.c_str(), false, &MeshDesc))))
						CRASH("Prototype Create Failed");

				}
				m_MapInstanceData.push_back(Desc);
				Safe_Delete_Array(MeshDesc.pTransformMatrix);
			}
		}
		else if (entry.path().string().find("MonsterSpawnor") != std::string::npos)
		{
			SPAWN_DESC Desc;
			while (File.read(reinterpret_cast<char*>(&Desc.vMonsterSpawnorPos), sizeof(_float4)))
			{
				memset(Desc.szMonsterName1, 0, sizeof(Desc.szMonsterName1));
				memset(Desc.szMonsterName2, 0, sizeof(Desc.szMonsterName2));
				memset(Desc.szMonsterName3, 0, sizeof(Desc.szMonsterName3));

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos1), sizeof(_float4));

				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName1, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos2), sizeof(_float4));
				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName2, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos3), sizeof(_float4));
				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName3, NameLength);
				//m_MonsterDesc[pFilePath.c_str()].push_back(Desc);
				m_MonsterDesc[eLevel].push_back(Desc);
			}
		}
		else if(entry.path().string().find("Prototype") != std::string::npos)
		{
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
						break;
					}
					//프로토타입 생성
				}
			}
		}
		File.close();
	}
}

void CParser::Clone_MapObjects(LEVEL eLevel)
{
	if (m_LoadingMap[eLevel].empty())
		MSG_BOX("Map Clone Failed");

	_char FileDrive[MAX_PATH] = {};
	_char FileDir[MAX_PATH] = {};
	_char FileName[MAX_PATH] = {};
	_char FileExt[MAX_PATH] = {};

	for (auto& FilePath : m_LoadingMap[eLevel])
	{
		_splitpath_s(FilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

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
}

#pragma region SPAWNER
void CParser::Clone_Spawners(LEVEL eLevel)
{
	for (auto& tSpawnerData : m_MonsterDesc[eLevel])
	{
		CSpawner::SPAWNERDESC Spawner{};
		Spawner.vPosition = tSpawnerData.vMonsterSpawnorPos;
		Spawner.vExtent = _float3(100.f, 20.f, 100.f);
		Spawner.strMonsterKey = { tSpawnerData.szMonsterName1, tSpawnerData.szMonsterName2 , tSpawnerData.szMonsterName3 };
		Spawner.vSpawnPositions = { tSpawnerData.vMonsterPos1, tSpawnerData.vMonsterPos2 ,tSpawnerData.vMonsterPos3 };
		Spawner.fSpawnTime = 5.f;

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_Spawner"), ENUM_CLASS(eLevel),
			TEXT("Layer_Interaction"), &Spawner)))
			CRASH("Spawner");
	}
}
#pragma endregion

void CParser::Read_Map_Dat(LEVEL eLevel, const _string pFilePath)
{
	if (pFilePath.find("Spawn") != _string::npos)
		return;
	
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
		for (_uint i = 0; i < m_MapInstanceData.size(); ++i)
		{
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Instance")
				, ENUM_CLASS(eLevel), TEXT("Layer_Instance"), &m_MapInstanceData[i]);
		}
	}
	else if (pFilePath.find("Destruction") != std::string::npos)
	{
		_uint NameLength;

		_matrix PreTransformMatrix = XMMatrixIdentity();
		_float fSize = 0.01f;
		PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

		CMapObject_Destruction::MAP_LOAD Desc{};

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);
			_string Name = Desc.ModelName;
			OBJECTTYPE Type;
			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Type), sizeof(OBJECTTYPE));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;
			Desc.iLevel = ENUM_CLASS(eLevel);
			File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

			File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePower), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint));

			m_pGameInstance->Add_GameObject_ToLayer(Desc.iLevel, TEXT("Prototype_GameObject_MapObject_Destruction"),
				Desc.iLevel, TEXT("Layer_Destruction"), &Desc);

			//m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex,
			//	Matrix = *Desc.WorldMatrix, BoundingPos = Desc.vBoundingPos, BoundingExtends = Desc.vBoundingExtends,
			//	vImpulsePos = Desc.m_vImpulsePos, vImpulsePower = Desc.m_vImpulsePower, TriggerIndex = Desc.iTriggerIndex]() mutable {
			//	CMapObject_Destruction::MAP_LOAD pDesc{};
			//	strcpy_s(pDesc.ModelName, ModelName.c_str());
			//	pDesc.iShaderPassIndex = ShaderPass;
			//	pDesc.WorldMatrix = &Matrix;
			//	pDesc.iLevel = ENUM_CLASS(eLevel);
			//	pDesc.m_vImpulsePos = vImpulsePos;
			//	pDesc.m_vImpulsePower = vImpulsePower;
			//	pDesc.vBoundingPos = BoundingPos;
			//	pDesc.vBoundingExtends = BoundingExtends;
			//	pDesc.iTriggerIndex = TriggerIndex;
			//	m_pGameInstance->Add_GameObject_ToLayer(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_Destruction"), pDesc.iLevel, TEXT("Layer_Destruction"), &pDesc);
			//	/*m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_Destruction")
			//		, PROTOTYPE::GAMEOBJECT, &pDesc);*/
			//	});
		}
	}
	else if (pFilePath.find("Meteo") != std::string::npos) 
	{
		CMapObject_Meteo::MAP_LOAD Desc{};
		
		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);
			_string Name = Desc.ModelName;

			OBJECTTYPE Type;
			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Type), sizeof(OBJECTTYPE));
			File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

			File.read(reinterpret_cast<char*>(&Desc.vSourPos), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&Desc.vDestPos), sizeof(_float4));

			File.read(reinterpret_cast<char*>(&Desc.fDuration), sizeof(_float));
			File.read(reinterpret_cast<char*>(&Desc.fArchY), sizeof(_float));


			File.read(reinterpret_cast<char*>(&Desc.TriggerIndex), sizeof(_uint));

			File.read(reinterpret_cast<char*>(&Desc.TriggerActiveIndex), sizeof(_int));
			//XMStoreFloat4x4(&Desc.WorldMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&Desc.vSourPos)));
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Meteo")
				, ENUM_CLASS(eLevel), TEXT("Layer_Meteo"), &Desc);
		}
	}
	else if (pFilePath.find("TriggerBox") != std::string::npos)
	{
		//_uint iTriggerIndex;
		CTrigger_Box::TRIGGER Desc{};
		while (File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint)))
		{
			File.read(reinterpret_cast<char*>(&Desc.vExtends), sizeof(_float3));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_TriggerBox")
				, ENUM_CLASS(eLevel), TEXT("Layer_Trigger"), &Desc);
		}
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
			File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

			//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
			_wstring ModelName = StringToWString(Desc.ModelName);

			//Desc.iLevel = ENUM_CLASS(eLevel);
			//
			//switch (Desc.eObjectType)
			//{
			//case OBJECTTYPE::SONORA:
			//	m_pGameInstance->Add_GameObject_ToLayer(Desc.iLevel, TEXT("Prototype_GameObject_MapObject_Sonoro")
			//		, Desc.iLevel, TEXT("Layer_Sonoro"), &Desc);
			//	break;
			//
			//case OBJECTTYPE::NONSONORA:
			//	m_pGameInstance->Add_GameObject_ToLayer(Desc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
			//		, Desc.iLevel, TEXT("Layer_NonSonoro"), &Desc);
			//	break;
			//
			//case OBJECTTYPE::NONSONORA_FLOOR:
			//	m_pGameInstance->Add_GameObject_ToLayer(Desc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
			//		, Desc.iLevel, TEXT("Layer_NonSonoro"), &Desc);
			//	break;
			//
			//default:
			//	m_pGameInstance->Clone_Prototype(Desc.iLevel, TEXT("Prototype_GameObject_MapObject")
			//		, PROTOTYPE::GAMEOBJECT, &Desc);
			//	break;
			//}

			m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType,
				Matrix = *Desc.WorldMatrix, BoundingPos = Desc.vBoundingPos, BoundingExtends = Desc.vBoundingExtends]() mutable {
					CMapObject::MAP_LOAD pDesc{};
					strcpy_s(pDesc.ModelName, ModelName.c_str());
					pDesc.iShaderPassIndex = ShaderPass;
					pDesc.eObjectType = eObjectType;
					pDesc.WorldMatrix = &Matrix;
					pDesc.iLevel = ENUM_CLASS(eLevel);
					pDesc.vBoundingPos = BoundingPos;
					pDesc.vBoundingExtends = BoundingExtends;
			
					switch (pDesc.eObjectType)
					{
					case OBJECTTYPE::SONORA:
						m_pGameInstance->Add_GameObject_ToLayer(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_Sonoro")
							, pDesc.iLevel, TEXT("Layer_Sonoro"), &pDesc);
						break;
			
					case OBJECTTYPE::NONSONORA:
						m_pGameInstance->Add_GameObject_ToLayer(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
							, pDesc.iLevel, TEXT("Layer_NonSonoro"), &pDesc);
						break;
			
					case OBJECTTYPE::NONSONORA_FLOOR:
						m_pGameInstance->Add_GameObject_ToLayer(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
							, pDesc.iLevel, TEXT("Layer_NonSonoro"), &pDesc);
						break;
			
					default:
						m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject")
							, PROTOTYPE::GAMEOBJECT, &pDesc);
						break;
					}
				});
		}
		m_pGameInstance->Wait_Thread_End();
	}
	File.close();
}

void CParser::Create_Effect(const string& strFolderPath, LEVEL eLevel)
{
    //프리팹까지 폴더경로 넣어줘야함
    _string strDefaultPath = strFolderPath;

    //파티클 VB 원형 생성
    _string strEffectPath = strDefaultPath;
    strEffectPath += "/ParticleVB/";
	for (const auto& entry : filesystem::directory_iterator(strEffectPath))
	{
		if (!entry.is_regular_file())
			continue;
		//파일 경로
		_string filePath = entry.path().string();
		//파일 이름
		_string fileName = entry.path().filename().string();
		//파일 정보
		_string extension = entry.path().extension().string();

		if (extension == ".json")
		{
			_string strParticleVBTag = entry.path().stem().string();

			Load_Particle_VB_FromJson(filePath, strParticleVBTag, eLevel);
		}

	}

    //파티클 OB 원형 생성
    strEffectPath = strDefaultPath;
    strEffectPath += "/Particle/";
    for (const auto& entry : filesystem::directory_iterator(strEffectPath))
    {
        if (entry.is_regular_file())
        {
            //파일 경로
            _string filePath = entry.path().string();
            //파일 이름
            _string fileName = entry.path().filename().string();
            //파일 정보
            _string extension = entry.path().extension().string();

            if (extension == ".json")
            {
                _string strParticleOBTag = entry.path().stem().string();

                Load_Particle_OB_FromJson(filePath, strParticleOBTag, eLevel);
            }
        }
    }

    //트레일매쉬 오브젝트 원형 생성, Dat도 만들어줘야 클론 문제없음.
    strEffectPath = strDefaultPath;
    strEffectPath += "/TrailMesh/";
    for (const auto& entry : filesystem::directory_iterator(strEffectPath))
    {
        if (entry.is_regular_file())
        {
            //파일 경로
            _string filePath = entry.path().string();
            //파일 이름
            _string fileName = entry.path().filename().string();
            //파일 정보
            _string extension = entry.path().extension().string();

            if (extension == ".json")
            {
                _string strTrailMeshTag = entry.path().stem().string();

                Load_TrailMesh_FromJson(filePath, strTrailMeshTag, eLevel);
            }
        }
    }

	strEffectPath = strDefaultPath;
	strEffectPath += "/FXRect/";
	for (const auto& entry : filesystem::directory_iterator(strEffectPath))
	{
		if (entry.is_regular_file())
		{
			//파일 경로
			_string filePath = entry.path().string();
			//파일 이름
			_string fileName = entry.path().filename().string();
			//파일 정보
			_string extension = entry.path().extension().string();

			if (extension == ".json")
			{
				_string strRectTag = entry.path().stem().string();

				Load_FXRect_FromJson(filePath, strRectTag, eLevel);
			}
		}
	}

	strEffectPath = strDefaultPath;
	strEffectPath += "/FXDecal/";
	for (const auto& entry : filesystem::directory_iterator(strEffectPath))
	{
		if (entry.is_regular_file())
		{
			//파일 경로
			_string filePath = entry.path().string();
			//파일 이름
			_string fileName = entry.path().filename().string();
			//파일 정보
			_string extension = entry.path().extension().string();
	
			if (extension == ".json")
			{
				_string strDecalTag = entry.path().stem().string();
	
				Load_FXDecal_FromJson(filePath, strDecalTag, eLevel);
			}
		}
	}

	strEffectPath = strDefaultPath;
	strEffectPath += "/FXRadial/";
	for (const auto& entry : filesystem::directory_iterator(strEffectPath))
	{
		if (entry.is_regular_file())
		{
			//파일 경로
			_string filePath = entry.path().string();
			//파일 이름
			_string fileName = entry.path().filename().string();
			//파일 정보
			_string extension = entry.path().extension().string();

			if (extension == ".json")
			{
				_string strRadialTag = entry.path().stem().string();

				Load_FXRadial_FromJson(filePath, strRadialTag, eLevel);
			}
		}
	}


    //추후 추가 될 이펙트들 더 있음. 나머진 추후 추가 예정.
}

void CParser::Create_Prefab(const string& strFolderPath, LEVEL eLevel, _int PoolingNum)
{
    //프리팹 폴더 경로 까지 지정해주면 내부에있는 프리팹들 다 읽어줌.
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            //파일 경로
            _string filePath = entry.path().string();
            //파일 이름
            _string fileName = entry.path().filename().string();
            //파일 정보
            _string extension = entry.path().extension().string();

            if (extension == ".json")
            {
                _string strPrefabTag = entry.path().stem().string();

                Load_Prefab_FromJson(filePath, strPrefabTag, eLevel, PoolingNum);
            }
        }
    }
}

void CParser::Load_Prefab_FromJson(const _string& strFilePath, const _string& strPrefabTag, LEVEL eLevel, _int PoolingNum)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json PrefabJson;
    JsonStream >> PrefabJson;
    JsonStream.close();

    CEffect_Prefab::PREFAB_DESC PrefabDesc = {};

    if (PrefabJson.contains("Prefab_Name"))
        PrefabDesc.strPrefabTag = StringToWString(PrefabJson["Prefab_Name"].get<string>());

    if (PrefabJson.contains("Children_Number"))
        PrefabDesc.ChildrenCount = PrefabJson["Children_Number"].get<_int>();

    if (PrefabJson.contains("Bone_Name"))
        PrefabDesc.strBoneTag = PrefabJson["Bone_Name"].get<string>();

    if (PrefabJson.contains("Prefab_LifeTime") && PrefabJson["Prefab_LifeTime"].is_array())
    {
        json LifeTime = PrefabJson["Prefab_LifeTime"];

        PrefabDesc.vLifeTime.x = LifeTime[0].get<_float>();
        PrefabDesc.vLifeTime.y = LifeTime[1].get<_float>();
    }

    if (PrefabJson.contains("Frames") && PrefabJson["Frames"].is_array())
    {
        for (size_t i = 0; i < PrefabDesc.ChildrenCount; i++)
        {
            CEffect_Prefab::FRAME_DESC FrameDesc = {};

            json Frame = PrefabJson["Frames"][i];

            if (Frame.contains("Children_Name"))
                FrameDesc.strChildrenTag = StringToWString(Frame["Children_Name"].get<string>());

            if (Frame.contains("Children_Type"))
                FrameDesc.eChildrenType = static_cast<EFFECT_TYPE>(Frame["Children_Type"].get<double>());

            if (Frame.contains("Activate_Time"))
                FrameDesc.fActivateTime = Frame["Activate_Time"].get<double>();

            if (Frame.contains("Offset_Size") && Frame["Offset_Size"].is_array())
            {
                json Size = Frame["Offset_Size"];

                FrameDesc.vOffsetSize.x = Size[0].get<_float>();
                FrameDesc.vOffsetSize.y = Size[1].get<_float>();
                FrameDesc.vOffsetSize.z = Size[2].get<_float>();
            }

            if (Frame.contains("Offset_Position") && Frame["Offset_Position"].is_array())
            {
                json Pos = Frame["Offset_Position"];

                FrameDesc.vOffsetPos.x = Pos[0].get<_float>();
                FrameDesc.vOffsetPos.y = Pos[1].get<_float>();
                FrameDesc.vOffsetPos.z = Pos[2].get<_float>();
            }

            if (Frame.contains("Offset_Rotation") && Frame["Offset_Rotation"].is_array())
            {
                json Rot = Frame["Offset_Rotation"];

                FrameDesc.vOffsetRot.x = Rot[0].get<_float>();
                FrameDesc.vOffsetRot.y = Rot[1].get<_float>();
                FrameDesc.vOffsetRot.z = Rot[2].get<_float>();
            }

            FrameDesc.bActivated = false;          //처음엔 기본적으로 비활성화

            PrefabDesc.FrameDesc.push_back(FrameDesc);
        }
    }

    PrefabDesc.CurrentLevel = ENUM_CLASS(eLevel);
    //프리팹 풀링 이름을 툴에서 설정한 프리팹 이름으로 할지 == Desc.PrefabName ex) test
    //아니면 json으로 저장할 때 이름으로 할지 == strPrefabTag ex)Dash_Test

    if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Prefab"),
        ENUM_CLASS(eLevel), TEXT("Layer_Effect"), PrefabDesc.strPrefabTag, PoolingNum, &PrefabDesc)))
    {
        MSG_BOX("Prefab Load Fail");
        return;
    }
}

void CParser::Load_Particle_VB_FromJson(const _string& strFilePath, const _string& VBTag, LEVEL eLevel)
{
    _string strProtoTag = "Prototype_Componenet_VIBuffer_Instance_Point_";
    strProtoTag += VBTag;

    _wstring wstrPrototTag = StringToWString(strProtoTag);

    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json ParticleVBJson;
    JsonStream >> ParticleVBJson;

    CVIBuffer_Point_Instance::POINT_INSTANCE_DESC Desc = {};

    if (ParticleVBJson.contains("NumInstance"))
        Desc.iNumInstance = ParticleVBJson["NumInstance"].get<_int>();

    if (ParticleVBJson.contains("Center") && ParticleVBJson["Center"].is_array())
    {
        json CenterJson = ParticleVBJson["Center"];
        Desc.vCenter.x = CenterJson[0].get<_float>();
        Desc.vCenter.y = CenterJson[1].get<_float>();
        Desc.vCenter.z = CenterJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Range") && ParticleVBJson["Range"].is_array())
    {
        json RangeJson = ParticleVBJson["Range"];
        Desc.vRange.x = RangeJson[0].get<_float>();
        Desc.vRange.y = RangeJson[1].get<_float>();
        Desc.vRange.z = RangeJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Size") && ParticleVBJson["Size"].is_array())
    {
        json SizeJson = ParticleVBJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Pivot") && ParticleVBJson["Pivot"].is_array())
    {
        json PivotJson = ParticleVBJson["Pivot"];
        Desc.vPivot.x = PivotJson[0].get<_float>();
        Desc.vPivot.y = PivotJson[1].get<_float>();
        Desc.vPivot.z = PivotJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Speed") && ParticleVBJson["Speed"].is_array())
    {
        json SpeedJson = ParticleVBJson["Speed"];
        Desc.vSpeed.x = SpeedJson[0].get<_float>();
        Desc.vSpeed.y = SpeedJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("LifeTime") && ParticleVBJson["LifeTime"].is_array())
    {
        json LifeTimeJson = ParticleVBJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Loop"))
        Desc.IsLoop = ParticleVBJson["Loop"].get<_bool>();

	if (ParticleVBJson.contains("SpawnBox"))
		Desc.IsSpawnBox = ParticleVBJson["SpawnBox"].get<_bool>();

	if (ParticleVBJson.contains("SpawnRing"))
		Desc.IsSpawnRing = ParticleVBJson["SpawnRing"].get<_bool>();

	if (ParticleVBJson.contains("RingAngle"))
		Desc.IsRingAngle = ParticleVBJson["RingAngle"].get<_bool>();

	if (ParticleVBJson.contains("RingAngle_Min"))
		Desc.fRmin = ParticleVBJson["RingAngle_Min"].get<_float>();

	if (ParticleVBJson.contains("RingAngle_max"))
		Desc.fRmax = ParticleVBJson["RingAngle_max"].get<_float>();

	if (ParticleVBJson.contains("DegreeAngle") && ParticleVBJson["DegreeAngle"].is_array())
	{
		json DegreeJson = ParticleVBJson["DegreeAngle"];
		Desc.fDegreeAngle.x = DegreeJson[0].get<_float>();
		Desc.fDegreeAngle.y = DegreeJson[1].get<_float>();
	}

    if (ParticleVBJson.contains("Stretch"))
        Desc.IsStretch = ParticleVBJson["Stretch"].get<_bool>();

    if (ParticleVBJson.contains("Stretch_Weight"))
        Desc.fStretchWeight = ParticleVBJson["Stretch_Weight"].get<_float>();

    if (ParticleVBJson.contains("Stretch_Range") && ParticleVBJson["Stretch_Range"].is_array())
    {
        json StrerchRangJson = ParticleVBJson["Stretch_Range"];
        Desc.fStretchRange.x = StrerchRangJson[0].get<_float>();
        Desc.fStretchRange.y = StrerchRangJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Sprite"))
        Desc.IsSprite = ParticleVBJson["Sprite"].get<_bool>();

    if (ParticleVBJson.contains("Sprite_Weight"))
        Desc.fSpriteWeight = ParticleVBJson["Sprite_Weight"].get<_float>();

    if (ParticleVBJson.contains("Sprite_DefulatSpeed"))
        Desc.fDefualtSpeed = ParticleVBJson["Sprite_DefulatSpeed"].get<_float>();

    if (ParticleVBJson.contains("Delay"))
        Desc.IsDelay = ParticleVBJson["Delay"].get<_bool>();

    if (ParticleVBJson.contains("Delay_Time") && ParticleVBJson["Delay_Time"].is_array())
    {
        json DelayTimeJson = ParticleVBJson["Delay_Time"];
        Desc.fDelay.x = DelayTimeJson[0].get<_float>();
        Desc.fDelay.y = DelayTimeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("SpreadWeight"))
        Desc.fSpreadWeight = ParticleVBJson["SpreadWeight"].get<_float>();

    if (ParticleVBJson.contains("DropWeight"))
        Desc.fDropWeight = ParticleVBJson["DropWeight"].get<_float>();

    if (ParticleVBJson.contains("RotationWeight"))
        Desc.fRotationWeight = ParticleVBJson["RotationWeight"].get<_float>();

    if (ParticleVBJson.contains("Gravity"))
        Desc.fGravity = ParticleVBJson["Gravity"].get<_float>();

    JsonStream.close();

    //읽은 정보로 원형 생성
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
        CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &Desc))))
    {
        MSG_BOX("VIBuffer_Point_Instance Load Fail");
        return;
    }
}

void CParser::Load_Particle_OB_FromJson(const _string& strFilePath, const _string& ParticleTag, LEVEL eLevel)
{
    _string strProtoTag = "Prototype_GameObject_Particle_";
    strProtoTag += ParticleTag;

    _wstring wstrPrototTag = StringToWString(strProtoTag);

    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json ParticleJson;
    JsonStream >> ParticleJson;
    JsonStream.close();

    CParticle::PARTICLE_DESC Desc = {};

    if (ParticleJson.contains("MyTag"))
        Desc.strMyTag = StringToWString(ParticleJson["MyTag"].get<_string>());

    if (ParticleJson.contains("MyType"))
        Desc.eMyType = static_cast<EFFECT_TYPE>(ParticleJson["MyType"].get<double>());

    if (ParticleJson.contains("Root"))
        Desc.IsRootOn = ParticleJson["Root"].get<_bool>();

	if (ParticleJson.contains("Pivot"))
		Desc.IsPivot = ParticleJson["Pivot"].get<_bool>();

    if (ParticleJson.contains("TextureTag"))
        Desc.strTextureTag = StringToWString(ParticleJson["TextureTag"].get<_string>());

    if (ParticleJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(ParticleJson["VIBufferTag"].get<_string>());

    if (ParticleJson.contains("ShaderPass"))
        Desc.iShaderPass = ParticleJson["ShaderPass"].get<_int>();

	if (ParticleJson.contains("MaskFlag"))
		Desc.iMaskFlag = ParticleJson["MaskFlag"].get<_int>();

    if (ParticleJson.contains("Size") && ParticleJson["Size"].is_array())
    {
        json SizeJson = ParticleJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
        Desc.vSize.z = SizeJson[2].get<_float>();
    }

    if (ParticleJson.contains("Position") && ParticleJson["Position"].is_array())
    {
        json PosJson = ParticleJson["Position"];
        Desc.vPos.x = PosJson[0].get<_float>();
        Desc.vPos.y = PosJson[1].get<_float>();
        Desc.vPos.z = PosJson[2].get<_float>();
    }

    if (ParticleJson.contains("Color") && ParticleJson["Color"].is_array())
    {
        json ColorJson = ParticleJson["Color"];
        Desc.vColor.x = ColorJson[0].get<_float>();
        Desc.vColor.y = ColorJson[1].get<_float>();
        Desc.vColor.z = ColorJson[2].get<_float>();
        Desc.vColor.w = ColorJson[3].get<_float>();
    }

    if (ParticleJson.contains("LifeTime") && ParticleJson["LifeTime"].is_array())
    {
        json LifeTimeJson = ParticleJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    if (ParticleJson.contains("Sprite"))
        Desc.IsSprite = ParticleJson["Sprite"].get<_bool>();

    if (ParticleJson.contains("Row"))
        Desc.iRows = ParticleJson["Row"].get<_int>();

    if (ParticleJson.contains("Col"))
        Desc.iCols = ParticleJson["Col"].get<_int>();

    Desc.CurrentLevel = ENUM_CLASS(eLevel);

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
        CParticle::Create(m_pDevice, m_pContext, &Desc))))
    {
        MSG_BOX("Particle Load Fail");
        return;
    }
}

void CParser::Load_TrailMesh_FromJson(const _string& strFilePath, const _string& TrailMeshTag, LEVEL eLevel)
{
    _string strProtoTag = "Prototype_GameObject_TrailMesh_";
    strProtoTag += TrailMeshTag;

    _wstring wstrPrototTag = StringToWString(strProtoTag);

    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json TrailMeshJson;
    JsonStream >> TrailMeshJson;
    JsonStream.close();

    CTrail_Mesh::TRAILMESH_DESC Desc = {};

	if (TrailMeshJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(TrailMeshJson["MyTag"].get<_string>());

	if (TrailMeshJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(TrailMeshJson["MyType"].get<double>());

	if (TrailMeshJson.contains("Root"))
		Desc.IsRootOn = TrailMeshJson["Root"].get<_bool>();

	if (TrailMeshJson.contains("TextureTag"))
		Desc.strTextureTag = StringToWString(TrailMeshJson["TextureTag"].get<_string>());

	if (TrailMeshJson.contains("ColorTextureTag"))
		Desc.strColorTextureTag = StringToWString(TrailMeshJson["ColorTextureTag"].get<_string>());

	if (TrailMeshJson.contains("DlssolveTextureTag"))
		Desc.strDissolveTextureTag = StringToWString(TrailMeshJson["DlssolveTextureTag"].get<_string>());

	if (TrailMeshJson.contains("DistortionTextureTag"))
		Desc.strDistortionTextureTag = StringToWString(TrailMeshJson["DistortionTextureTag"].get<_string>());

	if (TrailMeshJson.contains("VIBufferTag"))
		Desc.strVIBufferTag = StringToWString(TrailMeshJson["VIBufferTag"].get<_string>());

	if (TrailMeshJson.contains("ShaderPass"))
		Desc.iShaderPass = TrailMeshJson["ShaderPass"].get<_int>();

	if (TrailMeshJson.contains("SweepSpeed"))
		Desc.fSweep = TrailMeshJson["SweepSpeed"].get<_float>();

	if (TrailMeshJson.contains("SweepWitdh"))
		Desc.fSweepWitdh = TrailMeshJson["SweepWitdh"].get<_float>();

	if (TrailMeshJson.contains("SweepSoft"))
		Desc.fSoft = TrailMeshJson["SweepSoft"].get<_float>();

	if (TrailMeshJson.contains("DirFlag"))
		Desc.iDirFlag = TrailMeshJson["DirFlag"].get<_int>();

	if (TrailMeshJson.contains("MaskFloag"))		//오타있음
		Desc.iMaskFlag = TrailMeshJson["MaskFloag"].get<_int>();

	if (TrailMeshJson.contains("DissolveFlag"))
		Desc.IsDissolve = TrailMeshJson["DissolveFlag"].get<_bool>();

	if (TrailMeshJson.contains("DistortionFlag"))
		Desc.IsDistortion = TrailMeshJson["DistortionFlag"].get<_bool>();

	if (TrailMeshJson.contains("DistortionWeight"))
		Desc.fDistortionWeight = TrailMeshJson["DistortionWeight"].get<_float>();

	if (TrailMeshJson.contains("ColorSpeed"))
		Desc.fColorSpeed = TrailMeshJson["ColorSpeed"].get<_float>();

	if (TrailMeshJson.contains("MaskSpeed"))
		Desc.fMaskSpeed = TrailMeshJson["MaskSpeed"].get<_float>();

	if (TrailMeshJson.contains("Alpha"))
		Desc.fAlpha = TrailMeshJson["Alpha"].get<_float>();

	if (TrailMeshJson.contains("ColorGain"))
		Desc.fColorGain = TrailMeshJson["ColorGain"].get<_float>();

	if (TrailMeshJson.contains("ColorGamma"))
		Desc.fColorGamma = TrailMeshJson["ColorGamma"].get<_float>();


	if (TrailMeshJson.contains("Size") && TrailMeshJson["Size"].is_array())
	{
		json SizeJson = TrailMeshJson["Size"];
		Desc.vSize.x = SizeJson[0].get<_float>();
		Desc.vSize.y = SizeJson[1].get<_float>();
		Desc.vSize.z = SizeJson[2].get<_float>();
	}

	if (TrailMeshJson.contains("Position") && TrailMeshJson["Position"].is_array())
	{
		json PosJson = TrailMeshJson["Position"];
		Desc.vPos.x = PosJson[0].get<_float>();
		Desc.vPos.y = PosJson[1].get<_float>();
		Desc.vPos.z = PosJson[2].get<_float>();
	}

	if (TrailMeshJson.contains("LifeTime") && TrailMeshJson["LifeTime"].is_array())
	{
		json LifeTimeJson = TrailMeshJson["LifeTime"];
		Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
		Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
	}

    Desc.CurrentLevel = ENUM_CLASS(eLevel);

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
        CTrail_Mesh::Create(m_pDevice, m_pContext, &Desc))))
    {
        MSG_BOX("Trail_Mesh Load Fail");
        return;
    }
}

void CParser::Load_FXRect_FromJson(const _string& strFilePath, const _string& RectTag, LEVEL eLevel)
{
	_string strProtoTag = "Prototype_GameObject_FXRect_";
	strProtoTag += RectTag;

	_wstring wstrPrototTag = StringToWString(strProtoTag);
	
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json RectJson;
	JsonStream >> RectJson;
	JsonStream.close();

	CEffect_Rect::FXRECT_DESC Desc = {};

	if (RectJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(RectJson["MyTag"].get<_string>());

	if (RectJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(RectJson["MyType"].get<double>());

	if (RectJson.contains("Root"))
		Desc.IsRootOn = RectJson["Root"].get<_bool>();

	if (RectJson.contains("TextureTag"))
		Desc.strTextureTag = StringToWString(RectJson["TextureTag"].get<_string>());

	if (RectJson.contains("ShaderPass"))
		Desc.iShaderPass = RectJson["ShaderPass"].get<_int>();

	if (RectJson.contains("MaskFlag"))
		Desc.iMaskFlag = RectJson["MaskFlag"].get<_int>();

	if (RectJson.contains("SweepSpeed"))
		Desc.fSweepSpeed = RectJson["SweepSpeed"].get<_float>();

	if (RectJson.contains("SweepSoft"))
		Desc.fSoft = RectJson["SweepSoft"].get<_float>();

	if (RectJson.contains("SizeX"))
		Desc.fXSize = RectJson["SizeX"].get<_float>();

	if (RectJson.contains("SizeY"))
		Desc.fYSize = RectJson["SizeY"].get<_float>();

	if (RectJson.contains("Row"))
		Desc.iRows = RectJson["Row"].get<_int>();

	if (RectJson.contains("Col"))
		Desc.iCols = RectJson["Col"].get<_int>();

	if (RectJson.contains("Sprite"))
		Desc.IsSprite = RectJson["Sprite"].get<_bool>();

	if (RectJson.contains("Position") && RectJson["Position"].is_array())
	{
		json PosJson = RectJson["Position"];
		Desc.vPos.x = PosJson[0].get<_float>();
		Desc.vPos.y = PosJson[1].get<_float>();
		Desc.vPos.z = PosJson[2].get<_float>();
	}

	if (RectJson.contains("LifeTime") && RectJson["LifeTime"].is_array())
	{
		json LifeTimeJson = RectJson["LifeTime"];
		Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
		Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
	}

	if (RectJson.contains("Color") && RectJson["Color"].is_array())
	{
		json ColorJson = RectJson["Color"];
		Desc.vColor.x = ColorJson[0].get<_float>();
		Desc.vColor.y = ColorJson[1].get<_float>();
		Desc.vColor.z = ColorJson[2].get<_float>();
		Desc.vColor.w = ColorJson[3].get<_float>();
	}

	Desc.CurrentLevel = ENUM_CLASS(eLevel);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
		CEffect_Rect::Create(m_pDevice, m_pContext, &Desc))))
	{
		MSG_BOX("Effect_Rect Load Fail");
		return;
	}
}

void CParser::Load_FXDecal_FromJson(const _string& strFilePath, const _string& DecalTag, LEVEL eLevel)
{
	_string strProtoTag = "Prototype_GameObject_FXDecal_";
	strProtoTag += DecalTag;

	_wstring wstrPrototTag = StringToWString(strProtoTag);
	
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json DecalJson;
	JsonStream >> DecalJson;
	JsonStream.close();

	CEffect_Decal::DECAL_DESC Desc = {};

	if (DecalJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(DecalJson["MyTag"].get<_string>());

	if (DecalJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(DecalJson["MyType"].get<double>());

	if (DecalJson.contains("DecalTag"))
		Desc.wstrDecalTag = StringToWString(DecalJson["DecalTag"].get<_string>());

	if (DecalJson.contains("LifeTime"))
		Desc.LifeTime = DecalJson["LifeTime"].get<_float>();

	if (DecalJson.contains("Color") && DecalJson["Color"].is_array())
	{
		json ColorJson = DecalJson["Color"];
		Desc.vColor.x = ColorJson[0].get<_float>();
		Desc.vColor.y = ColorJson[1].get<_float>();
		Desc.vColor.z = ColorJson[2].get<_float>();
		Desc.vColor.w = ColorJson[3].get<_float>();
	}

	Desc.CurrentLevel = ENUM_CLASS(eLevel);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
		CEffect_Decal::Create(m_pDevice, m_pContext, &Desc))))
	{
		MSG_BOX("Effect_Rect Load Fail");
		return;
	}
}

void CParser::Load_FXDecal_Data_FromJson(const _string& strFilePath)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json DecalDataJson;
	JsonStream >> DecalDataJson;
	JsonStream.close();

	_wstring DecalDataTag = {};
	_int iTextureCount = {};
	_float3 EmissiveLuminance = {};

	if (DecalDataJson.contains("DecalTag"))
		DecalDataTag = StringToWString(DecalDataJson["DecalTag"].get<_string>());

	if (DecalDataJson.contains("TextureCount"))
		iTextureCount = DecalDataJson["TextureCount"].get<_int>();

	if (DecalDataJson.contains("EmissiveLuminance") && DecalDataJson["EmissiveLuminance"].is_array())
	{
		json Emissive = DecalDataJson["EmissiveLuminance"];
		EmissiveLuminance.x = Emissive[0].get<_float>();
		EmissiveLuminance.y = Emissive[1].get<_float>();
		EmissiveLuminance.z = Emissive[2].get<_float>();
	}

	const _tchar* DecalTexturePath[ENUM_CLASS(TEXTURETYPE::END)] = {};

	if (DecalDataJson.contains("Textures") && DecalDataJson["Textures"].is_array())
	{
		for (size_t i = 0; i < iTextureCount; i++)
		{
			_wstring strTexturePath = {};
			_int iType = {};
			_tchar TextPath[MAX_PATH] = {};

			json Texture = DecalDataJson["Textures"][i];

			if (Texture.contains("Texture_Type"))
				iType = Texture["Texture_Type"].get<_int>();

			if (Texture.contains("Texture_Path"))
				strTexturePath = StringToWString(Texture["Texture_Path"].get<_string>());
			// JSON 읽어서, 미리 데칼 매니저에 바인딩 

			wcscpy_s(TextPath, strTexturePath.c_str());

			DecalTexturePath[iType] = TextPath;
		}
	}

	m_pGameInstance->Add_Decal(DecalDataTag, DecalTexturePath, EmissiveLuminance);
}

void CParser::Load_FXRadial_FromJson(const _string& strFilePath, const _string& RadialTag, LEVEL eLevel)
{
	_string strProtoTag = "Prototype_GameObject_FXRadial_";
	strProtoTag += RadialTag;

	_wstring wstrPrototTag = StringToWString(strProtoTag);

	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json RadialJson;
	JsonStream >> RadialJson;
	JsonStream.close();

	CEffect_Radial::RADIAL_DESC Desc = {};

	if (RadialJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(RadialJson["MyTag"].get<_string>());

	if (RadialJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(RadialJson["MyType"].get<double>());

	if (RadialJson.contains("LifeTime"))
		Desc.fLifeTime = RadialJson["LifeTime"].get<_float>();

	if (RadialJson.contains("IntensityRange"))
		Desc.IntensityRange = RadialJson["IntensityRange"].get<_float>();

	if (RadialJson.contains("Center") && RadialJson["Center"].is_array())
	{
		json CenterJson = RadialJson["Center"];
		Desc.Center.x = CenterJson[0].get<_float>();
		Desc.Center.y = CenterJson[1].get<_float>();
	}

	if (RadialJson.contains("DistanceRange") && RadialJson["DistanceRange"].is_array())
	{
		json DistanceRangeJson = RadialJson["DistanceRange"];
		Desc.DistanceRange.x = DistanceRangeJson[0].get<_float>();
		Desc.DistanceRange.y = DistanceRangeJson[1].get<_float>();
	}

	Desc.CurrentLevel = ENUM_CLASS(eLevel);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrPrototTag,
		CEffect_Radial::Create(m_pDevice, m_pContext, &Desc))))
	{
		MSG_BOX("Effect_Rect Load Fail");
		return;
	}
}

void CParser::Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png" || extension == ".Png")
            {
                //텍스처 파일이름만 추출
                _string strTextureTag = entry.path().stem().string();

                //텍스처 파일경로
                _wstring wstrFilePath = StringToWString(filePath);

                _wstring wstrDefaultTag = TEXT("Prototype_Component_Texture_");
                wstrDefaultTag += StringToWString(strTextureTag);

               if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrDefaultTag,
                    CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1))))
               {
                   MSG_BOX("Texture Load Fail");
                   return;
               }
            }
        }
    }
}

void CParser::Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".Dat" || extension == ".dat")
            {
                //파일이름만 추출
                _string strMeshTag = entry.path().stem().string();

                _wstring wstrDefaultTag = TEXT("Prototype_Component_Mesh_");
                wstrDefaultTag += StringToWString(strMeshTag);

                _float fSize = 0.01f;
                _fmatrix DefualtMatrix = XMMatrixScaling(fSize, fSize, fSize);

                if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), wstrDefaultTag,
                    CVIBuffer_Mesh::Create(m_pDevice, m_pContext, filePath.c_str(),DefualtMatrix))))
                {
                    MSG_BOX("VIBuffer_Mesh Load Fail");
                    return;
                }
            }
        }
    }
}

void CParser::Load_FXDecal_Data_FromFolder(const string& strFolderPath)
{
	for (const auto& entry : filesystem::directory_iterator(strFolderPath))
	{
		if (entry.is_regular_file())
		{
			_string filePath = entry.path().string();
			_string fileName = entry.path().filename().string();
			_string extension = entry.path().extension().string();

			if (extension == ".json")
			{

				Load_FXDecal_Data_FromJson(filePath);
			}
		}
	}
}



const vector<vector<_string>>& CParser::Load_CSV(const _char* pFilePath)
{
	ifstream InputFile(pFilePath);

	_string strLine;

	m_Data.clear();

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

void CParser::Load_Sequence(const _char* pFolderPath)
{
	for (const auto& entry : filesystem::directory_iterator(pFolderPath))
	{
		if (entry.is_regular_file())
		{
			_string filePath = entry.path().string();
			_string fileName = entry.path().stem().string();
			

			ifstream InputFile(filePath);
			json SequeceJson;
			InputFile >> SequeceJson;

			CSequence::SEQUENCE_DESC SequenceDesc = {};
			SequenceDesc.fTrackPerSec = SequeceJson["TrackPerSec"];
			SequenceDesc.fDuration = SequeceJson["Duration"];

			vector<SEQUENCE_ITEM_INFO> ItemInfos;
			vector<SEQUENCE_ITEM_DATA*> ItemDatas;

			for (auto& ItemJson : SequeceJson["Item"])
			{
				// Info
				SEQUENCE_ITEM_INFO Info = {};
				Info.fStartFrame = ItemJson["FrameStart"];
				Info.fEndFrame = ItemJson["FrameEnd"];
				Info.strItemTag = StringToWString(ItemJson["Tag"]);

				// Data
				_string strItemType = ItemJson["Type"];
				if ("Actor" == strItemType)
				{

				}
				else if ("Scene" == strItemType)
				{
					Info.eType = ITEM_TYPE::SCENE;
					Load_Scene(ItemJson, ItemDatas);
				}

				ItemInfos.push_back(Info);
			}

			m_pGameInstance->Register_Sequence(StringToWString(fileName), ItemInfos, ItemDatas, &SequenceDesc);

			InputFile.close();
		}
	}
}

void CParser::Load_Scene(json& ItemJson, vector<SEQUENCE_ITEM_DATA*>& ItemDatas)
{
	vector<SCENE_CAMERA_FRAME> Frames;

	for (auto& FrameJson : ItemJson["Frame"])
	{
		SCENE_CAMERA_FRAME Frame = {};
		Frame.fStartFrame = FrameJson["Start"];
		Frame.fFovy = FrameJson["FOV"];
		Frame.isLerp = FrameJson["Lerp"];

		Frame.vQuaternion = _float4(FrameJson["Quaternion"][0], FrameJson["Quaternion"][1], FrameJson["Quaternion"][2], FrameJson["Quaternion"][3]);
		Frame.vPosition = _float3(FrameJson["Position"][0], FrameJson["Position"][1], FrameJson["Position"][2]);

		Frames.push_back(Frame);
	}

	SQ_CAMERA_DATA* SceneData = new SQ_CAMERA_DATA(ItemJson["FrameStart"], ItemJson["FrameEnd"], ItemJson["TrackPerSec"], Frames);
	ItemDatas.push_back(SceneData);
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
