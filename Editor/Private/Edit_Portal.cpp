#include"EditorPch.h"
#include "Edit_Portal.h"

CEdit_Portal::CEdit_Portal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

HRESULT CEdit_Portal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_Portal::Initialize_Clone(void* pArg)
{
	Ready_Components(pArg);
    return S_OK;
}

void CEdit_Portal::Priority_Update(_float fTimeDelta)
{
}

void CEdit_Portal::Update(_float fTimeDelta)
{
	//VTXPOSTEX 셰이더 사용하고. 디퓨즈는 모르겠지만 마스킹을 이용해서 Texcoord 밀기.
	//디졸브 수치에 비례해서 색 바꾸는 것처럼 마스킹의 Texcoord에 따라서 이미시브 효과 등 넣어보기?

	ImGui::Begin("Portal");

	ImGuiID Portal = ImGui::GetID("Container");
	ImGui::BeginChildFrame(Portal, ImVec2(100, 200));

	for (_uint i = 0; i < m_pShaderCom->Get_PassCount(); ++i)
	{
		if (ImGui::Button(m_pShaderCom->Get_PassName(i)))
			m_iShaderPassIndex = i;
	}
	ImGui::EndChildFrame();
	//DirectX::
	ImGui::End();
}

void CEdit_Portal::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_Portal::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pVIBufferCom->Bind_Resources();
	m_pShaderCom->Begin(m_iShaderPassIndex);
	m_pVIBufferCom->Render();
}

void CEdit_Portal::Ready_Components(void* pArg)
{

}

CEdit_Portal* CEdit_Portal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_Portal* pInstance = new CEdit_Portal(pDevice, pContext);
	pInstance->Initialize_Prototype();
    return pInstance;
}

CGameObject* CEdit_Portal::Clone(void* pArg)
{
	CEdit_Portal* pInstance = new CEdit_Portal(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
		CRASH("Failed");
	return pInstance;
}


void CEdit_Portal::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
