#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "FX11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/GeometricPrimitive.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"
using namespace DirectX;

// Fmod
#include "Fmod/fmod.hpp"
#define FMOD_CHANNEL_MAX 32

// Json
#include "Json/json.hpp"
using json = nlohmann::json;

// ImGui
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/ImGuiFileDialog.h"
#include "ImGui/ImGuiFileDialogConfig.h"

// Jolt
#define JPH_NAMESPACE JPH
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/JobSystemSingleThreaded.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/Collision/CollisionDispatch.h"
using namespace JPH;

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <Windows.h>
#include <fcntl.h>

#include <io.h>
#include <iostream>
using namespace std;

namespace Engine
{
	static const unsigned int g_iMaxNumBones = 512;

	//const unsigned int g_iMaxWidth = 16384;
	//const unsigned int g_iMaxHeight = 9216;	

	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Vertex.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

using namespace Engine;


#endif // Engine_Define_h__
