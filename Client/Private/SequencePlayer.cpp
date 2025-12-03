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

	// 1. Sequence Player를 GameSystem에 등록.
	m_pGameSystem->Register_SequencePlayer(this);

	if (FAILED(CGameObject::Initialize_Clone(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Players(pDesc)))
		return E_FAIL;

		
	

    return E_NOTIMPL;
}

void CSequencePlayer::Priority_Update(_float fTimeDelta)
{
}

void CSequencePlayer::Update(_float fTimeDelta)
{
}

void CSequencePlayer::Late_Update(_float fTimeDelta)
{
}

void CSequencePlayer::Render()
{
}

void CSequencePlayer::Render_Shadow()
{
}

void CSequencePlayer::Sorting_Target()
{
}

void CSequencePlayer::Toggle_LockOn()
{
}

HRESULT CSequencePlayer::Ready_Players(const SEQUENCEPLAYER_DESC* pDesc)
{
    return E_NOTIMPL;
}

HRESULT CSequencePlayer::Ready_Components(const SEQUENCEPLAYER_DESC* pDesc)
{
    return E_NOTIMPL;
}

CSequencePlayer* CSequencePlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CSequencePlayer::Clone(void* pArg)
{
    return nullptr;
}

void CSequencePlayer::Free()
{
}
