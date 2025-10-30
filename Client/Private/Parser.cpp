#include "ClientPch.h"
#include "Parser.h"
#include "MapObject.h"
#include "Effect_Prefab.h"
#include "Trail_Mesh.h"
#include "Particle.h"


CParser::CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance{ CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CParser::Create_Map_Model(const _char* pFilePath, LEVEL eLevel)
{
    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    _string PasingDir = FileDir;

    Read_Map_Prototype(PasingDir, eLevel);
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

                if (entry2.path().string().find(ModelName) == std::string::npos)
                    continue;

                if (entry2.path().extension() != ".dat")
                    continue;

                _string Path = entry2.path().string();
                _string Prototype = entry2.path().stem().string();
                //파서 수정중
                if(entry2.path().string().find("Instance") == std::string::npos)
                {
                    m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(Prototype), ModelPath = Path]() {
                        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName + StringToWString(Prototype),
                            CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, ModelPath.c_str()))))
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
                //프로토타입 생성
            }
        }
        File.close();
    }
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
                _string strParticleVBTag = entry.path().stem().string();

                Load_Particle_VB_FromJson(filePath, strParticleVBTag, eLevel);
            }
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

    //추후 추가 될 이펙트들 더 있음. 나머진 추후 추가 예정.
}

void CParser::Create_Prefab(const string& strFolderPath, LEVEL eLevel)
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

                Load_Prefab_FromJson(filePath, strPrefabTag, eLevel);
            }
        }
    }
}

void CParser::Load_Prefab_FromJson(const _string& strFilePath, const _string& strPrefabTag, LEVEL eLevel)
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

            FrameDesc.bActivated = false;          //처음엔 기본적으로 비활성화

            PrefabDesc.FrameDesc.push_back(FrameDesc);
        }
    }

    PrefabDesc.CurrentLevel = ENUM_CLASS(eLevel);
    //프리팹 풀링 이름을 툴에서 설정한 프리팹 이름으로 할지 == Desc.PrefabName ex) test
    //아니면 json으로 저장할 때 이름으로 할지 == strPrefabTag ex)Dash_Test

    if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Prefab"),
        ENUM_CLASS(eLevel), TEXT("Layer_Effect"), PrefabDesc.strPrefabTag, 3, &PrefabDesc)))
    {
        MSG_BOX("Prefab Load Fail");
        return;
    }
    //개수 설정은 몇개로 ?
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

    if (ParticleJson.contains("TextureTag"))
        Desc.strTextureTag = StringToWString(ParticleJson["TextureTag"].get<_string>());

    if (ParticleJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(ParticleJson["VIBufferTag"].get<_string>());

    if (ParticleJson.contains("ShaderPass"))
        Desc.iShaderPass = ParticleJson["ShaderPass"].get<_int>();

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

    if (TrailMeshJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(TrailMeshJson["VIBufferTag"].get<_string>());

    if (TrailMeshJson.contains("ShaderPass"))
        Desc.iShaderPass = TrailMeshJson["ShaderPass"].get<_int>();

    if (TrailMeshJson.contains("SweepSpeed"))
        Desc.fSweep = TrailMeshJson["SweepSpeed"].get<_float>();

    if (TrailMeshJson.contains("SweepWitdh"))
        Desc.fSweepWitdh = TrailMeshJson["SweepWitdh"].get<_float>();

    if (TrailMeshJson.contains("DirFlag"))
        Desc.iDirFlag = TrailMeshJson["DirFlag"].get<_int>();

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
