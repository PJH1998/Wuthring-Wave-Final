#pragma once

#include "../Default/framework.h"
#include <process.h>

#include "Editor_Enum.h"
//#include "Client_Struct.h"

#define GRAVITY 98.f

namespace Editor
{
	// Window SIze
	const unsigned int		g_iWinSizeX = 1920;
	const unsigned int		g_iWinSizeY = 1080;
	const unsigned int		g_iFrame = 60;
}

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
using namespace Editor;