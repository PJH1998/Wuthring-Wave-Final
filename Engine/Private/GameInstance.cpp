#include "EnginePch.h"
#include "GameInstance.h"

#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Sound_Manager.h"
#include "Font_Manager.h"
#include "Level_Manager.h"
#include "Prototype_Manager.h"
#include "Object_Manager.h"
#include "Pooling_Manager.h"
#include "Target_Manager.h"
#include "Renderer.h"
#include "Timer_Manager.h"
#include "PhysicsManager.h"
#include "Camera_Manager.h"
#include "EventBus.h"
#include "PipeLine.h"
#include "Light_Manager.h"
#include "Picking.h"
#include "Shadow.h"
#include "GUIManager.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{
}

#pragma region ENGINE
HRESULT CGameInstance::Ready_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eMode, EngineDesc.iSizeX, EngineDesc.iSizeY, ppDevice, ppContext);
	ASSERT_CRASH(m_pGraphic_Device);

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInst, EngineDesc.hWnd);
	ASSERT_CRASH(m_pInput_Device);

	m_pSound_Manager = CSound_Manager::Create(EngineDesc.iNumChannel);
	ASSERT_CRASH(m_pSound_Manager);

	m_pFont_Manager = CFont_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pFont_Manager);

	m_pLevel_Manager = CLevel_Manager::Create();
	ASSERT_CRASH(m_pLevel_Manager);
	m_iNumLevel = EngineDesc.iNumLevel;

	m_pPrototype_Manager = CPrototype_Manager::Create(m_iNumLevel);
	ASSERT_CRASH(m_pPrototype_Manager);
	
	m_pObject_Manager = CObject_Manager::Create(m_iNumLevel);
	ASSERT_CRASH(m_pObject_Manager);

	m_pPooling_Manager = CPooling_Manager::Create();
	ASSERT_CRASH(m_pPooling_Manager);

	m_pTargetManager = CTarget_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pTargetManager);

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pRenderer);

	m_pLight_Manager = CLight_Manager::Create();
	ASSERT_CRASH(m_pLight_Manager);

	m_pCamera_Manager = CCamera_Manager::Create(*ppDevice, *ppContext, EngineDesc.iNumLevel);
	ASSERT_CRASH(m_pCamera_Manager);

	m_pTimer_Manager = CTimer_Manager::Create();
	ASSERT_CRASH(m_pTimer_Manager);

	m_pPhysicsManager = CPhysicsManager::Create(*ppDevice, *ppContext, EngineDesc.iNumCollisionLayer);
	ASSERT_CRASH(m_pPhysicsManager);

	m_pEventBus = CEventBus::Create();
	ASSERT_CRASH(m_pEventBus);

	m_pPipeLine = CPipeLine::Create();
	ASSERT_CRASH(m_pPipeLine);

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pPicking);

	m_pShadow = CShadow::Create(static_cast<_float>(EngineDesc.iSizeX), static_cast<_float>(EngineDesc.iSizeY));
	ASSERT_CRASH(m_pShadow);

	m_pGUIManager = CGUIManager::Create(*ppDevice, *ppContext, EngineDesc.hWnd);
	ASSERT_CRASH(m_pGUIManager);

	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	m_pGUIManager->Update();

	m_pPicking->Update();
	m_pInput_Device->Update();

	m_pObject_Manager->Priority_Update(fTimeDelta);
	m_pObject_Manager->Update(fTimeDelta);

	m_pCamera_Manager->Update(fTimeDelta);
	m_pPipeLine->Update();
	m_pObject_Manager->Late_Update(fTimeDelta);

	m_pPooling_Manager->Update_Pooling();

	m_pLevel_Manager->Update_Level(fTimeDelta);

	m_pPhysicsManager->Update(fTimeDelta);
}

_float CGameInstance::Rand_Normal()
{
	return static_cast<_float>(rand()) / RAND_MAX;
}
_float CGameInstance::Rand(_float fMin, _float fMax)
{
	return Rand_Normal() * (fMax - fMin) + fMin;
}
#pragma endregion


#pragma region GRAPHIC_DEVICE
void CGameInstance::Render_Begin(const _float4* pClearColor)
{
	ASSERT_CRASH(m_pGraphic_Device);
	m_pGraphic_Device->Clear_BackBuffer_View(pClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();
}

HRESULT CGameInstance::Draw()
{
	ASSERT_CRASH(m_pRenderer);
	m_pRenderer->Render();

	ASSERT_CRASH(m_pLevel_Manager);
	m_pLevel_Manager->Render();

#ifdef _DEBUG
	ASSERT_CRASH(m_pGUIManager);
	m_pGUIManager->Render();
	ASSERT_CRASH(m_pPhysicsManager);
	m_pPhysicsManager->Render();
#endif

	return S_OK;
}

void CGameInstance::Render_End()
{
	m_pGraphic_Device->Present();
}
#pragma endregion

#pragma region INPUT_DEVICE
KEYSTATE CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}
KEYSTATE CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eState)
{
	return m_pInput_Device->Get_DIMouseState(eState);
}
_long CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eState)
{
	return m_pInput_Device->Get_DIMouseMove(eState);
}
#pragma endregion

#pragma region SOUND_MANAGER
HRESULT CGameInstance::Load_Sound(const _wstring& strSoundTag, const char* pSoundFilePath)
{
	return m_pSound_Manager->Load_Sound(strSoundTag, pSoundFilePath);
}
void CGameInstance::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop)
{
	m_pSound_Manager->Play_Sound(strSoundTag, iChannelID, fVolume, isStop);
}
void CGameInstance::Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop)
{
	m_pSound_Manager->Play_BGM(strSoundTag, iChannelID, fVolume, isStop);
}
void CGameInstance::Play_Other(const _wstring& strSoundTag, _float fVolume)
{
	m_pSound_Manager->Play_Other(strSoundTag, fVolume);
}
void CGameInstance::Stop_Sound(_uint iChannelID)
{
	m_pSound_Manager->Stop_Sound(iChannelID);
}
void CGameInstance::Stop_All()
{
	m_pSound_Manager->Stop_All();
}
void CGameInstance::Set_ChannelVolume(_uint iChannelID, _float fVolume)
{
	m_pSound_Manager->Set_ChannelVolume(iChannelID, fVolume);
}
#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _tchar* pFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFilePath);
}
HRESULT CGameInstance::Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale)
{
	return m_pFont_Manager->Draw_Text(strFontTag, pText, vPosition, vColor, fRadian, vOrigin, vScale);
}
#pragma endregion

#pragma region LEVEL_MANAGER
_uint CGameInstance::Get_CurrentLevel()
{
	return m_pLevel_Manager->Get_CurrentLevel();
}
HRESULT CGameInstance::Open_Level(_uint iNextLevelID, CLevel* pLevel)
{
	return m_pLevel_Manager->Open_Level(iNextLevelID, pLevel);
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER
HRESULT CGameInstance::Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iPrototypeLevelID, strPrototypeTag, pPrototype);
}
CBase* CGameInstance::Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, eType, pArg);
}
#pragma endregion

#pragma region OBJECT_MANAGER
HRESULT CGameInstance::Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg)
{
	return m_pObject_Manager->Add_GameObject_ToLayer(iPrototypeLevelID, strPrototypeTag, iLayerLevelID, strLayerTag, pArg);
}
HRESULT CGameInstance::Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, CGameObject* pObject)
{
	return m_pObject_Manager->Add_GameObject_ToLayer(iLayerLevelID, strLayerTag, pObject);
}
CComponent* CGameInstance::Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag)
{
	return m_pObject_Manager->Get_Component(iLayerLevelID, strLayerTag, iGameObjectIndex, strComponentTag);
}
HRESULT CGameInstance::Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio)
{
	return m_pObject_Manager->Change_TimeRatio_ToLayer(iLayerLevelID, strLayerTag, fTimeRatio);
}
#pragma endregion

#pragma region POOLING_MANAGER
HRESULT CGameInstance::Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg)
{
	return m_pPooling_Manager->Add_PoolingObject(iPrototypeLevelID, strPrototypeTag, iLayerLevelID, strLayerTag, strPoolingTag, iNumObjects, pArg);
}
HRESULT CGameInstance::Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg)
{
	return m_pPooling_Manager->Spawn_PoolingObject(strPoolingTag, WorldMatrix, pArg);
}
void CGameInstance::Add_Work(function<void()> Work)
{
	m_pPooling_Manager->Add_Work(Work);
}
_bool CGameInstance::IsWorkFinish()
{
	return m_pPooling_Manager->IsWorkFinish();
}
#pragma endregion

#pragma region TARGET_MANAGER
ID3D11Resource* CGameInstance::Get_RT_Resource(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_RT_Resource(strTargetTag);
}
HRESULT CGameInstance::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor)
{
	return m_pTargetManager->Add_RenderTarget(strTargetTag, iWidth, iHeight, eFormat, vClearColor);
}
HRESULT CGameInstance::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
	return m_pTargetManager->Add_MRT(strMRTTag, strTargetTag);
}
HRESULT CGameInstance::Bind_RenderTarget(const _wstring& strTargetTag, CShader* pShader, const _char* pConstantName)
{
	return m_pTargetManager->Bind_Shader_Resource(strTargetTag, pShader, pConstantName);
}
HRESULT CGameInstance::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isClear)
{
	return m_pTargetManager->Begin_MRT(strMRTTag, pDSV, isClear);
}
void CGameInstance::End_MRT()
{
	m_pTargetManager->End_MRT();
}
HRESULT CGameInstance::Clear_RT(const _wstring& strTargetTag)
{
    return m_pTargetManager->Clear_RT(strTargetTag);
}
#ifdef _DEBUG
HRESULT CGameInstance::Ready_Debug_RT(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTargetManager->Ready_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Render_RT(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pTargetManager->Render(pShader, pVIBuffer);
}
#endif
#pragma endregion

#pragma region RENDERER
HRESULT CGameInstance::Add_Render_Object(RENDERGROUP eGroup, CGameObject* pObject)
{
	return m_pRenderer->Add_Render_Object(eGroup, pObject);
}
#ifdef _DEBUG
HRESULT CGameInstance::Add_Render_Debug(CComponent* pDebugComponent)
{
	return m_pRenderer->Add_Render_Debug(pDebugComponent);
}
#endif
#pragma endregion

#pragma region LIGHT_MANAGER
const LIGHT_DESC* CGameInstance::Get_LightDesc(const _wstring& strLightTag)
{
	return m_pLight_Manager->Get_LightDesc(strLightTag);
}
HRESULT	CGameInstance::Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{

	return m_pLight_Manager->Add_Light(strLightTag, LightDesc);
}
HRESULT	CGameInstance::SetUp_Light(class CShader* pShader, const _wstring& strLightTag, LIGHT_DESC::TYPE eType)
{
	return m_pLight_Manager->SetUp_Light(pShader, strLightTag, eType);
}
HRESULT CGameInstance::Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pLight_Manager->Render(pShader, pVIBuffer);
}
#pragma endregion

#pragma region CAMERA_MANAGER
HRESULT CGameInstance::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, CCamera* pCamera)
{
	return m_pCamera_Manager->Add_Camera(iLevelID, strCameraTag, pCamera);
}
HRESULT CGameInstance::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pCamera_Manager->Add_Camera(iLevelID, strCameraTag, iPrototypeLevelID, strPrototypeTag, pArg);
}
HRESULT CGameInstance::Add_Camera_Action(const _wstring& strActionTag, const vector<ACTIONFRAME>& ActionFrames)
{
	return m_pCamera_Manager->Add_Camera_Action(strActionTag, ActionFrames);
}
HRESULT CGameInstance::Add_Camera_Action(const _wstring& strActionTag, const _char* pFilePath)
{
	return m_pCamera_Manager->Add_Camera_Action(strActionTag, pFilePath);
}
void CGameInstance::Play_Action(const _wstring& strActionTag)
{
	m_pCamera_Manager->Play_Action(strActionTag);
}
HRESULT CGameInstance::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag)
{
	return m_pCamera_Manager->Change_MainCamera(iLevelID, strCameraTag);
}
void CGameInstance::Change_Distance(_float fDistance)
{
	m_pCamera_Manager->Change_Distance(fDistance);
}
void CGameInstance::Change_FixedDistance(_float fFixedDistance)
{
	m_pCamera_Manager->Change_FixedDistance(fFixedDistance);
}
#pragma endregion

#pragma region TIMER_MANAGER
_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}
void CGameInstance::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate)
{
	m_pTimer_Manager->Change_TimeRate(strTimerTag, fTimeRate);
}
HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}
#pragma endregion

#pragma region PHYSICS_MANAGER
void CGameInstance::SetUp_PhysicsSystem()
{
	m_pPhysicsManager->SetUp_PhysicsSystem();
}
void CGameInstance::SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer)
{
	m_pPhysicsManager->SetUp_ObjectToBP(iObjectLayer, iBPLayer);
}
void CGameInstance::SetUp_ObjectFilter(_uint iSrc, _uint iDst)
{
	m_pPhysicsManager->SetUp_ObjectFilter(iSrc, iDst);
}
void CGameInstance::SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer)
{
	m_pPhysicsManager->SetUp_ObjectVsBPFilter(iObjectLayer, iBPLayer);
}
Body* CGameInstance::Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut)
{
	return m_pPhysicsManager->Register_Body(BodySetting, pOut);
}
#pragma endregion

#pragma region PIPELINE
const _float4x4* CGameInstance::Get_TransformState_Float4x4(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Float4x4(eState);
}

_matrix CGameInstance::Get_TransformState_Matrix(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Matrix(eState);
}

const _float4x4* CGameInstance::Get_TransformState_Float4x4_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Float4x4_Inv(eState);
}

_matrix CGameInstance::Get_TransformState_Matrix_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Matrix_Inv(eState);
}

void CGameInstance::Set_TransformState(D3DTS eState, _fmatrix Matrix)
{
	m_pPipeLine->Set_TransformState(eState, Matrix);
}

void CGameInstance::Set_TransformState(D3DTS eState, const _float4x4& Matrix)
{
	m_pPipeLine->Set_TransformState(eState, Matrix);
}

const _float4* CGameInstance::Get_CamPos() const
{
	return m_pPipeLine->Get_CamPos();
}
_float CGameInstance::Compute_Distance_ToCam(CGameObject* pObject)
{
	return m_pPipeLine->Compute_Distance(pObject);
}
#pragma endregion

#pragma region PICKING
_bool CGameInstance::isPicked(_float3* pOut)
{
	return m_pPicking->isPicked(pOut);
}
#pragma endregion

#pragma region SHADOW
const _float4x4* CGameInstance::Get_ShadowLight_Matrix(D3DTS eType)
{
	return m_pShadow->Get_Matrix(eType);
}
HRESULT CGameInstance::Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc)
{
	return m_pShadow->Ready_ShadowLight(Desc);
}
HRESULT CGameInstance::Bind_Shadow_Resource(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName)
{
    return m_pShadow->Bind_Shadow_Resource(pShader, pViewName, pProjName, pFarName);
}
void CGameInstance::Update_ShadowLight_Transform(const _fvector& vAt)
{
	m_pShadow->Update_Transform(vAt);
}
#pragma endregion

#pragma region GUIMANAGER
ImGuiContext* CGameInstance::Get_ImGuiContext()
{
	return m_pGUIManager->Get_ImGuiContext();
}
#pragma endregion

HRESULT CGameInstance::Clear_Resource(_uint iLevelID)
{
	if (FAILED(m_pCamera_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	if (FAILED(m_pObject_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	if (FAILED(m_pPrototype_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	return S_OK;
}

HRESULT CGameInstance::Clear_Memory()
{
	m_pSound_Manager->Stop_All();
	m_pEventBus->Unscribe();
	m_pLight_Manager->Clear_Light();

	if (FAILED(m_pPooling_Manager->Clear_Resource()))
		return E_FAIL;

	//if (FAILED(m_pSound_Manager->Clear_Resource()))
	//	return E_FAIL;

	return S_OK;
}

void CGameInstance::Release_Engine()
{
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pSound_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pPrototype_Manager);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pPooling_Manager);
	Safe_Release(m_pTargetManager);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pPhysicsManager);
	Safe_Release(m_pCamera_Manager);
	Safe_Release(m_pEventBus);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pPicking);
	Safe_Release(m_pShadow);
	Safe_Release(m_pGUIManager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pGraphic_Device);
	Release();
}

void CGameInstance::Free()
{
	__super::Free();
}