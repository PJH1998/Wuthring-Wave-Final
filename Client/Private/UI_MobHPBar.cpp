#include "ClientPch.h"
#include "UI_MobHPBar.h"

#include "GameSystem.h"


#define KSTA_CUSTOMTEST

CUI_MobHPBar::CUI_MobHPBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_MobHPBar::CUI_MobHPBar(const CUI_MobHPBar& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem { CGameSystem::GetInstance() }
{
}

HRESULT CUI_MobHPBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_MobHPBar::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();


	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_MobHPBarDynamic.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Main_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);
	 


	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_MobHPBar", this);

	m_vecMobInfo.reserve(10);
	//m_vecMobKeys.reserve(10);

	return S_OK;
}

void CUI_MobHPBar::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	//if (m_vecMobInfo.empty())
	//{
	//	m_isActivate = false;
	//}


	m_vecMobInfo.clear();
	//m_vecMobKeys.clear();

	__super::Priority_Update(fTimeDelta);
}

void CUI_MobHPBar::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	// 이 단계에서 정보들 받아온다고 가정
#pragma region [NUMPAD 5] KSTA_UITEST_MOBHPBAR

	//static _bool isActiveMobHPBar = false;
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	//	isActiveMobHPBar = !isActiveMobHPBar;


#ifdef KSTA_CUSTOMTEST
	static _float fTmpHP = 70.f;
	_bool isHitTmp = false;
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1) == KEYSTATE::DOWN)
	{
		fTmpHP -= 10.f;
		std::cout << "[CUI_MobHPBar::Update] fTmpHP : " << fTmpHP << std::endl;
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2) == KEYSTATE::DOWN)
	{
		fTmpHP += 10.f;
		std::cout << "[CUI_MobHPBar::Update] fTmpHP : " << fTmpHP << std::endl;
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD3) == KEYSTATE::DOWN)
	{
		isHitTmp = true;
		std::cout << "[CUI_MobHPBar::Update] Triggered! " << fTmpHP << std::endl;
	}

	
	

	//if (isActiveMobHPBar)
	//{
	UI_MOBINFO_DESC tTmpDesc = {};

	const _uint iNumTestMobs = 5;

	for (_uint i = 0; i < iNumTestMobs; i++)
	{
		if (i == 3)
			continue;
		//_float3 fTestOffset = {
		//	m_pGameInstance->Rand(-10.f, 10.f),
		//	m_pGameInstance->Rand(-10.f, 10.f) - 10.f,
		//	m_pGameInstance->Rand(-10.f, 10.f)
		//};

		_float3 fTestOffset = { 0.f, -10.f * i, 0.f };

		tTmpDesc.vMobPos = fTestOffset;
		tTmpDesc.fMobCurHP = (i == 2) ? fTmpHP : 50.f;
		tTmpDesc.fMobMaxHP = 100.f;
		tTmpDesc.isAtkedCurFrame = isHitTmp;

		tTmpDesc.iMonsterPtrKey = i;

		m_pGameSystem->Update_MobStatus(tTmpDesc);
	}
#endif // KSTA_TEST



	//}
#pragma endregion


	__super::Update(fTimeDelta);
}

void CUI_MobHPBar::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CachedData(fTimeDelta);



	Update_CombinedMatrix();
	Update_CombinedDesc();

	Update_Instances();

	__super::Late_Update(fTimeDelta);
}

void CUI_MobHPBar::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_MobHPBar::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_vecMobInfo.clear();

	m_isActivate = true;
}

void CUI_MobHPBar::Update_MobStatus(const UI_MOBINFO_DESC& tDesc)
{
	m_vecMobInfo.push_back(tDesc);
	//m_vecMobKeys.push_back(tDesc.pMonsterKey);
}

void CUI_MobHPBar::PreAssign_ChildUIs()
{
	//m_pUI_HPFrame	= Find_ChildObject(L"InstHPFrame");
	//m_pUI_HPBar		= Find_ChildObject(L"InstHPBar");
	//m_pUI_SAFrame	= Find_ChildObject(L"InstSAFrame");
	//m_pUI_SABar		= Find_ChildObject(L"InstSABar");

	m_pUI_Frame		= Find_ChildObject(L"InstHPFrame");
	m_pUI_Line		= Find_ChildObject(L"InstHPLine");
	m_pUI_HB		= Find_ChildObject(L"InstHPBar");
	m_pUI_HB_Inv	= Find_ChildObject(L"InstHPBar_Inv");
}

void CUI_MobHPBar::PreAssign_Presets()
{
	_float4 vGreenHB		= { 0.188f, 1.000f, 0.608f, 1.000f };
	_float4 vYellowHB		= { 0.973f, 1.000f, 0.188f, 1.000f };
	_float4 vRedHB			= { 1.000f, 0.037f, 0.188f, 1.000f };
	_float4 vBackgroundHB   = { 0.200f, 0.200f, 0.200f, 1.000f };

	m_arrColorPresets[HPC_GREEN]		= vGreenHB;
	m_arrColorPresets[HPC_YELLOW]		= vYellowHB;
	m_arrColorPresets[HPC_RED]			= vRedHB;
	m_arrColorPresets[HRC_BACKGROUND]	= vBackgroundHB;
}

void CUI_MobHPBar::Update_CachedData(_float fTimeDelta)
{
	// 여기서, 받아온 정보들을 기반으로 캐싱,
	// vecMobHB 에 저장되어있는지 확인 후 있다면 그쪽에 push
	
	//m_vecMobRTInfo 이거 전부 false로 돌리고,
	//m_vecMobInfo 검사하며 없으면 추가, 있으면 갱신하며 true로 돌리기
	//	
	//이후 만약 안 돌려졌으면 그건 제거하기
	
	const _float fAtkedResetTime = 0.5f;



	// 캐시된 정보들, 업데이트 여부 초기값으로
	for (auto& mobRT : m_vecMobInfo_RT)
		mobRT.isUpdatedThisFrame = false;

	// 업데이트를 위한 순회
	for (auto& mobInfo : m_vecMobInfo)
	{
		// 캐시에서 찾으면	: 생존 시간 및 정보의 갱신
		// 캐시에 없으면	: 정보 추가
		_bool isFind_CachedData = false;
		for (auto& mobRTInfo : m_vecMobInfo_RT)
		{
			UI_MOBINFO_DESC& pCachedMobInfo = mobRTInfo.tInfoDesc;

			// 찾음! -> 정보 갱신
			if (pCachedMobInfo.iMonsterPtrKey == mobInfo.iMonsterPtrKey)
			{
				mobRTInfo.isUpdatedThisFrame = true;
				mobRTInfo.fCurElapsedTime += fTimeDelta;
				mobRTInfo.fCurStackedTime += fTimeDelta;
				mobRTInfo.tInfoDesc = mobInfo;

				for (_uint i = 0; i < mobRTInfo.isTimerActived.size(); i++)	// 타이머 활성화중이면 시간 갱신. 지정된 시간을 넘으면 종료
				{
					if (mobRTInfo.isTimerActived[i])		// 피격 피드백 진행중임?
					{
						mobRTInfo.fAtkedElapsedTime[i] += fTimeDelta;					// 지정된 시간을 넘기지 않았으면 갱신

						if (mobRTInfo.fAtkedElapsedTime[i] >= m_fMaxAtkedTimer)			// 지정된 시간을 넘기면 종료
						{
							mobRTInfo.fAtkedElapsedTime[i] = 0.f;
							mobRTInfo.isTimerActived[i] = false;
						}
					}
				}
				
				if (mobRTInfo.tInfoDesc.isAtkedCurFrame)								// 피격 감지 시 랜덤한거 피드백 타이머 시작
				{
					_uint iRand = (_uint)m_pGameInstance->Rand(0.f, 1.999f);			// ?? : 기준 변경 필요할수도.
					//mobRTInfo.fAtkedElapsedTime[iRand] = 0.f;
					mobRTInfo.isTimerActived[iRand] = true;
				}

				isFind_CachedData = true;
				break;
			}
		}

		// 못찾음! -> 추가
		if (!isFind_CachedData)
		{
			UI_MOBRT_DESC pDesc = {};
			pDesc.tInfoDesc = mobInfo;
			pDesc.isUpdatedThisFrame = true;
			pDesc.fCurElapsedTime = 0.f;
			pDesc.fCurStackedTime = 0.f;
			m_vecMobInfo_RT.push_back(pDesc);
		}
	}

	// 캐시에 남아있는데 갱신도 안됨! -> 제거
	m_vecMobInfo_RT.erase(
		remove_if(m_vecMobInfo_RT.begin(), m_vecMobInfo_RT.end(),
			[](const auto& mobRT)
			{
				return mobRT.isUpdatedThisFrame == false;
			}),	
		m_vecMobInfo_RT.end());
}

void CUI_MobHPBar::Update_Instances()
{
	const _float fDistancecPivot = 10.f;

	//PreAssign_Presets(); // FOR RUNTIME TEST, TEMP

	vector<CCustom_UI*> vecFloatingUIs = {};
	//vecFloatingUIs.push_back(m_pUI_HPFrame);
	//vecFloatingUIs.push_back(m_pUI_HPBar);
	//vecFloatingUIs.push_back(m_pUI_SAFrame);
	//vecFloatingUIs.push_back(m_pUI_SABar);
	vecFloatingUIs.push_back(m_pUI_HB);
	vecFloatingUIs.push_back(m_pUI_HB_Inv);
	vecFloatingUIs.push_back(m_pUI_Line);
	vecFloatingUIs.push_back(m_pUI_Frame);


	for (auto& floatingUI : vecFloatingUIs)
	{
		auto resizedDesc = floatingUI->Get_UIDesc();
		resizedDesc.vecInstanceDescs.resize(m_vecMobInfo.size());
		floatingUI->Set_UIDesc(resizedDesc);


		Calc_ApplyTargetPos(floatingUI);

		//if (floatingUI->Get_UIDesc().strUIName == L"InstHPBar")
			Calc_CamDistScale(floatingUI, fDistancecPivot);

	}

	Calc_HBEff();
}

void CUI_MobHPBar::Calc_ApplyTargetPos(CCustom_UI* pTargetUI)
{
	// 인스턴스 별 화면상의 위치만을 반영.

	auto targetDesc = pTargetUI->Get_UIDesc();
	auto& vecInstDesc = targetDesc.vecInstanceDescs;

	_float4x4 matCombined = pTargetUI->Get_CombinedMatrix();

	_float3 vCombinedSca = _float3(
		XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matCombined._11)))),
		XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matCombined._21)))),
		XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matCombined._31))))
	);


	for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	{
		// 1. 부모 대비 상대적인 Transform 을 고려, (로컬 이동량 / 부모 scale) 만큼 이동했다고 생각하면 될 것 같음. 
		//	 예시로 최종적인 부모 scale.x 가 250 이라면 로컬 x좌표가 1 증가한 것이 실제 250픽셀만큼 이동한 것과 같아야 함. 
		
		// calc mob pos.
		const _matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
		const _matrix matCamProj = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);

		const _float2 vScreenSize = { g_iWinSizeX, g_iWinSizeY };
		_vector vTargetWorldPos = XMVectorSetW(XMLoadFloat3(&m_vecMobInfo[i].vMobPos), 1.0f);

		_matrix matViewProj = matCamView * matCamProj;
		_vector vTargetClipRaw = XMVector3Transform(vTargetWorldPos, matViewProj);

		_float fTargetW = XMVectorGetW(vTargetClipRaw);
		_bool isBehindCamera = (fTargetW <= 0.0f);

		_float2 vScreenPos = {};

		if (!isBehindCamera)
		{
			_vector vTargetNDC = XMVector3TransformCoord(vTargetWorldPos, matViewProj);

			vScreenPos.x = (XMVectorGetX(vTargetNDC) + 1.0f) * 0.5f * vScreenSize.x - vScreenSize.x * 0.5f;
			vScreenPos.y = (1.0f - XMVectorGetY(vTargetNDC)) * 0.5f * vScreenSize.y - vScreenSize.y * 0.5f;
		}
		else
			vScreenPos = { -2000.f, -2000.f }; // 카메라 뒤면 밖으로 쫒아냄

		_float3 vPos = _float3(vScreenPos.x, -vScreenPos.y, 0.f);			// 최종적인 위치
		
		
		// apply inst desc.
		_float3 vCalcedDeltaPos = { vPos.x / vCombinedSca.x, vPos.y / vCombinedSca.y ,vPos.z / vCombinedSca.z};

		vecInstDesc[i].vSInstTrans = {
			/* vecInstDesc[i].vSInstTrans.x +*/ vCalcedDeltaPos.x,
			/* vecInstDesc[i].vSInstTrans.y +*/ vCalcedDeltaPos.y,
			/* vecInstDesc[i].vSInstTrans.z +*/ vCalcedDeltaPos.z,
			/* vecInstDesc[i].vSInstTrans.w  */ 1.f
		};
	}

	// apply desc. finally.
	pTargetUI->Set_UIDesc(targetDesc);

#pragma region old variant (bar type hp)

	// calc variant desc.
	//vector<_float4x4> vecVariantMat = {};
	//
	//
	//_float4 vVariantColor = { 1.f, 0.f, 1.f, 1.f };
	//_float4 vVariantEndColor = { 0.f, 1.f, 1.f, 1.f };
	//_float4 vVariantTmpColor = { 1.f, 1.f, 0.f, 1.f };
	//
	//_float4 vTransparentColor = { 0.f, 0.f, 0.f, 0.f };
	//
	//_float4 vHPBarColor		= { 0.816f, 0.302f, 0.231f, 0.900f };
	//_float4 vHPBarGradColor	= { 0.820f, 0.361f, 0.231f, 0.900f };
	//_float4 vHPBgColor		= { 0.500f, 0.500f, 0.500f, 0.800f };
	//_float4 vSABarColor		= { 0.900f, 0.900f, 0.900f, 0.900f };
	//_float4 vSABgColor		= { 0.500f, 0.500f, 0.500f, 0.800f };
	//
	//const _float4 vHPColor1 = { 1.f, .7f, .1f, 1.f };
	//const _float4 vHPColor2 = { 1.f, .2f, .0f, 1.f };
	//const _float4 vHPBackColor1 = { .8f, .8f, .8f, 1.f };
	//
	//const _float4 vSAColor = { 1.f, 1.f, 1.f, 1.f };   // before armor break
	//const _float4 vSABreakColor = { .9f, .8f, .3f, 1.f };
	//const _float4 vSABackColor = { 1.f, 1.f, 1.f, .3f };   // after armor break
	//
	//vecVariantMat.resize(m_vecMobInfo.size());
	//
	//if (pTargetUI->Get_UIDesc().strUIName == L"InstHPFrame" ||
	//	pTargetUI->Get_UIDesc().strUIName == L"InstHPBar")
	//{
	//	if (pTargetUI->Get_UIDesc().strUIName == L"InstHPBar")		// HP Bar
	//		for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	//		{
	//			//_float fMobMaxHP = m_pGameSystem->Get_MonsterInfo(m_vecMobKeys[i].c_str())->fMaxHp;
	//			_float fMobCurHP = m_vecMobInfo[i].fMobCurHP;
	//			_float fMobMaxHP = m_vecMobInfo[i].fMobMaxHP;
	//
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vHPColor1;
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vHPColor2;
	//			*reinterpret_cast<_float*>(&vecVariantMat[i]._31) = fMobCurHP / fMobMaxHP; // m_vecMobInfo[i].fMobCurHP/ m_vecMobInfo[i].fMobMaxHP;
	//		}
	//	else														// HP BG
	//		for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	//		{
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vHPBackColor1;
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vHPBackColor1;
	//			*reinterpret_cast<_float*>(&vecVariantMat[i]._31) = 1.f;
	//		}
	//
	//}
	//else
	//{
	//	if (pTargetUI->Get_UIDesc().strUIName == L"InstSABar")		// SA Bar
	//		for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	//		{
	//			_float fMobCurSA = m_vecMobInfo[i].fMobCurSA;
	//			_float fMobMaxSA = m_vecMobInfo[i].fMobMaxSA;
	//
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSAColor;
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vSAColor;
	//			*reinterpret_cast<_float*>(&vecVariantMat[i]._31) = fMobCurSA / fMobMaxSA; // m_vecMobInfo[i].fMobCurSA / m_vecMobInfo[i].fMobMaxSA;
	//		}
	//	else														// SA BG
	//		for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	//		{
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._11) = vSABackColor;
	//			*reinterpret_cast<_float4*>(&vecVariantMat[i]._21) = vSABackColor;
	//			*reinterpret_cast<_float*>(&vecVariantMat[i]._31) = 1.f;
	//		}
	//}

	// create variant matrix.
	//CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
	//	vecVariantMat,
	//	ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_PLAYER_HP),
	//	true
	//};

	// apply variant desc.
	//pTargetUI->Set_VariantUIDesc(tVariantDesc);  
#pragma endregion

}

void CUI_MobHPBar::Calc_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{
	const _float fMaxScaleFactor = 0.6f;


	auto targetDesc = pTargetUI->Get_UIDesc();
	auto& vecInstDesc = targetDesc.vecInstanceDescs;
	
	for (_uint i = 0; i < m_vecMobInfo.size(); i++)
	{
		_float4x4 matInstTransform = {};

		*reinterpret_cast<_float4*>(&matInstTransform._11) = vecInstDesc[i].vSInstRight;
		*reinterpret_cast<_float4*>(&matInstTransform._21) = vecInstDesc[i].vSInstUp;
		*reinterpret_cast<_float4*>(&matInstTransform._31) = vecInstDesc[i].vSInstLook;
		*reinterpret_cast<_float4*>(&matInstTransform._41) = vecInstDesc[i].vSInstTrans;

		// calculate inst scale
		_float3 vInstScale = {
			XMVectorGetX(XMVector3Length(XMLoadFloat4(&vecInstDesc[i].vSInstRight))),
			XMVectorGetX(XMVector3Length(XMLoadFloat4(&vecInstDesc[i].vSInstUp))),
			XMVectorGetX(XMVector3Length(XMLoadFloat4(&vecInstDesc[i].vSInstLook)))
		};

		// calculate target distance from camera
		const _matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
		_float3 vTargetPos = m_vecMobInfo[i].vMobPos;
		_float fDist = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - XMLoadFloat3(&vTargetPos)));

		// calc scale factor
		_float fScaleFactor = fPivotDistance / fDist;
		fScaleFactor = fScaleFactor > fMaxScaleFactor ? fMaxScaleFactor : fScaleFactor; // 최대 1배까지만 확대

		// apply scale
		vecInstDesc[i].vSInstRight = {
			vecInstDesc[i].vSInstRight.x / vInstScale.x * fScaleFactor,
			vecInstDesc[i].vSInstRight.y / vInstScale.y * fScaleFactor,
			vecInstDesc[i].vSInstRight.z / vInstScale.z * fScaleFactor,
			0.f
		};
		vecInstDesc[i].vSInstUp = {
			vecInstDesc[i].vSInstUp.x / vInstScale.x * fScaleFactor,
			vecInstDesc[i].vSInstUp.y / vInstScale.y * fScaleFactor,
			vecInstDesc[i].vSInstUp.z / vInstScale.z * fScaleFactor,
			0.f
		};
		vecInstDesc[i].vSInstLook = {
			vecInstDesc[i].vSInstLook.x / vInstScale.x * fScaleFactor,
			vecInstDesc[i].vSInstLook.y / vInstScale.y * fScaleFactor,
			vecInstDesc[i].vSInstLook.z / vInstScale.z * fScaleFactor,
			0.f
		};

		// 카메라 거리에 따라 translation 도 보정할 필요가 있을 듯
		// 현재는 보정이 없어 중점으로부터 벗어난 sa 바가 거리에 관계없이, 작아지더라도 hp 바와 일정한 거리를 유지하는 문제가 생김
		// hp바는 원점이고, sa바는 y축으로 -10만큼 아래에 존재. (screen space 상으로)

		// 얘 자체의 transform 을 통해 역산 가능할 것 같은데..
		_float3 vObjTrans = {}; XMStoreFloat3(&vObjTrans, static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"))->Get_State(STATE::POSITION));
		_float3 vObjSca = static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"))->Get_Scaled();

		vecInstDesc[i].vSInstTrans = {
			vecInstDesc[i].vSInstTrans.x - vObjTrans.x * (1.f - fScaleFactor) / vObjSca.x,
			vecInstDesc[i].vSInstTrans.y - vObjTrans.y * (1.f - fScaleFactor) / vObjSca.y,
			vecInstDesc[i].vSInstTrans.z - vObjTrans.z * (1.f - fScaleFactor) / vObjSca.z,
			1.f
		};
		// -15*(1-거리에따른Scale배율)/14
		// 멀어질 수록 -15로 떨어져있던 것이 0에 가까워져야 함

		// 이제 다시 적용
		// apply inst desc.
		pTargetUI->Set_UIDesc(targetDesc);
	}
}

void CUI_MobHPBar::Calc_HBEff()
{
	// 여기서 variant desc 재정의 및  변수 전달, 색상 수정 ㅇㅇ

	auto targetDesc = m_pUI_HB->Get_UIDesc();		// for Normal Desc
	auto targetInvDesc = m_pUI_HB_Inv->Get_UIDesc();		// for Normal Desc
	auto targetBGDesc = m_pUI_Frame->Get_UIDesc();	// for BG Normal Desc
	auto targetLineDesc = m_pUI_Line->Get_UIDesc();	// for Line Normal Desc
	auto& vecInstDesc = targetDesc.vecInstanceDescs;
	auto& vecInvInstDesc = targetInvDesc.vecInstanceDescs;
	auto& vecBGInstDesc = targetBGDesc.vecInstanceDescs;
	auto& vecLineInstDesc = targetLineDesc.vecInstanceDescs;

	vector<_float4x4> vecVariantMat = {};		// for Variant Desc
	vecVariantMat.resize(vecInstDesc.size());
	vector<_float4x4> vecInvVariantMat = {};		// for Variant Desc
	vecInvVariantMat.resize(vecInvInstDesc.size());
	vector<_float4x4> vecLineVariantMat = {};		// for BG Variant Desc
	vecLineVariantMat.resize(vecLineInstDesc.size());
	vector<_float4x4> vecBGVariantMat = {};		// for BG Variant Desc
	vecBGVariantMat.resize(vecBGInstDesc.size());

	// 갯수만큼 수정,

	for (_uint i = 0; i < m_vecMobInfo_RT.size(); i++)
	{
		// 변수 도출용
		const _float fInstCurHP = m_vecMobInfo_RT[i].tInfoDesc.fMobCurHP;		// 현재 체력 비율. 이에 따라 색상 변화, 애니메이션 주기 등을 제어할 것. 
		const _float fInstMaxHP = m_vecMobInfo_RT[i].tInfoDesc.fMobMaxHP;		// 현재 체력 비율. 이에 따라 색상 변화, 애니메이션 주기 등을 제어할 것. 
		const _float fMinHBTime = m_fMinRTTime;	// 최대 주기값
		const _float fMaxHBTime = m_fMaxRTTime;	// 최소 주기값
		const _float fColorBranchHP = 0.4f;	// 색상이 변할 기준점

		// >> 게산에 쓸 것 
		const _float fElapsedLifetime = m_vecMobInfo_RT[i].fCurElapsedTime;		// 인스턴스 별 경과 시간
		const _float fHPRatio = fInstCurHP / fInstMaxHP;						// 현재 체력 비율. 이에 따라 색상 변화, 애니메이션 주기 등을 제어할 것. 
		
		const _float fFillTime	= 0.35f;			// 매 주기 시작마다, 차오르는 데에 걸리는 시간
		//const _float fFlickStartHPRatio = 0.5f;		// 체력이 얼마나 남았을 때 부터 깜빡임을 시작할 건지 

		// 1. 시간에 따른 HB 관리
		const _float fCurHBTime = fMinHBTime + (fMaxHBTime - fMinHBTime) * fHPRatio;	// 계산된 현재 주기

		// 2. 각 인스턴스에 적용할 최종 계산된 변수
		//_float fClipXRate = (fElapsedLifetime / fFillTime > 1.f) ? 1 : fElapsedLifetime / fFillTime;	// 얼마나 가릴건지. 0 부터 fFillTime 까를 0~1로
		

		array<_float, 2>	arrClipXRate = {};
		for (_uint j = 0; j < arrClipXRate.size(); j++)		// 내부적으로 계산된 각 웨이브 별 델타타임을 기준으로 계산
			arrClipXRate[j] = (m_vecMobInfo_RT[i].fAtkedElapsedTime[j] / fFillTime > 1.f) ? 1 : m_vecMobInfo_RT[i].fAtkedElapsedTime[j] / fFillTime;	
		array<_float, 2>	arrAlphaRate = {};
		for (_uint j = 0; j < arrAlphaRate.size(); j++)		// 내부적으로 계산된 각 웨이브 별 델타타임을 기준으로 계산
			arrAlphaRate[j] = (m_vecMobInfo_RT[i].fAtkedElapsedTime[j] <= fFillTime) ?				// 얼마나 보였다 사라질건지
				m_vecMobInfo_RT[i].fAtkedElapsedTime[j] / fFillTime :									// filltime 도달 전 알파
				(m_fMaxAtkedTimer - m_vecMobInfo_RT[i].fAtkedElapsedTime[j]) / (m_fMaxAtkedTimer - fFillTime);		// filltime 도달 후 알파

		//if (i == 2)
		//	std::cout << "[arrAlphaRate] [0] : " << arrAlphaRate[0] << ", \t[1] : " << arrAlphaRate[1] << std::endl;

		_float fAlphaRate = (fElapsedLifetime <= fFillTime )?				// 얼마나 보였다 사라질건지
			fElapsedLifetime / fFillTime :									// filltime 도달 전 알파
			(fCurHBTime - fElapsedLifetime) / (fCurHBTime - fFillTime);		// filltime 도달 후 알파

		_float4 vColorRate = {};				// 시간 경과에 따라 어떤 색으로 변할건지
		_vector vColorRate_Load = (fHPRatio < fColorBranchHP) ?
			XMVectorLerp(
				XMLoadFloat4(&m_arrColorPresets[HPC_RED]), 
				XMLoadFloat4(&m_arrColorPresets[HPC_YELLOW]), 
				fHPRatio / fColorBranchHP):
			XMVectorLerp(
				XMLoadFloat4(&m_arrColorPresets[HPC_YELLOW]),
				XMLoadFloat4(&m_arrColorPresets[HPC_GREEN]), 
				(fHPRatio - fColorBranchHP) / (1.f - fColorBranchHP)
			);
		XMStoreFloat4(&vColorRate, vColorRate_Load);

		const _float fScaleMultiply = 2.5f;	//												<<<<<<<<<<<<<<<<<<<<
		_float fScaleRate = 1.f + fAlphaRate * (fScaleMultiply - 1.f);	// 얼마나 커졌다 작아질건지			

		_float fStackedTime = m_vecMobInfo_RT[i].fCurStackedTime;

		const _float fCoordSpeedX = 1.5f; //                                    			<<<<<<<<<<<<<<<<<<<<



		// 3. 실질 적용부
		auto& targetInst = vecInstDesc[i];		// 적용 할 인스턴스의 데이터
		auto& targetVariantMat = vecVariantMat[i];
		auto& targetInvInst = vecInvInstDesc[i];		// 적용 할 인스턴스의 데이터
		auto& targetInvVariantMat = vecInvVariantMat[i];
		auto& targetLineInst = vecLineInstDesc[i];
		auto& targetLineVariantMat = vecLineVariantMat[i];
		auto& targetBGInst = vecBGInstDesc[i];
		auto& targetBGVariantMat = vecBGVariantMat[i];

		targetInst.vClipTexcoordX = { 0.f, arrClipXRate[0] };
		targetInvInst.vClipTexcoordX = { 0.f, arrClipXRate[1] };

		*reinterpret_cast<_float4*>(&targetVariantMat._11)		= vColorRate;
		*reinterpret_cast<_float4*>(&targetVariantMat._21)		= vColorRate;
		*reinterpret_cast<_float*>(&targetVariantMat._31)		= arrAlphaRate[0];
		*reinterpret_cast<_float*>(&targetVariantMat._32)		= fScaleRate;
		*reinterpret_cast<_float*>(&targetVariantMat._33)		= fStackedTime;
		*reinterpret_cast<_float*>(&targetVariantMat._34)		= fCoordSpeedX;

		*reinterpret_cast<_float4*>(&targetInvVariantMat._11)	= vColorRate;
		*reinterpret_cast<_float4*>(&targetInvVariantMat._21)	= vColorRate;
		*reinterpret_cast<_float*>(&targetInvVariantMat._31)	= arrAlphaRate[1];
		*reinterpret_cast<_float*>(&targetInvVariantMat._32)	= fScaleRate;
		*reinterpret_cast<_float*>(&targetInvVariantMat._33)	= fStackedTime;
		*reinterpret_cast<_float*>(&targetInvVariantMat._34)	= fCoordSpeedX;

		*reinterpret_cast<_float4*>(&targetLineVariantMat._11)	= vColorRate;
		*reinterpret_cast<_float4*>(&targetLineVariantMat._21)	= vColorRate;
		*reinterpret_cast<_float*>(&targetLineVariantMat._31)	= 0.5f + fAlphaRate * 0.5f;
		*reinterpret_cast<_float*>(&targetLineVariantMat._32)	= 0.7f;					// 기본 선의 굵기
		*reinterpret_cast<_float*>(&targetLineVariantMat._33)	= fStackedTime;
		*reinterpret_cast<_float*>(&targetLineVariantMat._34)	= fCoordSpeedX;

		*reinterpret_cast<_float4*>(&targetBGVariantMat._11)	= m_arrColorPresets[HRC_BACKGROUND];
		*reinterpret_cast<_float4*>(&targetBGVariantMat._21)	= m_arrColorPresets[HRC_BACKGROUND];
		*reinterpret_cast<_float*>(&targetBGVariantMat._31)		= 0.5f + fAlphaRate * 0.5f;// fAlphaRate;
		*reinterpret_cast<_float*>(&targetBGVariantMat._32)		= fScaleRate;
		*reinterpret_cast<_float*>(&targetBGVariantMat._33)		= fStackedTime;
		*reinterpret_cast<_float*>(&targetBGVariantMat._34)		= fCoordSpeedX;

		// 체력이 특정 이상일 때에는 애니메이션 반영 없이 그대로
		//if (fHPRatio > fFlickStartHPRatio)
		//{
		//	targetInst.vClipTexcoordX = { 0.f, 1.f };
		//	*reinterpret_cast<_float*>(&targetVariantMat._31) = 1.f;		// alpha
		//	*reinterpret_cast<_float*>(&targetBGVariantMat._31) = 1.f;		// alpha
		//
		//	//m_vecMobInfo_RT[i].fCurElapsedTime = fFillTime;		// 조건 미충족으로 넘어갈 시 애니메이션 부자연스럽게 넘어감을 방지
		//}
		//else if (!m_vecMobInfo_RT[i].isPendingFlick)	// 체력이 내려갔지만 다음 계산상의 alpha가 1이 되기 전에는 계속 1이어야 함
		//{
		//	if (fAlphaRate >= 0.95f)
		//		m_vecMobInfo_RT[i].isPendingFlick = true;
		//
		//	targetInst.vClipTexcoordX = { 0.f, 1.f };
		//	*reinterpret_cast<_float*>(&targetVariantMat._31) = 1.f;
		//	*reinterpret_cast<_float*>(&targetBGVariantMat._31) = 1.f;
		//} 

		
		// END : 시간 초기화 관리
		if (m_vecMobInfo_RT[i].fCurElapsedTime >= fCurHBTime)
			m_vecMobInfo_RT[i].fCurElapsedTime = 0.f;
	}





	//UIFLAG_ENEMY_HP

	// 그 후 해당 desc에 할당

	CCustom_UI::VARIANTREADY_UI_DESC tVariantDesc = {
		vecVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ENEMY_HP),
		true
	};
	CCustom_UI::VARIANTREADY_UI_DESC tInvVariantDesc = {
		vecInvVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ENEMY_HP),
		true
	};
	CCustom_UI::VARIANTREADY_UI_DESC tLineVariantDesc = {
		vecLineVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ENEMY_HP),
		true
	};
	CCustom_UI::VARIANTREADY_UI_DESC tBGVariantDesc = {
		vecBGVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_ENEMY_HP),
		true
	};

	targetDesc.vecInstanceDescs = vecInstDesc;
	m_pUI_HB->Set_UIDesc(targetDesc);
	m_pUI_HB->Set_VariantUIDesc(tVariantDesc);

	targetInvDesc.vecInstanceDescs = vecInvInstDesc;
	m_pUI_HB_Inv->Set_UIDesc(targetInvDesc);
	m_pUI_HB_Inv->Set_VariantUIDesc(tInvVariantDesc);

	targetLineDesc.vecInstanceDescs = vecLineInstDesc;
	m_pUI_Line->Set_UIDesc(targetLineDesc);
	m_pUI_Line->Set_VariantUIDesc(tLineVariantDesc);

	m_pUI_Frame->Set_UIDesc(targetBGDesc);
	m_pUI_Frame->Set_VariantUIDesc(tBGVariantDesc);
}

CUI_MobHPBar* CUI_MobHPBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_MobHPBar* pInstance = new CUI_MobHPBar(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_MobHPBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_MobHPBar::Clone(void* pArg)
{
	CUI_MobHPBar* pInstance = new CUI_MobHPBar(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_MobHPBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_MobHPBar::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_MobHPBar");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
