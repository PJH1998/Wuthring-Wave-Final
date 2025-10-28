#pragma once

#include "../Default/framework.h"
#include <process.h>

#include "Editor_Enum.h"
//#include "Editor_Struct.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

#define GRAVITY 98.f


namespace Engine
{
	class CModel;
}

namespace Editor
{
	// Window SIze
	const unsigned int		g_iWinSizeX = 1920;
	const unsigned int		g_iWinSizeY = 1080;
	const unsigned int		g_iFrame = 250;


	typedef struct tagEffectActorDesc
	{
		class CAnimationActor* pAnimActor = { nullptr }; // Animation 객체 주소
		float fDuration = {}; // 현재선택한 Animation Duration
	}EFFECTACTOR_DESC;
}



extern HWND			g_hWnd;
extern HINSTANCE		g_hInst;
using namespace Editor;