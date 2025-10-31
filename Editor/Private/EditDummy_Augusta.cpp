#include "EditorPch.h"
#include "EditDummy_Augusta.h"

CEditDummy_Augusta::CEditDummy_Augusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEditDummy{ pDevice, pContext }
{
}

CEditDummy_Augusta::CEditDummy_Augusta(const CEditDummy_Augusta& Prototype)
	: CEditDummy{ Prototype }
{
}

HRESULT CEditDummy_Augusta::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		CRASH("Failed Init_Prototype Dummy_Augu");

	return S_OK;
}

HRESULT CEditDummy_Augusta::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Failed to Cloned : Dummy_Wolf");

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMY_AUGU_DESC* pDesc = static_cast<DUMMY_AUGU_DESC*>( pArg );

	if (FAILED(Ready_Components(pDesc->PreTransformMatrix)))
		return E_FAIL;

	return S_OK;
}

void CEditDummy_Augusta::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Augusta::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_F10) == KEYSTATE::DOWN)
		m_IsEmissive = !m_IsEmissive;
}

void CEditDummy_Augusta::Late_Update(_float fTimeDelta)
{
	
	RENDERGROUP eRGIndex = m_IsEmissive ? RENDERGROUP::EMISSIVE : RENDERGROUP::DYNAMIC;
	m_pGameInstance->Add_Render_Object(eRGIndex, this);
//	m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);

	m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this);
}

void CEditDummy_Augusta::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_bool IsDynamicObject = true;
	m_pShaderCom->Bind_Value("g_IsDynamicObject", &IsDynamicObject, sizeof(_bool));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

		_bool HasNormal = { true };
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			HasNormal = false;

		_bool HasMetallic = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MetallicTexture", i, TEXTURETYPE::MASK)))
			HasMetallic = true;

		m_pShaderCom->Bind_Value("g_HasMetallic", &HasMetallic, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

		_uint iShaderIndex = m_IsEmissive ? 7 : 0;

		m_pShaderCom->Begin(iShaderIndex);
		m_pModelCom->Render(i);
	}
}

void CEditDummy_Augusta::Render_Shadow()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pShaderCom->Begin(5);

		m_pModelCom->Render(i);
	}
}

void CEditDummy_Augusta::Render_OutLine()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");

	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pShaderCom->Begin(6);

		m_pModelCom->Render(i);
	}
}

HRESULT CEditDummy_Augusta::Ready_Components(_fmatrix PreTransformMatrix)
{
	m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Character/CharacterDummy.dat");
	ASSERT_CRASH(m_pModelCom);

	m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
	ASSERT_CRASH(m_pShaderCom);

	return S_OK;
}

CEditDummy_Augusta* CEditDummy_Augusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditDummy_Augusta* pInstance = new CEditDummy_Augusta(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEditDummy_Augusta");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEditDummy_Augusta::Clone(void* pArg)
{
	CEditDummy_Augusta* pInstance = new CEditDummy_Augusta(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEditDummy_Augusta");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditDummy_Augusta::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
