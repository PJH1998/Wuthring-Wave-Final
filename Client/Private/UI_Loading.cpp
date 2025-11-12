#include "ClientPch.h"
#include "Animator_UI.h"
#include "PlayerStatus.h"

#include "UI_Loading.h"

CUI_Loading::CUI_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Loading::CUI_Loading(const CUI_Loading& Prototype)
	:CCustom_UI(Prototype)
{
}

HRESULT CUI_Loading::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Loading::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);
	m_vecCachedUITransform.resize(1);
	Ready_Components(pArg);
	__super::Ready_Events();

	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Loading.json";
	Load_ChildObjects(strFilePath);


	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/LoadingTestFadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);


	// 랜덤하게 로딩 이미지 적용
	_uint iNumBG = Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames.size();
	m_iRandomBGIndex = static_cast<_uint>(m_pGameInstance->Rand(0.f, iNumBG - 0.001f));

	_wstring strRandBGName = Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames[m_iRandomBGIndex];
	
	for (auto& strBGName : Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames)
		Find_ChildObject(strBGName)->SetActivate(false);
	Find_ChildObject(strRandBGName)->SetActivate(true);

	return S_OK;
}

void CUI_Loading::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Loading::Update(_float fTimeDelta)
{
	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Loading::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Loading::Render()
{

	//__super::Render();
	//for (auto& child : m_vecChildObjects)
	//	child->Render();
	//static _uint i = 0;
	//i++;
	//cout << "[CUI_Loading::Render] Render Called! : " << i << endl;
}

HRESULT CUI_Loading::Ready_Components(void* pArg)
{
	return S_OK;
}

HRESULT CUI_Loading::Load_ChildObjects(_wstring strFilePath)
{
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();

	// parse json
	ifstream file(strFilePath);
	json jUITreeData = {};
	if (file.is_open()) { file >> jUITreeData; }
	CUSTOM_UITREE_DESC tLoadTreeDesc = {};
	from_json(jUITreeData, tLoadTreeDesc);

	// load objects
	vector<CGameObject*> vecLoadObjects = {};
	for (auto& loadDesc : tLoadTreeDesc.vecUIInfoDescs)
	{
		UI_INFO_DESC tLoadUIInfoDesc = loadDesc;

		// Transform ���� ������ ��, ���ȭ�Ͽ� �ݿ��ϰ�, (�ӽ÷�) �ڽ� ������Ʈ�ν� �߰��Ѵ�.
		_float3 vCurObjPos = tLoadUIInfoDesc.vPos;
		_float3 vCurObjRot = tLoadUIInfoDesc.vRot;
		_float3 vCurObjSca = tLoadUIInfoDesc.vSca;

		CGameObject* pCustomObj = nullptr;
		switch (tLoadUIInfoDesc.tUIDesc.iUIType)
		{
		case ENUM_CLASS(UI_TYPE::NONE):   pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));  break;
		case ENUM_CLASS(UI_TYPE::BUTTON): pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc)); break;
		default:            break;
		}
		m_vecChildObjects.push_back(static_cast<CCustom_UI*>(pCustomObj)); // ���ÿ� ����.. 


		HIERARCHY_OBJ_DESC tObjDesc = { };
		tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
		tObjDesc.strObjName = tLoadUIInfoDesc.tUIDesc.strUIName;

		_matrix matScale = XMMatrixScaling(vCurObjSca.x, vCurObjSca.y, vCurObjSca.z);
		_matrix matRotX = XMMatrixRotationX(DegreesToRadians(vCurObjRot.x));
		_matrix matRotY = XMMatrixRotationY(DegreesToRadians(vCurObjRot.y));
		_matrix matRotZ = XMMatrixRotationZ(DegreesToRadians(vCurObjRot.z));
		_matrix matRot = matRotZ * matRotY * matRotX;
		_matrix matTrans = XMMatrixTranslation(vCurObjPos.x, vCurObjPos.y, vCurObjPos.z);

		_matrix matWorld = matScale * matRot * matTrans;
		static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);
	}

	// re-define childs of objects
	for (auto& child : m_vecChildObjects)
	{
		CUSTOM_UI_DESC tChildDesc = child->Get_UIDesc();
		for (auto& otherChild : m_vecChildObjects)
		{
			CUSTOM_UI_DESC tOtherChildDesc = otherChild->Get_UIDesc();

			for (auto& childName : tChildDesc.vecChildNames)
			{
				if (childName == tOtherChildDesc.strUIName)
					child->Add_Child(otherChild);
			}
		}
	}

	// re-define childs of this(container)
	vector<CCustom_UI*> vecTrueChildObjects = {};
	for (auto& child : m_vecChildObjects)
	{
		if (child->Get_UIDesc().strParentName.empty())
			vecTrueChildObjects.push_back(child);
	}

	m_vecChildObjects = move(vecTrueChildObjects);

	return S_OK;
}

HRESULT CUI_Loading::Load_Animations(vector<_wstring> vecAnimFilePath)
{
	for (auto& animPath : vecAnimFilePath)
	{
		// parse json
		ifstream file(animPath);
		json jUIAnimData = {};
		if (file.is_open()) { file >> jUIAnimData; }
		CAnimator_UI::UI_ANIM_DESC tLoadAnimDesc = {};
		from_json(jUIAnimData, tLoadAnimDesc);

		CCustom_UI* pTargetObject = Find_ChildObject(tLoadAnimDesc.tUIDesc.strUIName);

		if (!pTargetObject)
			CRASH("Cannot find targetobject");
		CAnimator_UI* pTargetAnimator = dynamic_cast<CAnimator_UI*>(pTargetObject->Get_Component(L"Com_Animator_UI"));

		pTargetAnimator->Insert_Animation(tLoadAnimDesc);
	}

	return S_OK;
}


CUI_Loading* CUI_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Loading* pInstance = new CUI_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Loading::Clone(void* pArg)
{
	CUI_Loading* pInstance = new CUI_Loading(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Created : CUI_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Loading::Free()
{
	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	__super::Free();
}
