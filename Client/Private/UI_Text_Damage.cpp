#include "ClientPch.h"
#include "UI_Text_Damage.h"

CUI_Text_Damage::CUI_Text_Damage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Text(pDevice, pContext)
{
}

CUI_Text_Damage::CUI_Text_Damage(const CUI_Text_Damage& Prototype)
	: CUI_Text(Prototype)
{
}

HRESULT CUI_Text_Damage::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Text_Damage::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	
	TEXT_UI_TIMED_DESC* pDesc = static_cast<TEXT_UI_TIMED_DESC*>(pArg);
	
	m_isActivate = false;

	m_fLifeTime = pDesc->vLifeTime.y;
	m_iNumText = pDesc->strText.size();
	m_fLifeElapsed = pDesc->vLifeTime.x;
	m_isAutoDeactivate = true;

	return S_OK;
}

void CUI_Text_Damage::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CUI_Text_Damage::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_LifeTime(fTimeDelta);
	Update_Instances(fTimeDelta);
	__super::Update(fTimeDelta);
}

void CUI_Text_Damage::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Late_Update(fTimeDelta);
}

void CUI_Text_Damage::Render()
{
	if (!m_isActivate)
		return;

	__super::Render();
}

void CUI_Text_Damage::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// wake up via pooling. like as initialize
	// initilize 처럼 초기값 갱신에 사용하되, 중복 생성될 여지는 피해야 함.

	__super::Bind_Description(pArg);

	m_fLifeElapsed = 0.f;
	m_fLifeTime = static_cast<TEXT_UI_DESC*>(pArg)->vLifeTime.y;
	m_tTextDesc = *static_cast<TEXT_UI_DESC*>(pArg);
	m_isActivate = true;
}

void CUI_Text_Damage::Update_LifeTime(_float fTimeDelta)
{
	if (m_fLifeTime <= 0.f)
		return;

	m_fLifeElapsed += max(0.f, fTimeDelta);

	if (m_isAutoDeactivate && (m_fLifeElapsed >= m_fLifeTime))
	{
		m_isActivate = false;
		// if needs trigger when its deactive, declare here.
		static _uint iDmgIndex = 0;
		iDmgIndex++;
		std::cout << "[CUI_Text_Damage::Update_LifeTime] Damage Destroyed! : " << iDmgIndex << std::endl;
	}
}

void CUI_Text_Damage::Update_Instances(_float fTimeDelta)
{
	auto& textUIDesc = Get_TextUIDesc();
	auto& vecInstDescs = textUIDesc.vecInstanceDescs;

	// ksta : 각종 인스턴스 갱신용 정보들 (위치용 행렬, 커스텀 변수용 행렬 등등)
	//		  꺼내와서 가공하고 다시 재할당해주는 식으로 사용하면 됨

	
}

CUI_Text_Damage* CUI_Text_Damage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Text_Damage* pInstance = new CUI_Text_Damage(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Text_Damage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Text_Damage::Clone(void* pArg)
{
	CUI_Text_Damage* pInstance = new CUI_Text_Damage(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Text_Damage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Text_Damage::Free()
{
	__super::Free();
}
