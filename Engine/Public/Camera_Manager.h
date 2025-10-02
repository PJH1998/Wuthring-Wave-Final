#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCamera_Manager final : public CBase
{
private:
	explicit CCamera_Manager();
	virtual ~CCamera_Manager() = default;

public:
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, class CCamera* pCamera);
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg);
	
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const vector<ACTIONFRAME>& ActionFrames);
	HRESULT			Add_Camera_Action(const _wstring& strActionTag, const _char* pFilePath);
	void				Play_Action(const _wstring& strActionTag);

	HRESULT			Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag);
	void				Change_Distance(_float fDistance);
	void				Change_FixedDistance(_float fFixedDistance);

public:
	HRESULT			Initialize(_uint iNumLevel);
	void				Update(_float fTimeDelta);	// PipeLine¿¡ Camera Matrix °»½Å

	HRESULT			Clear_Resource(_uint iCurrentLevelID);

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

	_uint							m_iNumLevel = {};
	typedef map<const _wstring, class CCamera*> CAMERA;
	CAMERA*					m_Cameras = { nullptr };
	class CCamera*				m_pMainCamera = { nullptr };

	typedef map<const _wstring, vector<ACTIONFRAME>> CAMERA_ACTION;
	CAMERA_ACTION			m_CameraActions;
	_wstring						m_strActionTag;
	_bool							m_isPlayAction = { false };
	_uint							m_iActionIndex = {};						// Action Index
	_float							m_fCurrentTrackPosition = {};			// Action Current Track Position

	_float4						m_vPreQuaternion = {};
	_float							m_fPreDistance = {};

private:
	class CCamera*		Find_Camera(_uint iLevelID, const _wstring& strCameraTag);

	void					Compute_Action(_float fTimeDelta);
	void					Compute_Pre();

public:
	static		CCamera_Manager*		Create(_uint iNumLevel);
	virtual		void							Free();
};

NS_END