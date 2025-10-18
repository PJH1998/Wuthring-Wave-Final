#pragma once
/*
	[寃뚯엫 ?몄뒪?댁뒪]
	Client? Engine ?곌껐
*/
#include "Prototype_Manager.h"
#include "EventBus.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	explicit CGameInstance();
	virtual ~CGameInstance() = default;

#pragma region ENGINE
public:
	HRESULT			Ready_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	void				Update_Engine(_float fTimeDelta);

	// 0~1 ?쒕뜡??媛?諛섑솚
	_float				Rand_Normal();
	// Min, Max ?ъ씠???쒕뜡 媛?蹂??
	_float				Rand(_float fMin, _float fMax);
#pragma endregion

#pragma region GRAPHIC_DEVICE
public:
	void				Render_Begin(const _float4* pClearColor);
	HRESULT			Draw();
	void				Render_End();
#pragma endregion

#pragma region INPUT_DEVICE
public:
	KEYSTATE Get_DIKeyState(_ubyte byKeyID);
	KEYSTATE Get_DIMouseState(MOUSEKEYSTATE eState);
	_long Get_DIMouseMove(MOUSEMOVESTATE eState);
#pragma endregion

#pragma region SOUND_MANAGER
	HRESULT		Load_Sound(const _wstring& strSoundTag, const char* pSoundFilePath);
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop = false);
	void			Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop = false);
	void			Play_Other(const _wstring& strSoundTag, _float fVolume);
	void			Stop_Sound(_uint iChannelID);
	void			Stop_All();
	void			Set_ChannelVolume(_uint iChannelID, _float fVolume);
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT		Add_Font(const _wstring& strFontTag, const _tchar* pFilePath);
	HRESULT		Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRadian = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), const _float2& vScale = _float2(1.f, 1.f));
#pragma endregion


#pragma region LEVEL_MANAGER
public:
	_uint				Get_CurrentLevel();
	HRESULT			Open_Level(_uint iNextLevelID, class CLevel* pLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT		Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype);
	void		Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag);
	CBase*		Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg = nullptr);
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT		Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg = nullptr);
	HRESULT		Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, class CGameObject* pObject);
	class CComponent* Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag);
	HRESULT		Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio);
#pragma endregion

#pragma region POOLING_MANAGER
	HRESULT		Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg = nullptr);
	HRESULT		Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg = nullptr);
	// Thread濡??ㅽ뻾???⑥닔 異붽?
	void			Add_Work(function<void()> Work);
	// Thread媛 紐⑤몢 醫낅즺?섏뿀?붿? 諛섑솚
	_bool			IsWorkFinish();
	// Thread 紐⑤몢 ?앸궇 ?뚭퉴吏 ?湲?
	void			Wait_Thread_End();
#pragma endregion

#pragma region OctoTree
	void			SetUp_OctoTree(_float3 vCenter, _float3 vExtent);
	void			Add_To_OctoTree(class CStaticObject* pObject, const BoundingBox* pBox);

#pragma endregion

#pragma region TARGET_MANAGER
	ID3D11Resource* Get_RT_Resource(const _wstring& strTargetTag);
	HRESULT		Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor);
	HRESULT		Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT		Bind_RenderTarget(const _wstring& strTargetTag, class CShader* pShader, const _char* pConstantName);
	HRESULT		Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isClear = true);
	void		End_MRT();
	HRESULT		Clear_RT(const _wstring& strTargetTag);
#ifdef _DEBUG
	HRESULT		Ready_Debug_RT(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT		Render_RT(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	HRESULT     Render_RT();
	ID3D11ShaderResourceView* Get_Debug_RT_Resource(const _wstring& strTargetTag);
#endif
#pragma endregion


#pragma region RENDERER
public:
	HRESULT		Add_Render_Object(RENDERGROUP eGroup, class CGameObject* pObject);
#ifdef _DEBUG
	HRESULT		Add_Render_Debug(class CComponent* pDebugComponent);
#endif
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC*		Get_LightDesc(const _wstring& strLightTag);
	HRESULT					Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	HRESULT					Render_Light(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#ifdef _DEBUG
	LIGHT_DESC* Get_LightDesc_For_Map(const _wstring& strLightTag);
#endif
#pragma endregion

#pragma region CAMERA_MANAGER
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, class CCamera* pCamera);
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg);
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const vector<ACTIONFRAME>& ActionFrames);
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const _char* pFilePath);
	void			Play_Action(const _wstring& strActionTag);
	HRESULT			Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag);
	void			Change_Distance(_float fDistance);
	void			Change_FixedDistance(_float fFixedDistance);
	_float			Get_CurrentCamera_Near();
	_float			Get_CurrentCamera_Far();
#pragma endregion

#pragma region TIMER_MANAGER
public:
	_float			Get_TimeDelta(const _wstring& strTimerTag);
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate);
	HRESULT			Add_Timer(const _wstring& strTimerTag);
#pragma endregion

#pragma region PHYSICS_MANAGER
	void				SetUp_PhysicsSystem();
	void				SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer);
	void				SetUp_ObjectFilter(_uint iSrc, _uint iDst);
	void				SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer);
	Body*				Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut);
	Character*			Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData);
	CharacterVirtual*	Register_Virtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData);
	void				Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer);
#ifdef _DEBUG
	void				DrawShape(const Shape* pShape);
#endif
#pragma endregion


#pragma region EVENTBUS
public:
	template<typename TEvent>
	void Subscribe(_uint iLevelID, const _wstring& strEventTag, function<void(const TEvent&)> handler) { m_pEventBus->Subscribe(iLevelID, strEventTag, handler); }
	void Publish(_uint iLevelID, const _wstring& strEventTag, const class CEvent& event) { m_pEventBus->Publish(iLevelID, strEventTag, event); }
	void Unscribe() { m_pEventBus->Unscribe(); };
#pragma endregion
	
#pragma region PIPELINE
public:
	const _float4x4*		Get_TransformState_Float4x4(D3DTS eState) const;
	_matrix					Get_TransformState_Matrix(D3DTS eState) const;

	const _float4x4*		Get_TransformState_Float4x4_Inv(D3DTS eState) const;
	_matrix					Get_TransformState_Matrix_Inv(D3DTS eState) const;

	void					Set_TransformState(D3DTS eState, _fmatrix Matrix);
	void					Set_TransformState(D3DTS eState, const _float4x4& Matrix);

	const _float4*			Get_CamPos() const;
	_float					Compute_Distance_ToCam(class CGameObject* pObject);
#pragma endregion

#pragma region PICKING
public:
	POINT					Get_MousePoint();
	_bool					isPicked(_float3* pOut);
#pragma endregion

#pragma region SHADOW
	const _float4x4*		Get_ShadowLight_Matrix(D3DTS eType);
	HRESULT					Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc);
	HRESULT					Bind_Shadow_Resource(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName);
	void					Update_ShadowLight_Transform(const _fvector& vAt);
#pragma endregion

#pragma region GUIMANAGER
public:
	ImGuiContext*		Get_ImGuiContext();
	void					Add_GUI_Func(function<void()> func);
	void					Use_Gizmo(class CTransform* pTransform = nullptr);
	void					Render_Gizmo(const _fmatrix& Matrix);
#pragma endregion

#pragma region FRUSTRUM
public:
	const _float4*		Get_Frustrum_WorldPoints() const;
	_bool				IsIn_WorldSpace(_fvector vWorldPosition, _float fRange);	
	_bool				IsIn_WorldSpace(const BoundingBox* pBoundingBox);
	_bool				IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange);	
#pragma endregion

#pragma region CSM
	HRESULT				SetUp_ShadowLight(const _wstring& strLightTag);
	HRESULT				Bind_CSM_Resources(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pDistanceName);
	HRESULT				Bind_CSM_SRV(CShader* pShader, const _char* pConstantName);
	HRESULT				Begin_CSM();
	HRESULT				End_CSM();
#pragma endregion

public:
	HRESULT			Clear_Resource(_uint iLevelID);
	HRESULT			Clear_Memory();
	void			Release_Engine();

private:
	class CGraphic_Device*		m_pGraphic_Device = { nullptr };
	class CInput_Device*		m_pInput_Device = { nullptr };
	class CSound_Manager*		m_pSound_Manager = { nullptr };
	class CFont_Manager*		m_pFont_Manager = { nullptr };
	class CLevel_Manager*		m_pLevel_Manager = { nullptr };
	class CPrototype_Manager*	m_pPrototype_Manager = { nullptr };
	class CObject_Manager*		m_pObject_Manager = { nullptr };
	class CPooling_Manager*		m_pPooling_Manager = { nullptr };
	class COctoTree*			m_pOctoTree = { nullptr };
	class CTarget_Manager*		m_pTargetManager = { nullptr };
	class CRenderer*			m_pRenderer = { nullptr };
	class CLight_Manager*		m_pLight_Manager = { nullptr };
	class CCamera_Manager*		m_pCamera_Manager = { nullptr };
	class CTimer_Manager*		m_pTimer_Manager = { nullptr };
	class CPhysicsManager*		m_pPhysicsManager = { nullptr };
	class CEventBus*			m_pEventBus = { nullptr };
	class CPipeLine*			m_pPipeLine = { nullptr };
	class CPicking*				m_pPicking = { nullptr };
	class CShadow*				m_pShadow = { nullptr };
	class CGUIManager*			m_pGUIManager = { nullptr };
	class CFrustrum*			m_pFrustrum = { nullptr };
	class CCSM*					m_pCSM = { nullptr };

	_uint									m_iNumLevel = {};

public:
	virtual void Free() override;

};

NS_END