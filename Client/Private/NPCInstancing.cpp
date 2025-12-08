#include "ClientPch.h"
#include "NPCInstancing.h"
#include "NPCCell.h"
#include "GameSystem.h"

CNPCInstancing::CNPCInstancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor{ pDevice, pContext }
{
}

CNPCInstancing::CNPCInstancing(const CNPCInstancing& Prototype)
	: CActor{ Prototype }
{
}

HRESULT CNPCInstancing::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNPCInstancing::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	NPC_DESC* pDesc = static_cast<NPC_DESC*>(pArg);
	m_eCurLevel = pDesc->eCurLevel;
	m_pGameSystem = CGameSystem::GetInstance();
	Safe_AddRef(m_pGameSystem);
	Ready_Component(pDesc);
	Ready_InstanceCells(pDesc);
	
	m_fFaceSize = 1.f / 3;
	m_iFacePaddingCount = 3;
	return S_OK;
}

void CNPCInstancing::Priority_Update(_float fTimeDelta)
{
}

void CNPCInstancing::Update(_float fTimeDelta)
{
}

void CNPCInstancing::Late_Update(_float fTimeDelta)
{
	m_pModelInstanceCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom);
	m_pModelInstanceCom->FetchModelMatrices_FromCompute(m_pSkinningCom);
	m_pModelInstanceCom->Update_WorldInstances();
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CNPCInstancing::Render()
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
			if (FAILED(hr))
				m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::DEFAULT_NORMAL));
			else
				m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::NORMAL_TEX));
			//m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMINST::DEFAULT_NORMAL));
		}

		m_pModelInstanceCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}
}

void CNPCInstancing::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
}

void CNPCInstancing::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CNPCInstancing::Object_Func(const _wstring& wStrObjectTag)
{
}

HRESULT CNPCInstancing::Bind_Resources()
{
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_fFaceSize", &m_fFaceSize, sizeof(_float));
	m_pShaderCom->Bind_Value("g_iTexPaddingCount", &m_iFacePaddingCount, sizeof(_uint));
	return S_OK;
}

void CNPCInstancing::Ready_Component(NPC_DESC* pDesc)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("NPCInstancing/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("NPCInstancing/Com_ComputeShader");

	// Com_ComputeShaderCombining
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, pDesc->wstrCombiningPrototypeTag, TEXT("Com_ComputeShaderCombining"), reinterpret_cast<CComponent**>(&m_pSkinningCom), nullptr)))
		CRASH("NPCInstancing/Com_ComputeShaderCombining");

	// Com_ModelInstance
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelInstanceCom), nullptr)))
		CRASH("NPCInstancing/Com_Model");
}

void CNPCInstancing::Ready_InstanceCells(NPC_DESC* pDesc)
{
	vector<NPCINFO> NpcData = m_pGameSystem->Get_NpcData(pDesc->iNPCType);

	_uint iNumCells = m_pModelInstanceCom->Get_NumInstance();
	m_MeshTypePadding = m_pModelInstanceCom->Get_MeshOffset();
	_float eps = XMVectorGetX(g_XMEpsilon);
	for (_uint i = 0; i < iNumCells; i++)
	{
		vector<_uint> MeshType(m_MeshTypePadding.size());
		//memcpy(MeshType.data(), NpcData[i].MeshtypeIndices, sizeof(NpcData[i].MeshtypeIndices));
		if(NpcData[i].MeshtypeIndices[0] < 0)
		{
			for (_uint j = 0; j < MeshType.size() - 1; ++j)
			{
				MeshType[j] = static_cast<_uint>(m_pGameInstance->Rand(m_MeshTypePadding[j], m_MeshTypePadding[j + 1] - eps)) - m_MeshTypePadding[j];
			}
			auto iter = MeshType.end() - 1;
			(*iter) = static_cast<_uint>(m_pGameInstance->Rand(m_MeshTypePadding.back(), m_pModelInstanceCom->Get_NumMesh())) - m_MeshTypePadding.back();
		}
		else
		{
			memcpy(MeshType.data(), NpcData[i].MeshtypeIndices, sizeof(NpcData[i].MeshtypeIndices));
		}

		CNPCCell::DUMMYCELL_DESC CellDesc = {};
		CellDesc.pUpdateRootFunc = [this](const _string& strAnimationName, CTransform* pTransform, _float fTimeDelta, _float* pTrackPos, 
			_bool isRootMotion, _bool isRootMotionRotate, _bool isRootMotionTranslate, _float fRootMotionRate) {
				return m_pModelInstanceCom->Update_RootMotion(strAnimationName, pTransform, fTimeDelta, pTrackPos, isRootMotion, isRootMotionRotate, isRootMotionTranslate, fRootMotionRate);
			};
		CellDesc.pUpdateAnimStateFunc = [this](const _string& strAnimName, _fmatrix WorldMatrix, _uint iInstanceIndex, _float* pTrackPos, _uint* pPaddingIndices, _uint iPadding) {
			m_pModelInstanceCom->Update_AnimationState(strAnimName, WorldMatrix, iInstanceIndex, pTrackPos, pPaddingIndices, iPadding);
			};

		memcpy(CellDesc.szAnimationTag, NpcData[i].strAnimTag.c_str(), NpcData[i].strAnimTag.length());


		CellDesc.iInstanceIndex = i;
		
		CellDesc.vStartPos = NpcData[i].vPosition;
		CellDesc.vStartRot = NpcData[i].vRotation;
		CellDesc.fTrackPos = m_pGameInstance->Rand(0.f, 60.f);
		CellDesc.iFaceIndex = m_pGameInstance->Rand(0.f, 9.f - XMVectorGetX(g_XMEpsilon));
		CellDesc.iNumMeshType = 0;
		CellDesc.fSpeedPerSec = 10.f;
		CellDesc.fRotationPerSec = XMConvertToRadians(90.f);
		CellDesc.isCollide = NpcData[i].isCollide;
		if (false == m_MeshTypePadding.empty())
		{
			CellDesc.iNumMeshType = static_cast<_uint>(MeshType.size());
			CellDesc.iMeshTypes = MeshType.data();
		}
		m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), pDesc->wstrObjectPrototypeTag, ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &CellDesc);
		
	}
}

void CNPCInstancing::Update_AnimationState(const _string& strAnimName, _fmatrix WorldMatrix, _uint iInstanceIndex, _float* pTrackPos, _uint* pPaddingIndices)
{

	m_pModelInstanceCom->Update_AnimationState(strAnimName, WorldMatrix, iInstanceIndex, pTrackPos, pPaddingIndices);
}

CNPCInstancing* CNPCInstancing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNPCInstancing* pInstance = new CNPCInstancing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CNPCInstancing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNPCInstancing::Clone(void* pArg)
{
	CNPCInstancing* pClone = new CNPCInstancing(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CNPCInstancing (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CNPCInstancing::Free()
{
	__super::Free();

	Safe_Release(m_pGameSystem);
	Safe_Release(m_pModelInstanceCom);
	Safe_Release(m_pSkinningCom);
}