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
#include "OctoTree.h"
#include "Frustrum.h"
#include "CSM.h"
#include "RCS_Manager.h"

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

	m_pOctoTree = COctoTree::Create();
	ASSERT_CRASH(m_pOctoTree);

	m_pTargetManager = CTarget_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pTargetManager);

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

	m_pFrustrum = CFrustrum::Create();
	ASSERT_CRASH(m_pFrustrum);

	m_pCSM = CCSM::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pCSM);

	m_pRCS_Manager = CRCS_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pRCS_Manager);

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pRenderer);
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
	//m_pPooling_Manager->Add_Work([this]() {m_pOctoTree->Update(); });
	m_pPhysicsManager->Update(fTimeDelta);
	m_pPhysicsManager->Late_Update();

	m_pObject_Manager->Late_Update(fTimeDelta);

	m_pCamera_Manager->Late_Update(fTimeDelta);
	m_pPipeLine->Update();
	m_pFrustrum->Update();
	m_pPooling_Manager->Add_Work([this]() {m_pCSM->Update_CSM(); });
	m_pOctoTree->Update();

	m_pPooling_Manager->Update_Pooling();

	m_pLevel_Manager->Update_Level(fTimeDelta);
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
	ASSERT_CRASH(m_pPooling_Manager);
	m_pPooling_Manager->Wait_Thread_End();

	ASSERT_CRASH(m_pRenderer);
	m_pRenderer->Render();

	ASSERT_CRASH(m_pLevel_Manager);
	m_pLevel_Manager->Render();

#ifdef _DEBUG
	ASSERT_CRASH(m_pPhysicsManager);
	m_pPhysicsManager->Render();
#endif
	ASSERT_CRASH(m_pGUIManager);
	m_pGUIManager->Render();

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
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _char* pFilePath)
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
HRESULT CGameInstance::Clear_CurrentLevel_Resources(_uint iNextLevel)
{
	return m_pLevel_Manager->Clear_CurrentLevel_Resources(iNextLevel);
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER
HRESULT CGameInstance::Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iPrototypeLevelID, strPrototypeTag, pPrototype);
}
void CGameInstance::Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag)
{
	m_pPrototype_Manager->Remove_Prototype(iPrototypeLevelID, strPrototypeTag);
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
void CGameInstance::Wait_Thread_End()
{
	m_pPooling_Manager->Wait_Thread_End();
}
#pragma endregion

#pragma region OctoTree
void CGameInstance::SetUp_OctoTree(_float3 vCenter, _float3 vExtent)
{
	m_pOctoTree->SetUp_OctoTree(vCenter, vExtent);
}

void CGameInstance::Add_To_OctoTree(CStaticObject* pObject, const BoundingBox* pBox)
{
	m_pOctoTree->Add_To_OctoTree(pObject, pBox);
}
#pragma endregion


#pragma region TARGET_MANAGER
ID3D11Resource* CGameInstance::Get_RT_Resource(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_RT_Resource(strTargetTag);
}
ID3D11ShaderResourceView* CGameInstance::Get_RT_SRV(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_RT_SRV(strTargetTag);
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
HRESULT CGameInstance::Render_RT()
{
	return m_pTargetManager->Render();
}
ID3D11ShaderResourceView* CGameInstance::Get_Debug_RT_Resource(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_Debug_RT_Resource(strTargetTag);
}
#endif
#pragma endregion

#pragma region RENDERER
HRESULT CGameInstance::Add_Render_Object(RENDERGROUP eGroup, CGameObject* pObject)
{
	return m_pRenderer->Add_Render_Object(eGroup, pObject);
}
#ifdef _DEBUG
void CGameInstance::Set_LUT_Index(_uint iIndex)
{
	m_pRenderer->Set_LUT_Index(iIndex);
}
HRESULT CGameInstance::Add_Render_Debug(CComponent* pDebugComponent)
{
	return m_pRenderer->Add_Render_Debug(pDebugComponent);
}
HRESULT CGameInstance::Bind_RawValue_Renderer(const _char* pConstantName, void* pValue, _uint iLength)
{
	return m_pRenderer->Bind_RawValue(pConstantName, pValue, iLength);
}
void CGameInstance::IsSSAO(_bool IsSSAO)
{
	m_pRenderer->IsSSAO(IsSSAO);
}
void CGameInstance::IsSSAO_Blur(_bool IsBlur)
{
	m_pRenderer->IsSSAO_Blur(IsBlur);
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
HRESULT CGameInstance::Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pLight_Manager->Render(pShader, pVIBuffer);
}
#ifdef _DEBUG
LIGHT_DESC* CGameInstance::Get_LightDesc_For_Map(const _wstring& strLightTag)
{
	return m_pLight_Manager->Get_LightDesc_For_Map(strLightTag);
}
#endif
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
_float CGameInstance::Get_CurrentCamera_Near()
{
	return m_pCamera_Manager->Get_CurrentCamera_Near();
}
_float CGameInstance::Get_CurrentCamera_Far()
{
	return m_pCamera_Manager->Get_CurrentCamera_Far();
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
Character* CGameInstance::Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	return m_pPhysicsManager->Register_Character(CharacterSetting, vPos, vQuat, pUserData);
}
Ref<CharacterVirtual> CGameInstance::Register_Virtual(const CharacterVirtualSettings& VirtualSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
    return m_pPhysicsManager->Register_CharacterVirtual(VirtualSetting, vPos, vQuat, pUserData);
}
void CGameInstance::Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer)
{
	m_pPhysicsManager->Add_Virtual(pVirtual, iObjectLayer);
}
void CGameInstance::Remove_Virtual(CharacterVirtual* pVirtual)
{
	m_pPhysicsManager->Remove_Virtual(pVirtual);
}
_bool CGameInstance::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, _float4* pOut)
{
	return m_pPhysicsManager->Ray_Cast(vStartPos, vEndPos, pOut);
}
#ifdef _DEBUG
void CGameInstance::DrawShape(const Shape* pShape, RMat44 Matrix)
{
	m_pPhysicsManager->DrawShape(pShape, Matrix);
}
void CGameInstance::DrawRay(const _fvector& vStartPos, const _fvector& vEndPos)
{
	m_pPhysicsManager->DrawRay(vStartPos, vEndPos);
}
#endif
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
POINT CGameInstance::Get_MousePoint()
{
	return m_pPicking->Get_MousePoint();
}
_bool CGameInstance::isPicked(_float3* pOut)
{
	return m_pPicking->isPicked(pOut);
}
_bool CGameInstance::Get_Points(_float fRange, vector<_float4>& pOut, _uint* NumPixels, _float4* pOutMousePos)
{
	return m_pPicking->Get_Points(fRange, pOut,NumPixels,pOutMousePos);
}
#pragma endregion

#pragma region SHADOW
const _float4x4* CGameInstance::Get_ShadowLight_Matrix(D3DTS eType)
{
	return m_pShadow->Get_Matrix(eType);
}
HRESULT CGameInstance::Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc)
{
//	return m_pShadow->Ready_ShadowLight(Desc);
	return S_OK;
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
void CGameInstance::Add_GUI_Func(function<void()> func)
{
	m_pGUIManager->Add_GUI_Func(func);
}
void CGameInstance::Use_Gizmo(CTransform* pTransform)
{
	m_pGUIManager->Use_Gizmo(pTransform);
}
void CGameInstance::Render_Gizmo(const _fmatrix& Matrix)
{
	m_pGUIManager->Render_Gizmo(Matrix);
}
#pragma endregion

#pragma region FRUSTRUM
const _float4* CGameInstance::Get_Frustrum_WorldPoints() const
{
	return m_pFrustrum->Get_Frustrum_WorldPoints();
}
_bool CGameInstance::IsIn_WorldSpace(_fvector vWorldPosition, _float fRange)
{
	return m_pFrustrum->IsIn_WorldSpace(vWorldPosition, fRange);
}
_bool CGameInstance::IsIn_WorldSpace(const BoundingBox* pBoundingBox)
{
	return m_pFrustrum->IsIn_WorldSpace(pBoundingBox);
}
_bool CGameInstance::IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange)
{
	return m_pFrustrum->IsIn_LocalSpace(WorldMatrix, vLocalPosition, fRange);
}
HRESULT CGameInstance::SetUp_ShadowLight(const _wstring& strLightTag)
{
	return m_pCSM->SetUp_ShadowLight(strLightTag);
}
HRESULT CGameInstance::SetUp_ShadowNF()
{
	return m_pCSM->SetUp_ShadowNF();
}
HRESULT CGameInstance::Bind_CSM_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName)
{
	return m_pCSM->Bind_CSM_Resources(pShader, pViewName, pProjName, pLightDirName);
}
HRESULT CGameInstance::Bind_ShadowDistance_Resource(_uint iDataBufferIndex)
{
	return m_pCSM->Bind_ShadowDistance_Resource(iDataBufferIndex);
}
HRESULT CGameInstance::Bind_CSM_SRV(CShader* pShader, const _char* pConstantName)
{
	return m_pCSM->Bind_CSM_SRV(pShader, pConstantName);
}
HRESULT CGameInstance::Begin_CSM()
{
	return m_pCSM->Begin_CSM();
}
HRESULT CGameInstance::End_CSM()
{
	return m_pCSM->End_CSM();
}
#ifdef _DEBUG
void CGameInstance::Render_CSM(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	m_pCSM->Render(pShader, pVIBuffer);
}
#endif
#pragma endregion

#pragma region RCS_MANAGER
HRESULT CGameInstance::Add_RCS(const _wstring& strRCSTag, void* pDesc)
{
	return m_pRCS_Manager->Add_RCS(strRCSTag, pDesc);
}
HRESULT CGameInstance::Add_BufferData(const _wstring& strRCSTag, const _char* pConstantName, void* pData, _uint iLength)
{
	return m_pRCS_Manager->Add_BufferData(strRCSTag, pConstantName, pData, iLength);
}
HRESULT CGameInstance::Add_SRVData(const _wstring& strRCSTag, const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	return m_pRCS_Manager->Add_SRVData(strRCSTag, pConstantName, pSRV);
}
HRESULT CGameInstance::Setting_UAV_Data(const _wstring& strRCSTag, const _char* pConstantName)
{
	return m_pRCS_Manager->Setting_UAV_Data(strRCSTag, pConstantName);
}
HRESULT CGameInstance::Bind_RendererCS(const _wstring& strRCSTag, CShader* pShader, const _char* pConstantName)
{
	return m_pRCS_Manager->Bind_RendererCS(strRCSTag, pShader, pConstantName);
}
HRESULT CGameInstance::Begin_RCS(const _wstring& strRCSTag)
{
	return m_pRCS_Manager->Begin_RCS(strRCSTag);
}
void CGameInstance::Clear_RCS(const _wstring& strRCSTag)
{
	m_pRCS_Manager->Clear_RCS(strRCSTag);
}
#ifdef _DEBUG
HRESULT CGameInstance::Debug_Render_RCS()
{
	return m_pRCS_Manager->Debug_Render();
}
#endif
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
	m_pOctoTree->Clear_OctoTree();
	m_pSound_Manager->Stop_All();
	m_pEventBus->Unscribe();
	m_pGUIManager->Clear_Func();
	m_pLight_Manager->Clear_Light();
	m_pCSM->Clear();

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
	Safe_Release(m_pOctoTree);
	Safe_Release(m_pTargetManager);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pCamera_Manager);
	Safe_Release(m_pEventBus);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pPicking);
	Safe_Release(m_pShadow);
	Safe_Release(m_pGUIManager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pGraphic_Device);
	Safe_Release(m_pFrustrum);
	Safe_Release(m_pCSM);
	Safe_Release(m_pRCS_Manager);
	Safe_Release(m_pPhysicsManager);

	Release();
}

void CGameInstance::Free()
{
	__super::Free();
}