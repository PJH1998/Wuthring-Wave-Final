#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCamera_Manager final : public CBase
{
private:
	explicit CCamera_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCamera_Manager() = default;

public:
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, class CCamera* pCamera);
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg);
	
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const vector<ACTIONFRAME>& ActionFrames);
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const _char* pFilePath);
	void				Play_Action(const _wstring& strActionTag);

	HRESULT			Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag);

	_float				Get_CurrentCamera_Near();
	_float				Get_CurrentCamera_Far();

public:
	HRESULT			Initialize(_uint iNumLevel);
	void				Update(_float fTimeDelta);	// PipeLine Camera Matrix Update
	void				Late_Update(_float fTimeDelta);

	HRESULT			Clear_Resource(_uint iCurrentLevelID);

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	_uint						m_iNumLevel = {};
	typedef map<const _wstring, class CCamera*> CAMERA;
	CAMERA*						m_Cameras = { nullptr };
	class CCamera*				m_pMainCamera = { nullptr };

	typedef map<const _wstring, vector<ACTIONFRAME>> CAMERA_ACTION;
	CAMERA_ACTION			m_CameraActions;
	_wstring						m_strActionTag;
	_bool							m_isPlayAction = { false };
	_uint							m_iActionIndex = {};						// Action Index
	_float							m_fCurrentTrackPosition = {};			// Action Current Track Position

	_float4						m_vPreQuaternion = {};
	_float							m_fPreDistance = {};

	class CCamera*				m_pFreeCamera = { nullptr };
	_bool							m_isFree = { false };

private:
	class CCamera*		Find_Camera(_uint iLevelID, const _wstring& strCameraTag);

	void					Compute_Action(_float fTimeDelta);
	void					Compute_Pre();

	void					Ready_FreeCamera();

public:
	static		CCamera_Manager*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumLevel);
	virtual		void							Free();
};

NS_END