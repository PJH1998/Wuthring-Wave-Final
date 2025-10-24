#include "EnginePch.h"
#include "Camera_Manager.h"

#include "GameInstance.h"

#include "FreeCamera.h"

CCamera_Manager::CCamera_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pGameInstance { CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CCamera_Manager::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, CCamera* pCamera)
{
    if (nullptr != Find_Camera(iLevelID, strCameraTag))
        return E_FAIL;

    m_Cameras[iLevelID].emplace(strCameraTag, pCamera);

    return S_OK;
}

HRESULT CCamera_Manager::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg)
{
    if (nullptr != Find_Camera(iLevelID, strCameraTag))
        return E_FAIL;

    CCamera* pCamera = static_cast<CCamera*>(m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::GAMEOBJECT, pArg));
    if (nullptr == pCamera)
        return E_FAIL;

    m_Cameras[iLevelID].emplace(strCameraTag, pCamera);

    return S_OK;
}

HRESULT CCamera_Manager::Add_Camera_Action(const _wstring& strActionTag, const vector<ACTIONFRAME>& ActionFrames)
{
    auto iter = m_CameraActions.find(strActionTag);
    if (iter != m_CameraActions.end())
        m_CameraActions.erase(iter);

    vector<ACTIONFRAME> Actions;

    for (size_t i = 0; i < ActionFrames.size(); ++i)
    {
        ACTIONFRAME ActionFrame = {};
        memcpy(&ActionFrame, &ActionFrames[i], sizeof(ACTIONFRAME));
        Actions.push_back(ActionFrame);
    }

    m_CameraActions.emplace(strActionTag, Actions);

    return S_OK;
}

HRESULT CCamera_Manager::Add_Camera_Action(const _wstring& strActionTag, const _char* pFilePath)
{
    ifstream InputFile(pFilePath, ios::binary);
    if (false == InputFile.is_open())
        return E_FAIL;

    _uint iNumActions = {};
    InputFile.read(reinterpret_cast<_char*>(&iNumActions), sizeof(_uint));

    vector<ACTIONFRAME> Actions;

    for (_uint i = 0; i < iNumActions; ++i)
    {
        ACTIONFRAME ActionFrame = {};
        InputFile.read(reinterpret_cast<_char*>(&ActionFrame), sizeof(ACTIONFRAME));
        Actions.push_back(ActionFrame);
    }

    m_CameraActions.emplace(strActionTag, Actions);

    InputFile.close();

    return S_OK;
}

void CCamera_Manager::Play_Action(const _wstring& strActionTag)
{
    auto iter = m_CameraActions.find(strActionTag);
    if (iter == m_CameraActions.end())
        return;

    m_strActionTag = strActionTag;
    m_isPlayAction = true;
    m_iActionIndex = 0;
    m_fCurrentTrackPosition = 0.f;
    Compute_Pre();
}

HRESULT CCamera_Manager::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag)
{
    CCamera* pCamera = Find_Camera(iLevelID, strCameraTag);
    if (nullptr == pCamera)
        return E_FAIL;

    Safe_Release(m_pMainCamera);
    m_pMainCamera = pCamera;
    Safe_AddRef(m_pMainCamera);

    return S_OK;
}

_float CCamera_Manager::Get_CurrentCamera_Near()
{
    if (nullptr == m_pMainCamera || true == m_isFree)
        return m_pFreeCamera->Get_Near();
    else
        return m_pMainCamera->Get_Near();
}

_float CCamera_Manager::Get_CurrentCamera_Far()
{
    if (nullptr == m_pMainCamera || true == m_isFree)
        return m_pFreeCamera->Get_Far();
    else
        return m_pMainCamera->Get_Far();
}

HRESULT CCamera_Manager::Initialize(_uint iNumLevel)
{
    m_iNumLevel = iNumLevel;
    m_Cameras = new CAMERA[m_iNumLevel];

	Ready_FreeCamera();

    return S_OK;
}

void CCamera_Manager::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_F1) == KEYSTATE::DOWN)
		m_isFree = !m_isFree;

	if (nullptr == m_pMainCamera || true == m_isFree)
	{
		m_pFreeCamera->Update(fTimeDelta);
	}
	else
	{
		if (false == m_isPlayAction)
			m_pMainCamera->Update(fTimeDelta);
		else
			Compute_Action(fTimeDelta);
	}
}

void CCamera_Manager::Late_Update(_float fTimeDelta)
{
	if (nullptr == m_pMainCamera || true == m_isFree)
	{
		m_pFreeCamera->Late_Update(fTimeDelta);
		m_pFreeCamera->Update_Matrix();
	}
	else
	{
		if (false == m_isPlayAction)
			m_pMainCamera->Late_Update(fTimeDelta);

		m_pMainCamera->Update_Matrix();
	}
}

HRESULT CCamera_Manager::Clear_Resource(_uint iCurrentLevelID)
{
    for (auto& Pair : m_Cameras[iCurrentLevelID])
        Safe_Release(Pair.second);
    m_Cameras[iCurrentLevelID].clear();

	Safe_Release(m_pMainCamera);
	m_pMainCamera = nullptr;

    return S_OK;
}

CCamera* CCamera_Manager::Find_Camera(_uint iLevelID, const _wstring& strCameraTag)
{
    if (m_iNumLevel <= iLevelID)
        return nullptr;

    auto iter = m_Cameras[iLevelID].find(strCameraTag);
    if (iter == m_Cameras[iLevelID].end())
        return nullptr;

    return iter->second;
}

void CCamera_Manager::Compute_Action(_float fTimeDelta)
{
    _float fDuration = m_CameraActions[m_strActionTag][m_iActionIndex].fDuration;
    m_fCurrentTrackPosition += fTimeDelta;
    // 1媛쒖쓽 Action ?꾨즺
    if (m_fCurrentTrackPosition > fDuration)
    {
        m_fCurrentTrackPosition = 0.f;
        m_vPreQuaternion = m_CameraActions[m_strActionTag][m_iActionIndex].vRotation;
        m_fPreDistance = m_CameraActions[m_strActionTag][m_iActionIndex].fDistance;
        ++m_iActionIndex;
        // Action End
        if (m_iActionIndex >= m_CameraActions[m_strActionTag].size())
        {
            m_isPlayAction = false;
            return;
        }
    }

    _float4 vRightQuaternion = m_CameraActions[m_strActionTag][m_iActionIndex].vRotation;
    _float fRightDistance = m_CameraActions[m_strActionTag][m_iActionIndex].fDistance;

    _float fRatio = m_fCurrentTrackPosition / fDuration;

    _vector vLerpQuaternion = XMQuaternionSlerp(XMLoadFloat4(&m_vPreQuaternion), XMLoadFloat4(&vRightQuaternion), fRatio);
    _float fLerpDistance = m_fPreDistance + (fRightDistance - m_fPreDistance) * fRatio;

    m_pMainCamera->Update_Action(vLerpQuaternion, fLerpDistance, fTimeDelta);
}

void CCamera_Manager::Compute_Pre()
{
    CTransform* pTransform = static_cast<CTransform*>(m_pMainCamera->Get_Component(TEXT("Com_Transform")));

    _vector vScale = {};
    _vector vRotation = {};
    _vector vTranslation = {};
    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, pTransform->Get_WorldMatrix());

    XMStoreFloat4(&m_vPreQuaternion, vRotation);
    m_fPreDistance = m_pMainCamera->Get_Distance();
}

void CCamera_Manager::Ready_FreeCamera()
{
	// Camera
	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 5000.f;
	CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
	CameraDesc.fSpeedPerSec = 1000.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fMouseSensor = 0.004f;
	
	m_pFreeCamera = CFreeCamera::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pFreeCamera);
	if (FAILED(m_pFreeCamera->Initialize_Clone(&CameraDesc)))
		CRASH("Free Camera");
}

CCamera_Manager* CCamera_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumLevel)
{
    CCamera_Manager* pInstance = new CCamera_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize(iNumLevel)))
    {
        MSG_BOX("Failed to Create : Camera_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCamera_Manager::Free()
{
    __super::Free();

    for (_uint i = 0; i < m_iNumLevel; ++i)
    {
        for (auto& Pair : m_Cameras[i])
            Safe_Release(Pair.second);
        m_Cameras[i].clear();
    }
    Safe_Delete_Array(m_Cameras);

    Safe_Release(m_pMainCamera);
    Safe_Release(m_pFreeCamera);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}
