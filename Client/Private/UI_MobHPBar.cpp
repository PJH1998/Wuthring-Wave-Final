#include "ClientPch.h"

#include "UI_MobHPBar.h"
#include "Animator_UI.h"


#define KSTA_UITEST_MOBHPPOS


CUI_MobHPBar::CUI_MobHPBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_MobHPBar::CUI_MobHPBar(const CUI_MobHPBar& Prototype)
	: CUI_Image(Prototype)
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
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_MobHP.json";
	Load_ChildObjects(strFilePath);

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Main_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);
	 

	//CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Initialize");


	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_MobHPBar", this);

	return S_OK;
}

void CUI_MobHPBar::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_MobHPBar::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	//CCustom_UI* pHpInstUI = Find_ChildObject(L"수정필요, 자식오브젝트 이름");
	//auto instDesc = pHpInstUI->Get_UIDesc().vecInstanceDescs;
	//
	//instDesc.resize(m_vecMobInfo.size());		// 주시중인 몹 갯수만큼 인스턴스 갯수 변경


	
	// 인스턴싱으로 불러오는 건 가능함. 이동도 가능함.
	// 근데 거리에 따른 Scale 조절 시에, 중점이 기준이 아닌 인스턴스들은
	// 각자의 중점에 따라 Scale이 조절되기에 한 부모가 Scale 조절되는 느낌이 아닌 지들 제각각 따로놀듯이 조절이 될 텐데
	// 이거 처리는 어떻게 하는가



	// 1. 이건 인스턴싱된 걸 움직이는게 아닌데
	// 후계산이 다 된 combined transform 을 조작하거나
	// 2. 이걸 인스턴싱에 적용한다? 그게 더 현실성 있을 듯
	// 부모의 combined transform 을 기준으로 역산한 뒤, 반영하고, 재계산하거나 하면 될 것 같음





	__super::Update(fTimeDelta);
}

void CUI_MobHPBar::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;





	//CCustom_UI* pHpInstUI = Find_ChildObject(L"수정필요, 자식오브젝트 이름");
	//auto instDesc = pHpInstUI->Get_UIDesc().vecInstanceDescs;

	//for (auto& mobInstDesc: instDesc)



	Update_CombinedMatrix();
	Update_CombinedDesc();

	//instDesc.resize(m_vecMobInfo.size());				// 주시중인 몹 갯수만큼 인스턴스 갯수 변경
	//Update_ApplyInstTargetPos(instDesc[i], );			// 해당 UI를 타겟 위치로 이동시킴.
	//Update_CamDistInstScale(mobInstDesc, 40.f);

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

#ifdef KSTA_UITEST_MOBHPPOS
	UI_MOBINFO_DESC tTmpDesc = {};
	m_vecMobInfo.push_back(tTmpDesc);
#endif // KSTA_UITEST_MOBHPPOS


	m_isActivate = true;
}

void CUI_MobHPBar::Ready_Presets()
{

}

void CUI_MobHPBar::Update_Instances()
{
	vector<CCustom_UI*> vecFloatingUIs = {};
	vecFloatingUIs.push_back(Find_ChildObject(L"InstHPFrame"));
	vecFloatingUIs.push_back(Find_ChildObject(L"InstHPBar"));
	vecFloatingUIs.push_back(Find_ChildObject(L"InstSAFrame"));
	vecFloatingUIs.push_back(Find_ChildObject(L"InstSABar"));

	for (auto& floatingUI : vecFloatingUIs)
	{
		auto resizedDesc = floatingUI->Get_UIDesc();
		resizedDesc.vecInstanceDescs.resize(m_vecMobInfo.size());
		floatingUI->Set_UIDesc(resizedDesc);


		Calc_ApplyTargetPos(floatingUI);

		_float fDistancecPivot = 40.f;
		Calc_CamDistScale(floatingUI, fDistancecPivot);
	}
}

void CUI_MobHPBar::Calc_ApplyTargetPos(CCustom_UI* pTargetUI)
{
	// 인스턴스 별 화면상의 위치만을 반영.

	auto targetDesc = pTargetUI->Get_UIDesc();
	auto& vecInstDesc = targetDesc.vecInstanceDescs;

	_float4x4 matCombined = pTargetUI->Get_CombinedMatrix();
	//_matrix matCombined = XMLoadFloat4x4(&pTargetUI->Get_CombinedMatrix());
	//_matrix matLocal = static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"))->Get_WorldMatrix();
	//_matrix matParentCombined_Load = XMMatrixInverse(nullptr, matLocal) * matCombined;
	//
	//_float4x4 matParentCombined = {}; XMStoreFloat4x4(&matParentCombined, matParentCombined_Load);	// 역산해 온 부모의 combined matrix, 이러면 굳이 부모 정보 안가져와도 됨
	//_float3 vParentSca = _float3(
	//	XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matParentCombined._11)))),
	//	XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matParentCombined._21)))),
	//	XMVectorGetX(XMVector3Length(XMLoadFloat3(reinterpret_cast<_float3*>(&matParentCombined._31))))
	//);
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

		vecInstDesc[i];
	}

	// apply desc. finally.
	pTargetUI->Set_UIDesc(targetDesc);
}

void CUI_MobHPBar::Calc_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{
	



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
