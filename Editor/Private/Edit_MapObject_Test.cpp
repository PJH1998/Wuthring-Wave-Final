#include"EditorPch.h"
#include "Edit_MapObject_Test.h"
CEdit_MapObject_Test::CEdit_MapObject_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_MapObject_Test::CEdit_MapObject_Test(const CEdit_MapObject_Test& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Test::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_MapObject_Test::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_pGameInstance->Rand(-200.f, 200.f), m_pGameInstance->Rand(-200.f, 200.f), m_pGameInstance->Rand(-200.f, 200.f), 1.f));
	return S_OK;
}

void CEdit_MapObject_Test::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Test::Update(_float fTimeDelta)
{

}

void CEdit_MapObject_Test::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::BLEND, this);
	if (m_pGameInstance->Get_DIKeyState(DIK_L) == KEYSTATE::DOWN)
		m_iIndex = 1;
	if (m_pGameInstance->Get_DIKeyState(DIK_P) == KEYSTATE::DOWN)
		m_iIndex = 2;
	if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
		m_iIndex = 3;
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
		m_iIndex = 0;
	//아니면 여기에서 Late Render를 하기 전에 LOD 파악해서 바꿔치기 하는 방법도 존재. <- 여기다가 하는 게 좀 더 좋을듯.
	m_pGameInstance->Add_To_RenderTest(m_iIndex, this);
	//m_pModelCom->Render(0, 0);
}

void CEdit_MapObject_Test::Render()
{
}

void CEdit_MapObject_Test::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	if (m_pModelCom->Get_MeshState(iIndex) != LOADSTATE::LOADED)
	{
		//여기서 Late Render같은 곳에 추가해버리는 코드 추가?.
		//return;
	}
	//ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map"), iIndex);
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	_bool HasNormal = { true };
	_bool HasMask = { true };

	for (_uint i = 0; i < m_pModelCom->Get_NumMesh(iIndex); ++i)
	{

		if (m_pModelCom->Is_Overed(iIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", iIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}


		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);

		m_pModelCom->Render(iIndex, i);
	}
}

void CEdit_MapObject_Test::Render_Shadow()
{
}

void CEdit_MapObject_Test::Set_ImGuiOption()
{
}

HRESULT CEdit_MapObject_Test::Ready_Component(void* pArg)
{
	BUFFER_TEST* pDesc = static_cast<BUFFER_TEST*>(pArg);

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::MAP), StringToWString(pDesc->ModelName),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;
	return S_OK;
}

CEdit_MapObject_Test* CEdit_MapObject_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Test::Clone(void* pArg)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Test::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}