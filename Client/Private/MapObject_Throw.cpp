#include"ClientPch.h"
#include "MapObject_Throw.h"
#include"GameSystem.h"
CMapObject_Throw::CMapObject_Throw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_Throw::CMapObject_Throw(const CMapObject_Throw& Prototype)
	:CGameObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Throw::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Throw::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Components(pArg);

	m_pDetectRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
			int a = 0;
			//m_pCollideRigidbodyCom->Change_MotionType(EMotionType::Dynamic);
		});
    return S_OK;
}

void CMapObject_Throw::Priority_Update(_float fTimeDelta)
{
	//목표 방향은 화면의 정중앙.
}

void CMapObject_Throw::Update(_float fTimeDelta)
{
	m_pDetectRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);


	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{
		m_IsThrowed = true;
		XMStoreFloat3(&m_vStartPos, m_pTransformCom->Get_State(STATE::POSITION));


		m_pCollideRigidbodyCom->Impulse(m_vImpulse);

		//
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vStartPos), 1.f));
		m_IsThrowed = false;
		m_fThrowTime = 0.f;
	}

	if (!m_IsThrowed)
	{
		if (m_pGameInstance->GetCenterPos(&m_vTargetPos))
			int a = 0;

		_vector DisplaceMent = XMLoadFloat3(&m_vTargetPos) - XMLoadFloat3(&m_vStartPos);

		_float fTime = 1.f;

		_vector vGravityAccel = XMVectorSet(0.f, -9.81f, 0.f, 0.f);
		_vector vGravityDrop = vGravityAccel * 0.5f * fTime * fTime;
		XMStoreFloat3(&m_vImpulse, (DisplaceMent - vGravityDrop) / fTime);



		////_float GravityTerm = 0.5f * -9.8f * fTime * fTime;

		////_vector Velocity = XMVectorSet(DisplaceMent.m128_f32[0] - GravityTerm / fTime, DisplaceMent.m128_f32[1] - GravityTerm / fTime, DisplaceMent.m128_f32[2] - GravityTerm / fTime, 0.f);

		////Velocity *= 10.f;
		//XMStoreFloat3(&m_vImpulse, Velocity);
	}
	else 
	{
		m_fThrowTime += fTimeDelta;
		if(m_fThrowTime<1.f)
		{


			_vector vt = XMLoadFloat3(&m_vImpulse) * m_fThrowTime;

			_vector gt2 = 0.5f * XMVectorSet(0.f, -9.81f, 0.f, 0.f) * m_fThrowTime * m_fThrowTime;
			_vector NewPos = XMLoadFloat3(&m_vStartPos) + vt + gt2;
			m_pTransformCom->Set_State(STATE::POSITION, NewPos);
		}

		//_vector Pos = m_pTransformCom->Get_State(STATE::POSITION);
		//
		//if (T < 1.f)
		//	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMVectorLerp(XMLoadFloat3(&m_vStartPos), XMLoadFloat3(&m_vTargetPos), T), 1.f));

	}
	m_pCollideRigidbodyCom->Set_Transform(m_pTransformCom->Get_WorldMatrix());

	_float3 Grav(0.f, -9.8f, 0.f);
	m_pGameSystem->Req_Render_CurveTrace(m_vStartPos, m_vImpulse, Grav);
}

void CMapObject_Throw::Late_Update(_float fTimeDelta)
{
	if (m_IsThrowed)
		m_pCollideRigidbodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);
}

void CMapObject_Throw::Render()
{
	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CMapObject_Throw::Ready_Components(void* pArg)
{


	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);


	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	//// ShadowShader
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
	//	TEXT("Com_ShadowShader"), reinterpret_cast<CComponent**>(&m_pShadowShaderCom), nullptr)))
	//	CRASH("FAILED");

	_wstring ModelName = Model;
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();

	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");


	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(10.f, 10.f, 10.f);
	//플레이어 감지용 1개
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_DetectRigidbody"), reinterpret_cast<CComponent**>(&m_pDetectRigidbodyCom), &RigidbodyDesc);

	CRigidbody::BOXBODY_DESC Sibal{};
	XMStoreFloat4(&Sibal.vQuat, m_pTransformCom->Get_Quaternion());
	Sibal.eShape = SHAPE::BOX;
	XMStoreFloat3(&Sibal.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	Sibal.eType = EMotionType::Dynamic;
	Sibal.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	Sibal.vExtent = _float3(1.f, 1.f, 1.f);

	RigidbodyDesc.eType = EMotionType::Dynamic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::THROW);
	RigidbodyDesc.vExtent = _float3(1.f, 1.f, 1.f);
	//불타는 벽과 충돌 감지용
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_CollideRigidbody"), reinterpret_cast<CComponent**>(&m_pCollideRigidbodyCom), &Sibal);
}

void CMapObject_Throw::Collide()
{
	m_pDetectRigidbodyCom->IsActivate(false);
	//m_pCollideRigidbodyCom->IsActivate(false);
	SetActivate(false);

	//사운드 및 이펙트 호출.
}

void CMapObject_Throw::Graped()
{
	m_pCollideRigidbodyCom->Change_MotionType(EMotionType::Dynamic);
}

CMapObject_Throw* CMapObject_Throw::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Throw* pInstance = new CMapObject_Throw(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Throw");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Throw::Clone(void* pArg)
{
	CMapObject_Throw* pInstance = new CMapObject_Throw(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Throw (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMapObject_Throw::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pCollideRigidbodyCom);
	Safe_Release(m_pDetectRigidbodyCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGameSystem);
	
}
