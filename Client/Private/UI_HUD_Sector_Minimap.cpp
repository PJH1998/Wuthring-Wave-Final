#include "ClientPch.h"
#include "Animator_UI.h"
#include "UI_HUD_Sector_Minimap.h"
#include "GameSystem.h"

CUI_HUD_Sector_Minimap::CUI_HUD_Sector_Minimap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_HUD_Sector_Minimap::CUI_HUD_Sector_Minimap(const CUI_HUD_Sector_Minimap& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
}

HRESULT CUI_HUD_Sector_Minimap::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_HUD_Sector_Minimap::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_Sector_Minimap.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);


	//static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Initialize");

	//m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"Root_HUD_Sector_Minimap", this);

	return S_OK;
}

void CUI_HUD_Sector_Minimap::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_HUD_Sector_Minimap::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	// implement
	Update_TargetDegrees();

	__super::Update(fTimeDelta);
}

void CUI_HUD_Sector_Minimap::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	Update_Instances();

	__super::Late_Update(fTimeDelta);
}

void CUI_HUD_Sector_Minimap::Render()
{
	if (!m_isActivate)
		return;
}

HRESULT CUI_HUD_Sector_Minimap::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_HUD_Sector_Minimap::PreAssign_ChildUIs()
{
	m_pRUI_All					= Find_ChildObject(L"Sub_All");
	
	m_pUI_SectorLT_IconProps	= Find_ChildObject(L"SectorLT_IconProps");
	m_pUI_SectorLT_Minimap		= Find_ChildObject(L"SectorLT_Minimap");
	
	m_pUI_InstIcons				= Find_ChildObject(L"LT_InstIcons");
	m_pUI_InstMinimapBG			= Find_ChildObject(L"InstMinimapBG");
	m_pUI_InstCamAndPlayer		= Find_ChildObject(L"InstCamAndPlayer");

	//m_pUI_LT_Minimap_StaticBG	= Find_ChildObject(L"LT_Minimap_StaticBG");
	//m_pUI_LT_Minimap_TurnPoint	= Find_ChildObject(L"LT_Minimap_TurnPoint");
}

void CUI_HUD_Sector_Minimap::PreAssign_Presets()
{
}

void CUI_HUD_Sector_Minimap::Update_TargetDegrees()
{
	_float4x4 camViewMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);

	_float3 camViewLook = { camViewMatrix._13, camViewMatrix._23, camViewMatrix._33 };			
	_vector camLookDir = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&camViewLook), 0.f));
	_float dot = clamp(XMVectorGetX(XMVector3Dot(camLookDir, XMVectorSet(0.f, 0.f, 1.f, 0.f))), 0.f, 1.f);	// z = 1 단위벡터 기준 사잇

	_float camLookDirX = XMVectorGetX(camLookDir);
	_float camLookDirZ = XMVectorGetZ(camLookDir);

	_float camDegreeByY = (360.f - RadiansToDegrees(atan2f(camLookDirX, camLookDirZ)));				// 사잇각의 degree 화 및 -+ 데이터 포함. 그냥 acos는 -+ 정보포함X
	if (camDegreeByY >= 360.f)	camDegreeByY = camDegreeByY - 360.f;
	if (camDegreeByY < 0.f)		camDegreeByY = camDegreeByY + 360.f;
	
	m_fCamDirDegree = camDegreeByY;

	//std::cout << "[UI_HUD_Sector_Minimap::Update_TargetDegrees] Target Degree : " << camDegreeByY << std::endl;

	// 이후 플레이어 방향 정보 가져올 수 있게 된다면 그에 맞게 회전값 적용
	m_fPlayerDirDegree = camDegreeByY;
}

void CUI_HUD_Sector_Minimap::Update_Instances()
{
	auto& camPlayerDesc		= m_pUI_InstCamAndPlayer->Get_UIDesc();
	auto& camPlayerInstDesc = camPlayerDesc.vecInstanceDescs;


	enum MIMIMAP_INST_INDEX { INST_CAMERA, INST_PLAYER, INST_END };

	for (_uint i = 0; i < INST_END; i++)
	{
		_float4x4 matTransform = {};

		_float fTargetDegree = {};
		
		switch (i)
		{
		case INST_CAMERA:	fTargetDegree = m_fCamDirDegree;		break;
		case INST_PLAYER:	fTargetDegree = m_fPlayerDirDegree;		break;
		}

		*reinterpret_cast<_float4*>(&matTransform._11) = camPlayerInstDesc[i].vSInstRight;
		*reinterpret_cast<_float4*>(&matTransform._21) = camPlayerInstDesc[i].vSInstUp;
		*reinterpret_cast<_float4*>(&matTransform._31) = camPlayerInstDesc[i].vSInstLook;
		*reinterpret_cast<_float4*>(&matTransform._41) = camPlayerInstDesc[i].vSInstTrans;


		_vector vSca, vRotQuat, vPos;
		XMMatrixDecompose(&vSca, &vRotQuat, &vPos, XMLoadFloat4x4(&matTransform));

		_matrix matRot = XMMatrixRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 0.f), DegreesToRadians(fTargetDegree));

		_matrix matResult = XMMatrixScalingFromVector(vSca) * matRot * XMMatrixTranslationFromVector(vPos);
		_float4x4 matStoreResult = {}; XMStoreFloat4x4(&matStoreResult, matResult);
		//OutPutDebugMatrix(L"MatResult", matStoreResult); 


		camPlayerInstDesc[i].vSInstRight	= *reinterpret_cast<_float4*>(&matStoreResult._11);
		camPlayerInstDesc[i].vSInstUp		= *reinterpret_cast<_float4*>(&matStoreResult._21);
		camPlayerInstDesc[i].vSInstLook		= *reinterpret_cast<_float4*>(&matStoreResult._31);
		camPlayerInstDesc[i].vSInstTrans	= *reinterpret_cast<_float4*>(&matStoreResult._41);
	}
}

CUI_HUD_Sector_Minimap* CUI_HUD_Sector_Minimap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_HUD_Sector_Minimap* pInstance = new CUI_HUD_Sector_Minimap(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_HUD_Sector_Minimap");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_HUD_Sector_Minimap::Clone(void* pArg)
{
	CUI_HUD_Sector_Minimap* pInstance = new CUI_HUD_Sector_Minimap(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_HUD_Sector_Minimap");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_HUD_Sector_Minimap::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_HUD_Sector_Minimap");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
