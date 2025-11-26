#include "ClientPch.h"
#include "DummyNPC.h"
#include "DummyCell.h"

CDummyNPC::CDummyNPC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor{ pDevice, pContext }
{
}

CDummyNPC::CDummyNPC(const CDummyNPC& Prototype)
	: CActor{ Prototype }
{
}

HRESULT CDummyNPC::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDummyNPC::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	DUMMYNPC_DESC* pDesc = static_cast<DUMMYNPC_DESC*>(pArg);
	m_eCurLevel = pDesc->eCurLevel;
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vStartPositions), 1.f));

	Ready_Component(pDesc);
	Ready_InstanceCells(pDesc);

	m_fFaceSize = 1.f / 3;
	m_iFacePaddingCount = 3;
	return S_OK;
}

void CDummyNPC::Priority_Update(_float fTimeDelta)
{
}

void CDummyNPC::Update(_float fTimeDelta)
{
}

void CDummyNPC::Late_Update(_float fTimeDelta)
{
	m_pModelInstanceCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom);
	m_pModelInstanceCom->FetchModelMatrices_FromCompute(m_pSkinningCom);
	m_pModelInstanceCom->Update_WorldInstances();
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CDummyNPC::Render()
{
	if (FAILED(Bind_Resources()))
		return;

	_uint iNumMesh = m_pModelInstanceCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	m_pModelInstanceCom->Bind_ConstantBuffers(m_pShaderCom);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelInstanceCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		HRESULT hr = m_pModelInstanceCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);
		m_pModelInstanceCom->Bind_OffsetMatrices(m_pShaderCom, "g_OffsetMatrices", i);
		if (i >= m_MeshTypePadding[MESHTYPE::FACE] && i < m_MeshTypePadding[MESHTYPE::HAIR])
			m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::FACE));
		else
		{
			if(FAILED(hr))
				m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::DEFAULT_NORMAL));
			else
				m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::NORMAL_TEX));
		}

		m_pModelInstanceCom->Render(i);
	}
}

void CDummyNPC::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
}

void CDummyNPC::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CDummyNPC::Object_Func(const _wstring& wStrObjectTag)
{
}

HRESULT CDummyNPC::Bind_Resources()
{
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_fFaceSize", &m_fFaceSize, sizeof(_float));
	m_pShaderCom->Bind_Value("g_iTexPaddingCount", &m_iFacePaddingCount, sizeof(_uint));
	return S_OK;
}

void CDummyNPC::Ready_Component(DUMMYNPC_DESC* pDesc)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("DummyNPC/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("DummyNPC/Com_ComputeShader");

	// Com_ComputeShaderSkinning
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, pDesc->wstrSkinningPrototypeTag, TEXT("Com_ComputeShaderSkinning"), reinterpret_cast<CComponent**>(&m_pSkinningCom), nullptr)))
		CRASH("DummyNPC/Com_ComputeShaderSkinning");

	// Com_ModelInstance
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelInstanceCom), nullptr)))
		CRASH("DummyNPC/Com_Model");
}

void CDummyNPC::Ready_InstanceCells(DUMMYNPC_DESC* pDesc)
{
#ifdef _DEBUG
	vector<_string> AnimationNames = m_pModelInstanceCom->Get_AnimationNames();
	_uint iNumAnimaiton = AnimationNames.size();
#endif // _DEBUG
	_float3 vPosition{};
	XMStoreFloat3(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
	_uint iNumCells = m_pModelInstanceCom->Get_NumInstance();
	m_MeshTypePadding = m_pModelInstanceCom->Get_MeshOffset();
	_float eps = XMVectorGetX(g_XMEpsilon);
	for (_uint i = 0; i < iNumCells; i++)
	{
		vector<_uint> MeshType(m_MeshTypePadding.size());
		for (_uint j = 0; j < MeshType.size() - 1; ++j)
		{
			MeshType[j] = static_cast<_uint>(m_pGameInstance->Rand(m_MeshTypePadding[j], m_MeshTypePadding[j + 1] - eps)) - m_MeshTypePadding[j];
		}
		auto iter = MeshType.end() - 1;
		(*iter) = static_cast<_uint>(m_pGameInstance->Rand(m_MeshTypePadding.back(), m_pModelInstanceCom->Get_NumMesh())) - m_MeshTypePadding.back();

		CDummyCell::DUMMYCELL_DESC CellDesc = {};
		CellDesc.pUpdateRootFunc = [this](const _string& strAnimationName, CTransform* pTransform, _float fTimeDelta, _float* pTrackPos, 
			_bool isRootMotion, _bool isRootMotionRotate, _bool isRootMotionTranslate, _float fRootMotionRate) {
				return m_pModelInstanceCom->Update_RootMotion(strAnimationName, pTransform, fTimeDelta, pTrackPos, isRootMotion, isRootMotionRotate, isRootMotionTranslate, fRootMotionRate);
			};
		CellDesc.pUpdateAnimStateFunc = [this](const _string& strAnimName, _fmatrix WorldMatrix, _uint iInstanceIndex, _float* pTrackPos, _uint* pPaddingIndices, _uint iExtra) {
			m_pModelInstanceCom->Update_AnimationState(strAnimName, WorldMatrix, iInstanceIndex, pTrackPos, pPaddingIndices, iExtra);
			};

#ifdef _DEBUG
		memcpy(CellDesc.szAnimationTag, AnimationNames[i % iNumAnimaiton].c_str(), AnimationNames[i % iNumAnimaiton].length());
#endif // _DEBUG

		CellDesc.iInstanceIndex = i;
		
		CellDesc.vStartPos = vPosition;
		CellDesc.vStartPos.x -= i * 1.f;
		CellDesc.fTrackPos = m_pGameInstance->Rand(0.f, 30.f);
		CellDesc.iNumMeshType = 0;
		CellDesc.fSpeedPerSec = 10.f;
		CellDesc.fRotationPerSec = XMConvertToRadians(90.f);
		CellDesc.isCollide = true;
		if (false == m_MeshTypePadding.empty())
		{
			CellDesc.iNumMeshType = static_cast<_uint>(MeshType.size());
			CellDesc.iMeshTypes = MeshType.data();
		}
		m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), pDesc->wstrObjectPrototypeTag, ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &CellDesc);
		
	}
}

CDummyNPC* CDummyNPC::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDummyNPC* pInstance = new CDummyNPC(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDummyNPC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDummyNPC::Clone(void* pArg)
{
	CDummyNPC* pClone = new CDummyNPC(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CDummyNPC (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CDummyNPC::Free()
{
	__super::Free();

	Safe_Release(m_pModelInstanceCom);
	Safe_Release(m_pSkinningCom);
}