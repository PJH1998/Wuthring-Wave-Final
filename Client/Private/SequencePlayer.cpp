#include "ClientPch.h"
#include "SequencePlayer.h"
#include "Character.h"
#include "GameSystem.h"

CSequencePlayer::CSequencePlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
	, m_pGameSystem { CGameSystem::GetInstance()}
{
	Safe_AddRef(m_pGameSystem);
}

CSequencePlayer::CSequencePlayer(const CSequencePlayer& Prototype)
	: CGameObject (Prototype)
	, m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CSequencePlayer::Initialize_Prototype()
{
	if (FAILED(CGameObject::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSequencePlayer::Initialize_Clone(void* pArg)
{
	SEQUENCEPLAYER_DESC* pDesc = static_cast<SEQUENCEPLAYER_DESC*>(pArg);

	m_eCurLevel = pDesc->eCurLevel;

	
	if (FAILED(CGameObject::Initialize_Clone(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Players(pDesc)))
		return E_FAIL;

	_fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Scale(pDesc->vScale);

	// Sequence Player를 GameSystem에 등록.
	m_pGameSystem->Register_SequencePlayer(this);

	m_isActivate = false;

    return S_OK;
}

void CSequencePlayer::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	CGameObject::Priority_Update(fTimeDelta);

	for (auto& pCharacter : m_SequenceCharacters)
		pCharacter->Priority_Update(fTimeDelta);
}

void CSequencePlayer::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	for (auto& pCharacter : m_SequenceCharacters)
	{
		pCharacter->Set_AutoLockOn(m_pTargetTransform, true);
		pCharacter->Update(fTimeDelta);
	}
	
}

void CSequencePlayer::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	for (auto& pCharacter : m_SequenceCharacters)
		pCharacter->Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (FAILED((m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this))))
		return;
#endif // DEBUG

}

void CSequencePlayer::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 둘다 아닌 경우에만.
	if ((ENUM_CLASS(COLLISIONLAYER::ENEMY) != iLayer))
		return;

	// Detect Body 탐지용 => switch
	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(iLayer);
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	switch (eLayer)
	{
	case COLLISIONLAYER::ENEMY:
		Process_CollideEnemy(pcallDesc);
		break;
	}
}

void CSequencePlayer::Summon_Squad_Near_Boss(CTransform* pTarget)
{
	if (m_Characters.empty()) return;
	if (nullptr == pTarget) return;

	// 1. 기존에 관리하던 활성화 리스트 초기화
	m_SequenceCharacters.clear();
	SetActivate(true);

	// 2. 보스 주변 위치 계산
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vBossPos = pTarget->Get_State(STATE::POSITION);
	_vector vBossLook = XMVectorSetY(pTarget->Get_State(STATE::LOOK), 0.f);
	vBossLook = XMVector3Normalize(vBossLook);
	_vector vBossRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vBossLook));
	m_pTransformCom->Set_State(STATE::POSITION, vBossPos);

	_float fDist = 2.0f;

	_vector vPlayerPos = m_pGameSystem->Get_PlayerPosition();

	cout << "Boss Pos " << vBossPos.m128_f32[0] << ", " << vBossPos.m128_f32[1] << ", " << vBossPos.m128_f32[2] << endl;

	for (_int i = 0; i < SEQUENCECHARACTER::SEQUENCE_END; ++i)
	{
		// 현재 인덱스의 캐릭터 가져오기
		CCharacter* pCharacter = m_Characters[i];
		_vector vSpawnPos = vBossPos;

		if (i == YUNO)
			vSpawnPos += (vBossLook * -1.f * fDist);
			
		if (i == AUGUSTA)
			vSpawnPos += (vBossRight * 1.f * fDist * 2.f);
			
		if (i == LUPA)
			vSpawnPos += (vBossRight * -1.f * fDist * 3.f);
			
		vSpawnPos = XMVectorSetY(vSpawnPos, XMVectorGetY(vBossPos) + 1.0f); // Y축 보정

		if (pCharacter)
		{
			// 타겟 바인딩 및 활성화
			pCharacter->Set_Position(XMVectorSetW(vSpawnPos, 1.f));
			
			pCharacter->Bind_TargetPosition(vBossPos);
			pCharacter->Activate(true);

			// 업데이트 리스트에 추가
			m_SequenceCharacters.push_back(pCharacter);
		}
	}



}


void CSequencePlayer::Binding_Trigger()
{
	
}

void CSequencePlayer::Render()
{
#ifdef _DEBUG
	m_pRigidbodyCom->Render();
#endif // _DEBUG
}

void CSequencePlayer::Render_Shadow()
{
}


void CSequencePlayer::Process_CollideEnemy(const CALLBACK_CLIENT* pcallDesc)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
	if (nullptr == pTargetTransform)
		return;
	{

		lock_guard<mutex> lock(m_Mutex);
		// 캐스팅 타입이 안맞아서 터질 수 있으므로 정확한 Rule을 지켜서 Desc을 설정해야함.
		// Vector 컨테이너에 넣어줄 거면 
		m_TargetTransforms.push_back(pTargetTransform);
		m_pTargetTransform = nullptr;
	}
}

HRESULT CSequencePlayer::Ready_Players(const SEQUENCEPLAYER_DESC* pDesc)
{
	ASSERT_CRASH(pDesc);
	m_Characters.resize(SEQUENCECHARACTER::SEQUENCE_END);

	CCharacter::CHARACTER_DESC CharacterDesc;
	CCharacter* pCharacter = { nullptr };

	for (_uint i = 0; i < pDesc->iPlayerCount; i++)
	{
		switch (i)
		{
		case SEQUENCECHARACTER::YUNO:
			CharacterDesc = pDesc->PlayerSpecs[SEQUENCECHARACTER::YUNO].CharacterDesc;
			pCharacter = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[i].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pCharacter);
			m_Characters[i] = pCharacter;
			pCharacter->SetActivate(false);
			break;

		case SEQUENCECHARACTER::AUGUSTA:
			CharacterDesc = pDesc->PlayerSpecs[SEQUENCECHARACTER::AUGUSTA].CharacterDesc;
			pCharacter = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[SEQUENCECHARACTER::AUGUSTA].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pCharacter);
			m_Characters[i] = pCharacter;
			pCharacter->SetActivate(false);
			break;
		case SEQUENCECHARACTER::LUPA:
			CharacterDesc = pDesc->PlayerSpecs[SEQUENCECHARACTER::LUPA].CharacterDesc;
			pCharacter = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[SEQUENCECHARACTER::LUPA].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pCharacter);
			m_Characters[i] = pCharacter;
			pCharacter->SetActivate(false);
			break;
		}
	}

    return S_OK;
}

HRESULT CSequencePlayer::Ready_Components(const SEQUENCEPLAYER_DESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(30.f, 15.f, 30.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	// 몬스터 탐지용?
	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
	});

    return S_OK;
}

HRESULT CSequencePlayer::Ready_Sequence(const SEQUENCEPLAYER_DESC* pDesc)
{
	m_SequenceCharacters.reserve(m_iSequenceMax); // 최대 3?

	return S_OK;
}

CSequencePlayer* CSequencePlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSequencePlayer* pInstance = new CSequencePlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CSequencePlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSequencePlayer::Clone(void* pArg)
{
	CSequencePlayer* pInstance = new CSequencePlayer(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CSequencePlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSequencePlayer::Free()
{
	CGameObject::Free();
	Safe_Release(m_pGameSystem);

	for (auto& pCharacter : m_Characters)
		Safe_Release(pCharacter);

	Safe_Release(m_pRigidbodyCom);
}
