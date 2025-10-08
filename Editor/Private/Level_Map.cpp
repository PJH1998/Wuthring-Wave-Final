#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"

CLevel_Map::CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Map::Initialize()
{
    if (FAILED(Ready_Static_Component()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Map"));
    Menu_Select();

    switch (m_eMenu)
    {
    case Editor::CLevel_Map::MENU_OBJECT:
        Menu_Object();
        break;

    case Editor::CLevel_Map::MENU_RANDSCAPE:
        Menu_RandSacpe();
        break;

    case Editor::CLevel_Map::MENU_LIGHT:
        Menu_Light();
        break;
    }
}

void CLevel_Map::Render()
{

}

void CLevel_Map::Menu_Select()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Ojbect")) {
            m_eMenu = MENU_OBJECT;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("RandScape")) {
            m_eMenu = MENU_RANDSCAPE;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Light")) {
            m_eMenu = MENU_LIGHT;
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void CLevel_Map::Menu_Object()
{
    ImGui::Begin("Menu_Object");
    ImGui::End();
}

void CLevel_Map::Menu_RandSacpe()
{
    ImGui::Begin("Menu_RandScape");

    //풀떼기들은 플레이어랑 가까이 있을 때 플레이어를 중점으로 옆으로 누움. 누운 상태로 바람에 흔들림.
    //플레이어랑 거의 겹친 풀떼기들은 Clip되는듯. 안보임.
    //움직일 때 플레이어 발바닥에 발자국 데칼 생김. 마스킹 이미지 같은 거로 하는듯?
    //그림자 진 곳이든 아닌 곳이든 똑같이 어두움. 무조건 마스킹.

    // 점프는 발자국은 안생기지만 뛸 때와 착지할 때 풀떼기가 심하게 흔들림.(어떻게 함?)
    //벽에서 달리기 할 때 발 위치에 발자국 데칼 대신 이펙트가 생김.

    //맵에 깔려있는 아이템을 먹을 때는 바닥에 나뭇잎 흔들리는 이펙트 생기면서 사라짐.
    //근처에 먹을 수 있는(상호작용 가능한 아이템이 있으면 UI 생성. 일정 주기마다 겉부분이 빛남.
    //바닥 풀떼기 말고 키 큰 풀떼기들이랑 몸 비빌 때 소리 나야함.(콜라이더?) 얘네도 똑같이 플레이어 위치에 맞춰서 흔들리는듯.

    //위치에 따라 디렉셔널라이트 디퓨즈 색이 바뀌는듯? -> 그냥 메쉬가 다른 거일 수도

    //바람에 흔들리는 방향은 모두 같은 방향인?듯 

    //돌은 ㅋㅋ 그냥 에셋스토어에서 떼온듯ㅋㅋ
    //특정 위치에 따라 풀떼기의 색이 ㅈ금씩 바뀜.
    //광물류는 멀리 있으면 빤짝빤짝댐.


    //길찾기. 가만히 있으면 목표 위치로 일렁이는 이펙트 생기면서 길 알려줌. 무조건 1자가 아니라 좌우로 쪼끔씩 흔들리는 이펙트인듯.
    //거리가 좀 있으면 안개가 살짝 깔리는 맵도 있는 거 같음.
    ImGui::End();
}

void CLevel_Map::Menu_Light()
{
    // 조명. 일단 Imgui에 List로 현재 내가 넣은 조명들 정보? 순서 띄우기. 버튼형식으로 누르면 그 조명에 대한 정보가 나오게.
       // 라이트 오브젝트를 하나 만들어서 그 놈의 위치 정보를 조명으로. 조절할 수 있게. -> 라이트 객체가 현재 추가된 조명들 중에서 몇 번째 순서인지
       // 각종 색상정보 및 세기, 디퓨즈 앰비언트 기타 등등 다 수정할 수 있게. -> 실시간 적용? or 버튼 누르면 적용. 되돌리기 기능도 있음 좋을듯
       // 
       //기즈모 달거면 조명에 달기. 

}

HRESULT CLevel_Map::Ready_Static_Component()
{
    return S_OK;
}

CLevel_Map* CLevel_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Map* pInstance = new CLevel_Map(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Map");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Map::Free()
{
    __super::Free();

}
