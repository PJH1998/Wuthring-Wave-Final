#include "ClientPch.h"
#include "Director.h"

#include "Event_Camera.h"

CDirector::CDirector()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CDirector::Add_Action(const _char* pFolderPath)
{
	for (const auto& entry : filesystem::directory_iterator(pFolderPath))
	{
		CAMERA_ACTION Action = {};
		_string strFilePath = entry.path().string();
		_wstring wstrFileName = entry.path().stem().wstring();

		ifstream InputFile(strFilePath);
		if (false == InputFile.is_open())
			CRASH(strFilePath);

		json ActionJson;
		InputFile >> ActionJson;

		Action.iFrameStart = 0;
		Action.iFrameEnd = static_cast<_int>(ActionJson["Duration"]);


		for (auto& Frame : ActionJson["Frame"])
		{
			CAMERA_FRAME CameraFrame = {};
			CameraFrame.fStartFrame = Frame["Start"];
			CameraFrame.vRotation = _float4(Frame["Rotation"][0], Frame["Rotation"][1], Frame["Rotation"][2], Frame["Rotation"][3]);
			CameraFrame.vTranslation = _float3(Frame["Translation"][0], Frame["Translation"][1], Frame["Translation"][2]);
			CameraFrame.fDistance = Frame["Distance"];
			CameraFrame.fFovy = Frame["FOV"];
			if (Frame.contains("Lerp"))
				CameraFrame.isLerp = Frame["Lerp"];

			Action.Frames.push_back(CameraFrame);
		}

		m_CameraActions.emplace(wstrFileName, Action);

		InputFile.close();
	}
}

void CDirector::Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain, _bool isEscape)
{
	CAMERA_ACTION CameraAction = Find_Action(strActionTag);
	_float4x4 Matrix = {};
	XMStoreFloat4x4(&Matrix, WorldMatrix);
	CAMERA_ACTION_EVENT event{ CameraAction.Frames, true, CameraAction.iFrameStart, CameraAction.iFrameEnd, Matrix, isMaintain, isEscape };
	m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), event);
}

void CDirector::Stop_Action()
{
	CAMERA_ACTION CameraAction = {};
	_float4x4 Temp = {};
	CAMERA_ACTION_EVENT event{ CameraAction.Frames, false, 0, 0, Temp, false};
	m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), event);
}

const CDirector::CAMERA_ACTION& CDirector::Find_Action(const _wstring& strActionTag)
{
	auto iter = m_CameraActions.find(strActionTag);

	if (iter == m_CameraActions.end())
		CRASH("Action Tag");

	return iter->second;
}

CDirector* CDirector::Create()
{
    return new CDirector();
}

void CDirector::Free()
{
	Safe_Release(m_pGameInstance);
}
