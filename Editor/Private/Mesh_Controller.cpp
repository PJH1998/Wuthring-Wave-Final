#include "EditorPch.h"
#include "Mesh_Controller.h"

CMesh_Controller::CMesh_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CMesh_Controller::Initialize()
{
    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/EffectMesh/Texture");
    Load_AllMeshDatFromFolder("../../Client/Bin/Resource/Effect/EffectMesh/Dat");

    return S_OK;
}

void CMesh_Controller::Update()
{
    EffectMesh_Tab();
}

void CMesh_Controller::Render()
{

}

void CMesh_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png")
            {
                MESH_TEXTURE Desc{};
                CTexture* pTexture = {};

                // 확장자 제외한 파일명
                _string strTextureTag = entry.path().stem().string();

                //파일명으로 텍스처 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //파일명으로 텍스처 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                //파일경로 wstring 변환
                _wstring wstrFilePath = StringToWString(filePath);
    
                //텍스처 컴포넌트 생성
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                //생성한 텍스처 주소 등록, 미리보기 띄울려면 주소로 SRV가져와야해서 저장해줘야함.
                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

//이름만 읽어서 리스트박스에 이름 띄우는 용도로만 사용하자. 
void CMesh_Controller::Load_AllMeshDatFromFolder(const _string& strFolderPath)
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
                MESH_TAG Desc = {};
                CVIBuffer_Mesh::MESH_FXINSTANCE_DESC FXMeshDesc = {};

                // 확장자 제외한 파일명
                _string strMeshTag = entry.path().stem().string();

                //파일명으로 매쉬 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strMeshTag.c_str());

                //파일명으로 매쉬버퍼 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_EffectMesh_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strMeshTag, MAX_PATH);

                _string DefaultPath = "/Resource/Effect/EffectMesh/Dat/";

                DefaultPath += fileName;

                strcpy_s(Desc.szDatPath, sizeof(Desc.szDatPath), DefaultPath.c_str());

                ////매쉬버퍼 컴포넌트 생성
                //_fmatrix DefualtMatrix = XMMatrixIdentity();

                //FXMeshDesc.vSize = _float2(1.f, 1.f);
                //FXMeshDesc.iNumInstance = 1;

                //m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strMeshTag,
                //   CVIBuffer_Mesh::Create(m_pDevice, m_pContext, filePath.c_str(), DefualtMatrix, &FXMeshDesc));


                //m_tVBMeshDesc.emplace(strMeshTag, FXMeshDesc);
                m_MeshVBTag.push_back(Desc);
            }
        }
    }
}

void CMesh_Controller::EffectMesh_Tab()
{
    if (m_bSelectedMesh)
    {
        if (ImGui::Begin("FXMesh Info"))
        {

            //파티클 설정값 VIBuffer
            if (ImGui::CollapsingHeader("VIBuffer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /////////////////////////////////////// 체크박스
                ImGui::Checkbox("Loop", &(m_pSelectedVBFXDesc->IsLoop));

                if (ImGui::Checkbox("SpawnRing", &(m_pSelectedVBFXDesc->IsSpawnRing)))
                {
                    //선택됐으니 다른 얘들 꺼주자
                    m_pSelectedVBFXDesc->IsSpawnBox = false;
                }

                ImGui::Checkbox("RingAngle", &(m_pSelectedVBFXDesc->IsRingAngle));

                if (ImGui::Checkbox("SpawnBox", &(m_pSelectedVBFXDesc->IsSpawnBox)))
                {
                    m_pSelectedVBFXDesc->IsSpawnRing = false;
                }
                ImGui::Separator();
                ///////////////////////////////////////

                ImGui::Text("NumInstance");
                ImGui::PushItemWidth(100);
                ImGui::DragInt("##NumInstance", (int*)&(m_pSelectedVBFXDesc->iNumInstance));
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(200);

                /////////////////////////////////////// 가중치 설정
                ImGui::Separator();
                ImGui::Text("SpreadWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##SpreadW", &(m_pSelectedVBFXDesc->fSpreadWeight), 0.1f, 0.f, 1.f);

                ImGui::Text("DropWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##DorpW", &(m_pSelectedVBFXDesc->fDropWeight), 0.1f, 0.f, 1.f);

                ImGui::Text("RotationWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##RotationW", &(m_pSelectedVBFXDesc->fRotationWeight), 0.1f, 0.f, 1.f);

                ImGui::PopItemWidth();
                ImGui::Separator();
                ///////////////////////////////////////
         
                if (m_pSelectedVBFXDesc->IsSpawnRing)
                {
                    /////////////////////////////////////// 링 스폰시 설정값
                    ImGui::Text("RMin/RMax");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RMin", &(m_pSelectedVBFXDesc->fRmin));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RMax", &(m_pSelectedVBFXDesc->fRmax));
                    ImGui::PopItemWidth();

                    ImGui::Text("Degree Start/End");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##DegreeX", &(m_pSelectedVBFXDesc->fDegreeAngle.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##DegreeY", &(m_pSelectedVBFXDesc->fDegreeAngle.y));
                    ImGui::PopItemWidth();
                    ImGui::Separator();
                    ///////////////////////////////////////
                }

                ImGui::Checkbox("InWard", &(m_pSelectedVBFXDesc->IsInWard));
                /////////////////////////////////////// 방향 설정

                ImGui::Text("Pitch");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##Pitch", &(m_pSelectedVBFXDesc->fPitch));
                ImGui::PopItemWidth();
                ImGui::Separator();
                ///////////////////////////////////////
        
                ImGui::Text("Center");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##CenterX", &(m_pSelectedVBFXDesc->vCenter.x));
                ImGui::SameLine();
                ImGui::InputFloat("##CenterY", &(m_pSelectedVBFXDesc->vCenter.y));
                ImGui::SameLine();
                ImGui::InputFloat("##CenterZ", &(m_pSelectedVBFXDesc->vCenter.z));
                ImGui::PopItemWidth();

                ImGui::Text("Size");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##SizeX", &(m_pSelectedVBFXDesc->vSize.x));
                ImGui::SameLine();
                ImGui::InputFloat("##SizeY", &(m_pSelectedVBFXDesc->vSize.y));
                ImGui::PopItemWidth();

                ImGui::Text("Range");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##RangeX", &(m_pSelectedVBFXDesc->vRange.x));
                ImGui::SameLine();
                ImGui::InputFloat("##RangeY", &(m_pSelectedVBFXDesc->vRange.y));
                ImGui::SameLine();
                ImGui::InputFloat("##RangeZ", &(m_pSelectedVBFXDesc->vRange.z));
                ImGui::PopItemWidth();

                ImGui::Text("Pivot");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##PivotX", &(m_pSelectedVBFXDesc->vPivot.x));
                ImGui::SameLine();
                ImGui::InputFloat("##PivotY", &(m_pSelectedVBFXDesc->vPivot.y));
                ImGui::SameLine();
                ImGui::InputFloat("##PivotZ", &(m_pSelectedVBFXDesc->vPivot.z));
                ImGui::PopItemWidth();

                ImGui::Text("Speed");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##SppedX", &(m_pSelectedVBFXDesc->vSpeed.x));
                ImGui::SameLine();
                ImGui::InputFloat("##SppedY", &(m_pSelectedVBFXDesc->vSpeed.y));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##LifeTimeX", &(m_pSelectedVBFXDesc->fLifeTime));
        /*        ImGui::SameLine();
                ImGui::InputFloat("##LifeTimeY", &(m_pSelectedVBFXDesc->vLifeTime.y));*/
                ImGui::PopItemWidth();
            }

            if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /*      ImGui::Checkbox("Spread", &(m_pSelectedEffectMeshDesc->bSpread));
                      ImGui::Checkbox("Drop", &(m_pSelectedEffectMeshDesc->bDrop));*/
                ImGui::Checkbox("Root", &(m_pSelectedEffectMeshDesc->IsRootOn));

                ImGui::Text("Size");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleSizeX", &(m_pSelectedEffectMeshDesc->vSize.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleSizeY", &(m_pSelectedEffectMeshDesc->vSize.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleSizeZ", &(m_pSelectedEffectMeshDesc->vSize.z));
                ImGui::PopItemWidth();

                ImGui::Text("Position");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticlePosX", &(m_pSelectedEffectMeshDesc->vPos.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticlePosY", &(m_pSelectedEffectMeshDesc->vPos.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticlePosZ", &(m_pSelectedEffectMeshDesc->vPos.z));
                ImGui::PopItemWidth();

                ImGui::Text("Color");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleColorX", &(m_pSelectedEffectMeshDesc->vColor.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleColorY", &(m_pSelectedEffectMeshDesc->vColor.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleColorZ", &(m_pSelectedEffectMeshDesc->vColor.z));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleLifeTimeX", &(m_pSelectedEffectMeshDesc->vLifeTime.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleLifeTimeY", &(m_pSelectedEffectMeshDesc->vLifeTime.y));
                ImGui::PopItemWidth();
            }
            ImGui::End();
        }
    }

}

void CMesh_Controller::EffectMesh_Base_Tab(CEffect_Mesh::EFFECTMESH_DESC& tEffectMeshDesc, _bool& IsCreate)
{
    //프리팹이 클론으로 자식을 생성할 정보를 넘겨줘야함.
    //일단 기본베이스로 생성할 수 있게 해주자.
    //이펙트매쉬 (오브젝트)가 가져야할 정보가 생각보다 많을 거 같음. ex) 매쉬의 움직임 정보, 텍스처 정보 (여러개), 패스정보, 라이프타임 등등..

    //리스트박스로 어떤 매쉬 컴포넌트를 가질건지.
    //텍스처 미리보기로 어떤 디퓨즈 텍스처(보통 색상일듯?) 가질건지만 일단 설정해서 기본베이스로 만들 수 있게?

    if (ImGui::Begin("Mesh Base"))
    {
        vector<const _char*> szMeshTag = {};

       for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
       {
            szMeshTag.push_back(iter->szName);
        }

        //매쉬 리스트박스 띄우기
        if (ImGui::ListBox("Effect Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
        {
            //선택된 매쉬태그 임시저장? 
            m_bMeshVBTag = true;
        }

        //매쉬 기본 색상 텍스처 설정
        if (ImGui::BeginCombo("Texture", "")) {
            for (size_t i = 0; i < m_Textures.size(); i++)
            {
                bool IsSelected = (m_iSelectedTexture == i);
                if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                    m_iSelectedTexture = i;

                if (IsSelected)
                    ImGui::SetItemDefaultFocus();

            }

            ImGui::EndCombo();
        }

        //Root 설정
        if(ImGui::Checkbox("Root", &m_IsRoot))

        ImGui::Separator();
        if (m_iSelectedTexture >= 0) {
            ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

        if (m_bTagFlag && m_bMeshVBTag) //이펙트 컨트롤러가 설정해준 이름값이 있고, 선택한 매쉬버퍼가 있어야지만 생성할 수 있게.
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar EffectMeshTag[MAX_PATH] = {};
                CEffect_Mesh::EFFECTMESH_DESC EffectMeshDesc{};
                CVIBuffer_Mesh::MESH_FXINSTANCE_DESC VBFXMhesDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_EffectMeshTag, strlen(m_EffectMeshTag), EffectMeshTag, MAX_PATH);

                //EffectMeshDesc.strMyTag = EffectMeshTag;

                //이펙트 매쉬 이름 및 클론할 컴포넌트 이름들
                EffectMeshDesc.strMyTag = EffectMeshTag;
                EffectMeshDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                EffectMeshDesc.strVIBufferTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;

                //이펙트매쉬(오브젝트)가 가질 디폴트 설정값.
                EffectMeshDesc.vLifeTime.y = 10.f;
                EffectMeshDesc.vPos = _float3(0.f, 0.f, 0.f);
                EffectMeshDesc.vSize = _float3(0.5f, 0.5f, 0.5f);
                EffectMeshDesc.fShaderPass = 0;

                //인스턴싱매쉬 디폴트 설정값. 여기서 미리 원형 생성을 해줘야함.
                _fmatrix DefualtMatrix = XMMatrixIdentity();
                VBFXMhesDesc.vSize = _float2(1.f, 1.f);
                VBFXMhesDesc.iNumInstance = 1;
                
                _char szDatPath[MAX_PATH] = {};

                strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin");
                strcat_s(szDatPath, sizeof(szDatPath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                //원형 생성
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag,
                CVIBuffer_Mesh::Create(m_pDevice, m_pContext, szDatPath, DefualtMatrix, &VBFXMhesDesc));
                
                //원형이 읽은 Dat 경로 VB에 저장해줘야할거 같음.
                strcpy_s(VBFXMhesDesc.DatFilePath, sizeof(VBFXMhesDesc.DatFilePath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                ////Desc에 VBMesh 이름 저장?
                //_tchar strFXMehsTag[MAX_PATH] = {};
                //MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_MeshVBTag[m_iSelectedMeshVBTag].szName, strlen(m_MeshVBTag[m_iSelectedMeshVBTag].szName), strFXMehsTag, MAX_PATH);
               
                m_tVBMeshDesc.emplace(EffectMeshTag, VBFXMhesDesc);

                if (m_IsRoot)
                {
                    EffectMeshDesc.IsRootOn = true;
                    m_IsRoot = false;
                }

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tEffectMeshDesc.emplace(EffectMeshTag, EffectMeshDesc);

                tEffectMeshDesc = EffectMeshDesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
                m_EffectMeshTag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }
       
        ImGui::End();
    }

}

void CMesh_Controller::Set_EffectMeshTag(const _char* szEffectMeshTag)
{
    strcat_s(m_EffectMeshTag, szEffectMeshTag);

    m_bTagFlag = true;
}

void CMesh_Controller::UpdateSelected_FXMeshFormTag(_wstring FXMeshTag)
{
    auto iterEffectMeshDesc = m_tEffectMeshDesc.find(FXMeshTag);
    
    if (iterEffectMeshDesc == m_tEffectMeshDesc.end())
        m_pSelectedEffectMeshDesc = nullptr;
    else
        m_pSelectedEffectMeshDesc = &iterEffectMeshDesc->second;

    auto iterVBFXDesc = m_tVBMeshDesc.find(FXMeshTag);
    
    if (iterVBFXDesc == m_tVBMeshDesc.end())
        m_pSelectedVBFXDesc = nullptr;
    else
        m_pSelectedVBFXDesc = &iterVBFXDesc->second;

    if (m_pSelectedEffectMeshDesc != nullptr)
        m_bSelectedMesh = true;

}

CEffect_Mesh::EFFECTMESH_DESC* CMesh_Controller::Get_EffectMeshDesc(_wstring& EffectMeshTag)
{
    auto iter = m_tEffectMeshDesc.find(EffectMeshTag);

    if (iter == m_tEffectMeshDesc.end())
        return nullptr;

    return &iter->second;
}

CVIBuffer_Mesh::MESH_FXINSTANCE_DESC* CMesh_Controller::Get_VBMeshDesc(_wstring& VBMesTag)
{
    auto iter = m_tVBMeshDesc.find(VBMesTag);

    if (iter == m_tVBMeshDesc.end())
        return nullptr;

    return &iter->second;
}

void CMesh_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterEffectMeshDesc = m_tEffectMeshDesc.find(DescTag);

    if (iterEffectMeshDesc != m_tEffectMeshDesc.end())
    {
        //혹시 같은 이름으로 다시 만들어지는거 대비해서 지워줘야할거 같음.
        m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), iterEffectMeshDesc->second.strVIBufferTag);

        m_tEffectMeshDesc.erase(iterEffectMeshDesc);
    }

    auto iterVBFXDesc = m_tVBMeshDesc.find(DescTag);

    if (iterVBFXDesc != m_tVBMeshDesc.end())
    {
        m_tVBMeshDesc.erase(iterVBFXDesc);
    }

    //초기화
    m_bSelectedMesh = false;
    m_pSelectedEffectMeshDesc = nullptr;
    m_pSelectedVBFXDesc = nullptr;
}

CMesh_Controller* CMesh_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMesh_Controller* pInstance = new CMesh_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CMesh_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMesh_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    //for (auto& pTexture : m_Textures)
    //    Safe_Release(pTexture.pTexture);
    m_Textures.clear();
}