#include "ClientPch.h"
#include "UI_LockOn.h"

// 락온UI 생성 자체는 그냥  데미지나 상호작용 만들듯이 만들고 (pooling으로 관리), 
// 보이는 위치를 항시 전달받아온 포인터의 좌표를 기반으로 (transformCom을 받아오든 등) 셰이더에 계속 갱신 
// 
// LockOnUI_Attach
// LockOnUI_Detach 와 같이 게임시스템에 제작

CUI_LockOn::CUI_LockOn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_LockOn::CUI_LockOn(const CUI_LockOn& Prototype)
	: CUI_Image(Prototype)
{
}

HRESULT CUI_LockOn::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_LockOn::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CUI_LockOn::Priority_Update(_float fTimeDelta)
{
}

void CUI_LockOn::Update(_float fTimeDelta)
{
}

void CUI_LockOn::Late_Update(_float fTimeDelta)
{
}

void CUI_LockOn::Render()
{
}

void CUI_LockOn::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

CUI_LockOn* CUI_LockOn::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{

}

CGameObject* CUI_LockOn::Clone(void* pArg)
{
	return nullptr;
}

void CUI_LockOn::Free()
{

}
