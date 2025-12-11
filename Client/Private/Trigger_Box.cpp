#include"ClientPch.h"
#include "Trigger_Box.h"
#include"GameSystem.h"
CTrigger_Box::CTrigger_Box(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

CTrigger_Box::CTrigger_Box(const CTrigger_Box& Prototype)
	:CGameObject(Prototype), m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CTrigger_Box::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger_Box::Initialize_Clone(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	m_iTriggerIndex = pDesc->iTriggerIndex;

	Ready_Components(pArg);

	_float4x4 Mat;
	switch (m_iTriggerIndex)
	{
	case 0:
		m_CamMatrix = make_pair(TEXT("Action_Asphodel_Barrens_Start"), make_pair(*m_pTransformCom->Get_WorldMatrixPtr(), false));
		break;

	case 2:
		m_CamMatrix = make_pair(TEXT("Action_Asphodel_Barrens_Meteo"), make_pair(*m_pTransformCom->Get_WorldMatrixPtr(), false));
		break;

	case 4:
		m_CamMatrix = make_pair(TEXT("Action_Asphodel_Barrens_Horizon"), make_pair(*m_pTransformCom->Get_WorldMatrixPtr(), true));
		break;
	case 21:
		XMStoreFloat4x4(&Mat, XMMatrixRotationY(1.6736f + 3.14f) * XMMatrixTranslation(3546.f, 173.f, 2931.f));
		m_CamMatrix = make_pair(TEXT("Action_False_Sonora"), make_pair(Mat, false));
		break;

	case 22:
		XMStoreFloat4x4(&Mat, XMMatrixRotationY(1.6736f + 3.14f) * XMMatrixTranslation(3546.f, 173.f, 2931.f));
		m_CamMatrix = make_pair(TEXT("Action_False_Sonora"), make_pair(Mat, false));
		break;
	case 23:
		XMStoreFloat4x4(&Mat, XMMatrixRotationY(XMConvertToRadians(177.5f)));
		m_CamMatrix = make_pair(TEXT("Action_False_Sonora_03"), make_pair(Mat, false));
		break;
	case 24:
		XMStoreFloat4x4(&Mat, XMMatrixRotationY(1.6736f + 3.14f + XMConvertToRadians(120.f)));
		m_CamMatrix = make_pair(TEXT("Action_False_Sonora"), make_pair(Mat, false));
		break;
	case 25:
		XMStoreFloat4x4(&Mat, m_pTransformCom->Get_WorldMatrix());
		m_CamMatrix = make_pair(TEXT("Action_False_Sonora_04"), make_pair(Mat, false));
		break;
	}


	Register_Trigger();


	if (pDesc->iTriggerIndex >= 21 && pDesc->iTriggerIndex <= 25)
	{
		m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
				m_pGameSystem->Show_InteractUI(m_pGameSystem->Get_SonoroText());
			});

		m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
				Collision_During();
			});

		m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::REMOVE, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
				m_pGameSystem->Hide_InteractUI(false);
			});
	}
	else
	{
		m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
				Collision_Enter();
			});
	}


	if (m_iTriggerIndex >= 22 && m_iTriggerIndex <= 25)
	{
		m_pGameSystem->TriggerRegister(m_iTriggerIndex + 100, [this](void* pArg) {
			m_IsTriggered = true;
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			});
	}

	return S_OK;
}

void CTrigger_Box::Priority_Update(_float fTimeDelta)
{
	if (m_iTriggerIndex > 20 && m_iTriggerIndex < 30)
	{
		if (m_IsTriggered)
		{
			m_pGameSystem->Change_Sonoro(true);
			m_IsTriggered = !m_IsTriggered;
			m_bOnCoolDown = true;
		}
	}
	if (m_bOnCoolDown)
	{
		m_fCoolDown += fTimeDelta;
		if (m_fCoolDown >= 6.f)
		{
			m_bOnCoolDown = !m_bOnCoolDown;
			m_fCoolDown = 0.f;
		}
	}
}

void CTrigger_Box::Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CTrigger_Box::Late_Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Render();
}

void CTrigger_Box::Ready_Components(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vExtends;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
	switch (m_iTriggerIndex)
	{
	case 34:
		m_pTempPtr = m_pGameSystem->Create_GrapplePoint(_float3(3396.9f, 321.6f, 2016.2f), UI_GRAPPLE_TYPE::ANCHOR);
		m_pGameSystem->Toggle_GrapplePoint(m_pTempPtr, false);
		break;
	}
}

void CTrigger_Box::Collision_Enter()
{
	if (m_IsTriggered)
		return;

	m_pGameSystem->OnTriggerActivate(m_iTriggerIndex);

	if (m_pTempPtr)
		m_pGameSystem->Toggle_GrapplePoint(m_pTempPtr, true);

	if (m_pSecondTempPtr)
		m_pGameSystem->Toggle_GrapplePoint(m_pSecondTempPtr, true);
}

void CTrigger_Box::Collision_During()
{
	if(m_pGameInstance->Get_DIKeyState(DIK_F) ==KEYSTATE::DOWN && !m_bOnCoolDown)
	{
		if (!m_pGameSystem->IsSonoro())
		{
			if (!m_pGameSystem->IsSonoro())
			{
				m_pGameSystem->OnTriggerActivate(m_iTriggerIndex);
			}
			else
				m_pGameSystem->OnTriggerActivate(m_iTriggerIndex + 100);
			m_pGameSystem->Hide_InteractUI(true);
		}
		else if (m_pGameInstance->Get_CurrentLevel() == ENUM_CLASS(LEVEL::HEAVEN))
		{
			/*if (!m_IsDoingPalette)
			{
				m_pGameSystem->Open_Game_OverflowPalette();
				m_pGameSystem->Hide_InteractUI(true);
				m_pGameSystem->Lock_Input_ToPlayer(true);
			}
			else
			{
				m_pGameSystem->Close_Game_OverflowPalette();
				m_pGameSystem->Show_InteractUI(TEXT("다채화"));
				m_pGameSystem->Lock_Input_ToPlayer(false);
			}
			m_IsDoingPalette = !m_IsDoingPalette;*/
		}
		else
			m_pGameSystem->OnTriggerActivate(m_iTriggerIndex + 100);

		m_pGameSystem->Hide_InteractUI(true);
	}
}

void CTrigger_Box::Collision_End()
{
}

void CTrigger_Box::Register_Trigger()
{
	m_pGameSystem->TriggerRegister(m_iTriggerIndex, [this](void* pArg) {
		_float4x4 Mat;
		switch (m_iTriggerIndex)
		{
		case 0:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;


		case 2:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;

		case 4:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;

		case 7:
			m_pGameSystem->Stop_Action();
			break;

		case 20:
			m_pGameInstance->Set_CurrentCamera_Far(600.f);
			break;

		case 21:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;
		case 22:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;
		case 23:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;
		case 24:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;
		case 25:
			m_pGameSystem->Play_Action(m_CamMatrix.first, XMLoadFloat4x4(&m_CamMatrix.second.first), m_CamMatrix.second.second);
			break;
		case 34:
			m_pGameSystem->Change_TimeRate(COLLISIONLAYER::PLAYER, 0.2f, 2.f);
			m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 0.2f, 2.f);
			m_pGameSystem->Play_QTE(_float2(-300.f, 200.f), UI_QTE_TYPE::TRIGGER_ROPE, UI_QTE_BTN::T);
			break;
		}
		m_IsTriggered = true;
		});
}

void CTrigger_Box::UI_Set(_bool B)
{
	B ?
		true :
		false;
}


CTrigger_Box* CTrigger_Box::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrigger_Box* pInstance = new CTrigger_Box(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTrigger_Box::Clone(void* pArg)
{
	CTrigger_Box* pInstance = new CTrigger_Box(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrigger_Box::Free()
{
	__super::Free();
	m_pTempPtr = nullptr;
	m_pSecondTempPtr= nullptr;
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pRigidbodyCom);
}