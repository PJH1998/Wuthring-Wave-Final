#include "EnginePch.h"
#include "Model.h"
#include "GameInstance.h"

#include "Mesh.h"
#include "MeshMaterial.h"
#include "Bone.h"
#include "Animation.h"
#include "Channel.h"
#include "ComputeShader.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
    : CComponent { Prototype },
	m_eType { Prototype.m_eType },
	m_iNumMeshes { Prototype.m_iNumMeshes },
	m_Meshes { Prototype.m_Meshes },
	m_iNumMaterials { Prototype.m_iNumMaterials},
	m_Materials { Prototype.m_Materials },
	m_PreTransformMatrix { Prototype.m_PreTransformMatrix },
	m_vPreRootPosition { Prototype.m_vPreRootPosition },
	m_RootMatrix{ Prototype.m_RootMatrix },
	m_iRootBoneIndex{ Prototype.m_iRootBoneIndex },
	m_iNumAnimations { Prototype.m_iNumAnimations },
	m_AnimationNameToIndex { Prototype.m_AnimationNameToIndex},
	m_Buffers {Prototype.m_Buffers},
	m_SRVs { Prototype.m_SRVs }
{
	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pBone : Prototype.m_Bones)
		m_Bones.push_back(pBone->Clone());

	for (auto& Pair : Prototype.m_Animations)
		m_Animations.emplace(Pair.first, Pair.second->Clone());

	//  Prototype ?앹꽦 Buffer? Instance ?앹꽦 Buffer媛 ?ㅻⅤ湲??뚮Ц??nullptr 泥댄겕瑜??댁쨳?덈떎.
	for (auto& pBuffer : m_Buffers)
	{
		if (nullptr != pBuffer)
			Safe_AddRef(pBuffer);
	}

	//  SRV??Prototype, Instance 紐⑤몢 ?숈씪?섍쾶 ?ъ슜.
	for (auto& pSRV : m_SRVs)
	{
		if (nullptr != pSRV)
			Safe_AddRef(pSRV);
	}

	// ?ш린留?吏??
	m_UAVs.resize(Prototype.m_UAVs.size());

#ifdef _DEBUG
	m_AnimationNames = Prototype.m_AnimationNames;
#endif


}

void CModel::Sync_RootNode(CTransform* pOwnerTransform, CNavigation* pOwnerNavigation, _float fTimeDelta)
{
	_vector vPrePosition = pOwnerTransform->Get_State(STATE::POSITION);

	_matrix ResultMatrix = m_RootMatrix * pOwnerTransform->Get_WorldMatrix();
	
	_vector vScale, vRotation, vPosition;
	XMMatrixDecompose(&vScale, &vRotation, &vPosition, ResultMatrix);

	// nullptr == pOwnerNavigation || 

	_uint iLineIndex = {};
	if(true == pOwnerNavigation->IsMove(vPosition, pOwnerTransform->Get_State(STATE::LOOK), &iLineIndex))
		pOwnerTransform->Set_WorldMatrix(ResultMatrix);
	else
	{
		pOwnerTransform->Slide(vPosition - vPrePosition, -1.f * XMLoadFloat3(pOwnerNavigation->Get_Normal(iLineIndex)), pOwnerNavigation);
		//pOwnerTransform->Set_WorldMatrix(XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, pOwnerTransform->Get_State(STATE::POSITION)));
	}
}

const _float4x4* CModel::Get_BoneMatrixPtr(const _char* pBoneName)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool {
			if (0 == strcmp(pBoneName, pBone->Get_Name()))
				return true;
			return false;
		});

	if (iter == m_Bones.end())
		return nullptr;

	return (*iter)->Get_CombinedTransformationMatrix();
}

const vector<_float3>& CModel::Get_VerticesPos(_uint iIndex)
{
	if (iIndex >= m_iNumMeshes)
		CRASH("Mesh Index Error");
	return m_Meshes[iIndex]->Get_VerticesPos();
}

const vector<_uint>& CModel::Get_Indices(_uint iIndex)
{
	if (iIndex >= m_iNumMeshes)
		CRASH("Mesh Index Error");
	return m_Meshes[iIndex]->Get_Indices();
}

#ifdef _DEBUG
_float* CModel::Get_TrackPositionPtr(const _string& strAnimName)
{
	return m_Animations[strAnimName]->Get_TrackPositionPtr();
}

_float CModel::Get_Duration(const _string& strAnimName)
{
	return m_Animations[strAnimName]->Get_Duration();
}
void CModel::Set_TrackPosition(const _string& strAnimName, const _float fTrackPosition)
{
	m_Animations[strAnimName]->Set_CurrentTrackPosition(fTrackPosition);
}
HRESULT CModel::Bind_Bone_to_GUI(_int& iBoneIndex, _fmatrix TransformMatrix)
{
	_int iNextBoneIndex = iBoneIndex + 1;
	ImGuiTreeNodeFlags iFlag = 0;
	if((iNextBoneIndex >= m_Bones.size()) || (m_Bones[iNextBoneIndex]->Get_ParentIndex() != iBoneIndex))
		iFlag |= ImGuiTreeNodeFlags_Leaf;
	else
		iFlag |= (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);

	if(ImGui::TreeNodeEx(m_Bones[iBoneIndex]->Get_Name(), iFlag))
	{
		//Selecting Interaction 
		if(ImGui::IsItemClicked())
		{
			std::cout << "selected : " << m_Bones[iBoneIndex]->Get_Name() << std::endl;
		}

		while(iNextBoneIndex < m_Bones.size() && m_Bones[iNextBoneIndex]->Get_ParentIndex() == iBoneIndex)
		{
			Bind_Bone_to_GUI(iNextBoneIndex, TransformMatrix);
		}

		ImGui::TreePop();
	}
	iBoneIndex = iNextBoneIndex;
	return S_OK;
}
#endif // _DEBUG

void CModel::Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions)
{
	ifstream InputFile(strFilePath);

	json InputData;
	InputFile >> InputData;

	for (auto& Data : InputData)
	{
		_string strAnimName = Data["AnimName"];
		_float fTrackPosition = Data["TrackPosition"];
		_uint iEventID = Data["EventID"];

		auto iter = m_Animations.find(strAnimName);
		if (iter == m_Animations.end())
			return;
		m_Animations[strAnimName]->Register_Notify({ fTrackPosition, Functions[iEventID] });
	}

	for (auto& Pair : m_Animations)
		Pair.second->Sort_Notify();
}

void CModel::Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void()> EffectCallback)
{
	for (auto& pair : m_Animations)
	{
		const _string& animName = pair.first;
		CAnimation* pAnimation = pair.second;

		_string filePath = strNotifyFolderPath + "/" + animName + ".json";


		ifstream inputFile(filePath);
		// 1. ?대━硫?
		if (inputFile.is_open())
		{
			json notifyData;
			inputFile >> notifyData;
			inputFile.close();

			if (notifyData.contains("Notifies") && notifyData["Notifies"].is_array())
				pAnimation->Load_Notify(notifyData["Notifies"], ColliderCallback, EffectCallback);
			
		}

	}

	for (auto& pair : m_Animations)
		pair.second->Sort_AnimNotify();
		
}

HRESULT CModel::Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
	m_eType = eType;
	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	ifstream InputFile(pFilePath, ios::binary);
	if (false == InputFile.is_open())
	{
		MSG_BOX("Failed Open : Model");
		return E_FAIL;
	}

	if (MODELTYPE::ANIM == m_eType)
	{
		if (FAILED(Ready_Bone(InputFile, -1)))
			return E_FAIL;

		if (FAILED(Ready_Animation(pFilePath)))
			return E_FAIL;
	}
	if (FAILED(Ready_Mesh(InputFile)))
		return E_FAIL;
	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;
	InputFile.close();

	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_RootMatrix = XMMatrixIdentity();


	if (MODELTYPE::ANIM == m_eType)
	{
		if (FAILED(Ready_Shared_Buffers()))
			return E_FAIL;

	}
	

	return S_OK;
}

HRESULT CModel::Initialize_Clone(void* pArg)
{
	if (MODELTYPE::ANIM == m_eType)
	{
		if (FAILED(Ready_Instance_Buffers()))
			return E_FAIL;
	}
	

    return S_OK;
}

HRESULT CModel::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType);

}

HRESULT CModel::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Bind_BoneMatrices(pShader, pConstantName, m_Bones);
}

_bool CModel::Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend, _bool isRootMotion, _float fRootMotionRate)
{
	//
	//if (m_strPreAnimation != strAnimationName)
	//{
	//	m_isChangeAnimation = true;
	//	m_strPreAnimation = strAnimationName;
	//}

	// Animation
	if (true == isBlend && true == m_isBlend)
	{
		*pTrackPosition = 0.f;
		if (true == m_Animations.find(strAnimationName)->second->Blend_TransformationMatrices(fTimeDelta, m_Bones, 1.f))
		{
			Clear_Animation(strAnimationName);
			m_isBlend = false;
		}
	}
	else
	{
		auto iter = m_Animations.find(strAnimationName);
		if (iter == m_Animations.end())
			return S_OK;

		_float fTrackPosition = {};

		if (true == iter->second->Update_TransformationMatrices_All(fTimeDelta, m_Bones, &fTrackPosition))
		{
			if (m_strPreAnimation != strAnimationName)
			{
				if(m_strPreAnimation != "")
					m_isBlend = true;
				m_strPreAnimation = strAnimationName;
			}
			Clear_Animation(strAnimationName);
			return true;
		}
		if(nullptr != pTrackPosition)
			*pTrackPosition = fTrackPosition;

		// Root Node Translation
		if (true == isRootMotion)
			Compute_RootAnimation(fRootMotionRate);
	}



	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);


#ifdef _DEBUG
	_float4 fValue = {};
	OutputDebugString(TEXT("Play_Animation CPU \n"));
	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[0], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Right : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[1], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Up : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[2], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Look : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[3], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Pos : "), fValue);
#endif

	return false;
}

_bool CModel::Play_Animation_GPU(CComputeShader* pComputeShaderCom, const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isRootMotion, _float fRootMotionRate)
{
	ASSERT_CRASH(pComputeShaderCom);
	ASSERT_CRASH(pTrackPosition);

	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return S_OK;

#pragma region 1. Compute Shader
	// 1. 
	//    (
	_float fTrackPosition = 0.f;

	// 2. ?꾩옱 ?몃옓 ?ъ??섏쓣 媛?몄샃?덈떎. (?몃옓 ?ъ??섏? ?좊땲硫붿씠???대옒?ㅼ뿉??媛깆떊??諛쏆뒿?덈떎.)
	_bool bIsAnimationEnd = iter->second->Update_TrackPosition(fTimeDelta, &fTrackPosition);
	*pTrackPosition = fTrackPosition;

	// 3. ?곸닔 踰꾪띁(CB) ?낅뜲?댄듃
	//    - ?곗씠?붿뿉???꾩옱 ?좊땲硫붿씠???뺣낫瑜?李얘린 ?꾪븳 ?몃뜳?ㅼ? ?꾩옱 ?ъ깮 ?쒓컙???꾨떖
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pContext->Map(m_Buffers[BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	// ?좊땲硫붿씠???뺣낫 CB 援ъ“泥?=> ?꾩옱 AnimIndex? TrackPosition???뚯쑀.
	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_AnimationNameToIndex[strAnimationName]; /* ?좊땲硫붿씠???대쫫(strAnimationName)???대떦?섎뒗 ?몃뜳??*/;

	m_pContext->Unmap(m_Buffers[BUFFER_ANIM_INFOCB], 0);


	// 4. Compute Shader??由ъ냼??諛붿씤??
//    - ComputeShader.h/cpp??Set ?⑥닔?ㅼ쓣 ?ъ슜
	pComputeShaderCom->Set_SRV("g_BoneHierarchy", m_SRVs[SRV_BONE_HIERARCHY]); // ?꾩쭅 .hlsl???놁쓬
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_SRVs[SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_SRVs[SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_SRVs[SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_SRV("g_InverseBindPose", m_SRVs[SRV_INVERSEBIND_POSE]); // ?꾩쭅 .hlsl???놁쓬
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_UAVs[UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_Buffers[BUFFER_ANIM_INFOCB]);
	

#pragma region 이 부분 빡셈.... Dispatch

	// EX) 堉?504媛? ? ?ш린 64紐?
	
	// 5. Compute Shader ?ㅽ뻾 (Dispatch)
	// - 珥?堉?媛쒖닔留뚰겮 ?ㅻ젅?쒕? ?앹꽦?섎룄濡??ㅻ젅??洹몃９ ?섎? 議곗젅
	// - ?? ?곗씠???ㅻ젅??洹몃９ ?ш린媛 64???? (珥?堉?媛쒖닔 + 63) / 64
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);
#pragma endregion

	
	
	// 6. 以묎컙 寃곌낵 ?곸슜.(?쇰떒 RootAnimation CombinedTransofrmationMatrix??洹몃?濡??곸슜)
	ApplyComputeResults_ToBones();

	// 7. Rib ?좊땲硫붿씠???ъ깮 ??堉덉뿉 ?뺣낫 ?꾨떖.
	//_string strRibAnimationName = "Rib_" + strAnimationName;
	//Play_RibAnimation_GPU(strRibAnimationName, fTimeDelta);

	
#pragma endregion

	
	

	// 8. ?좊땲硫붿씠?섏씠 ?앸궗?ㅻ㈃? Clear ?묒뾽??吏꾪뻾?섍퀬 Animation???대━?댄빐以띾땲??
	if (bIsAnimationEnd)
	{
		Clear_Animation(strAnimationName);
		return true; // ?좊땲硫붿씠??醫낅즺
	}

	// Root Node Translation 議곗젙
	if (true == isRootMotion)
		Compute_RootAnimation(fRootMotionRate);


	// Combined???쒕쾲留?
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);


#ifdef _DEBUG
	
	/*OutputDebugString(TEXT("Play_Animation GPU "));
	
	_wstring strAnimDebug = StringToWString(strAnimationName) + L"\n";
	OutputDebugString(strAnimDebug.c_str());

	_float4 fValue = {};
	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[0], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Right : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[1], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Up : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[2], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Look : "), fValue);

	memcpy(&fValue, m_Bones[3]->Get_TransformationMatrix()->m[3], sizeof(_float4));
	OutPutDebugFloat4(TEXT("Bip001 Pos : "), fValue);


	OutPutDebugFloat(TEXT("Track Position : "), fTrackPosition);*/
#endif

	return false;
}



void CModel::Play_RibAnimation(const _string& strRibAnimationName, _float fTimeDelta)
{
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
		return;

	iter->second->Update_TransformationMatrices(fTimeDelta, m_Bones);

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

}

void CModel::Play_RibAnimation_GPU(const _string& strRibAnimationName, _float fTimeDelta)
{
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
		return;

	iter->second->Update_RibTransformationMatrices(fTimeDelta, m_Bones);


#ifdef _DEBUG
	// Bone Name占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쌕몌옙?
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		
		if (0 == strcmp(m_Bones[i]->Get_Name(), "Bip001RHand"))
		{
			_float4x4 mat = *m_Bones[i]->Get_TransformationMatrix();
			OutPutDebugMatrix(TEXT("Bip001RHand Play Rib Animation Matrix"), mat);

			_uint iParentIndex = m_Bones[i]->Get_ParentIndex();
			while (0 != strcmp(m_Bones[m_Bones[iParentIndex]->Get_ParentIndex()]->Get_Name(), "Bip001Spine1"))
			{
				_float4x4 mat = *m_Bones[iParentIndex]->Get_TransformationMatrix();
				_wstring strBoneName = StringToWString(m_Bones[iParentIndex]->Get_Name()) + TEXT(" Play Rib Animation Matrix");
				OutPutDebugMatrix(strBoneName, mat);
				iParentIndex = m_Bones[iParentIndex]->Get_ParentIndex();
			}
		}
	}
#endif // _DEBUG

	//for (auto& pBone : m_Bones)
	//	pBone->Update_RibCombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
}

void CModel::Clear_Animation(const _string& strAnimationName, _float fTrackPosition)
{
	if (strAnimationName == "")
		return;
	m_isChangeAnimation = true;
	m_Animations[strAnimationName]->Set_CurrentTrackPosition(fTrackPosition);
	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	//m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
}

BoundingBox* CModel::Get_BoundingBox(_uint iNumMesh)
{
	if (m_eType != MODELTYPE::MAP)
		ASSERT_CRASH("Is Not Map Object");

	if (iNumMesh >= m_iNumMeshes)
		return nullptr;

	return m_Meshes[iNumMesh]->Get_BoundingBox();
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;
	m_Meshes[iMeshIndex]->Render();
	
	return S_OK;
}

#ifdef _DEBUG
_bool CModel::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
	_float fMin = FLT_MAX;
	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		_float fDistance = {};
		if (true == m_Meshes[i]->Is_Picked(vRayPos, vRayDir, &fDistance) && fMin > fDistance)
			fMin = fDistance;
	}

	if (fMin < FLT_MAX)
	{
		*pDistance = fMin;
		return true;
	}

	return false;
}
#endif
 

void CModel::ApplyComputeResults_ToBones()
{
	// 1. GPU??異쒕젰 踰꾪띁(m_pFinalBoneMatrix_Buffer) ?댁슜??Staging 踰꾪띁濡?蹂듭궗?⑸땲??
	m_pContext->CopyResource(m_Buffers[BUFFER_STAGING], m_Buffers[BUFFER_FINAL_BONEMATRIX]);

	// 2. Staging 踰꾪띁瑜?CPU媛 ?쎌쓣 ???덈룄濡?Map ?⑸땲??
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	HRESULT hr = m_pContext->Map(m_Buffers[BUFFER_STAGING], 0, D3D11_MAP_READ, 0, &MappedSubResource);
	if (FAILED(hr))
		return;

	// 3. 留듯븨??硫붾え由ъ뿉??濡쒖뺄 ?됰젹 ?곗씠?곕? CPU 蹂?섎줈 蹂듭궗?⑸땲??
	vector<_float4x4> vLocalMatrices(m_Bones.size()); // UP??w媛 -7.4媛 ?섏샂.
	memcpy(vLocalMatrices.data(), MappedSubResource.pData, sizeof(_float4x4) * m_Bones.size());

	// 4. m_Bones 諛곗뿴??GPU媛 怨꾩궛??理쒖떊 濡쒖뺄 ?됰젹???곸슜?⑸땲??
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		/* Prev Final 怨깊븯湲?*/
		//_matrix FinalMatrix = XMLoadFloat4x4(m_Bones[i]->Get_TransformationMatrix()) * XMLoadFloat4x4(&vLocalMatrices[i]);
		_matrix FinalMatrix = XMLoadFloat4x4(&vLocalMatrices[i]);
		m_Bones[i]->Set_TransformationMatrix(FinalMatrix);

#ifdef _DEBUG
		// Bone Name占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쌕몌옙?
		if (0 == strcmp(m_Bones[i]->Get_Name(), "Bip001RHand"))
		{
			_float4x4 mat = *m_Bones[i]->Get_TransformationMatrix();
			OutPutDebugMatrix(TEXT("Bip001RHand Play Animation Matrix : "), mat);

			_uint iParentIndex = m_Bones[i]->Get_ParentIndex();
			while (0 != strcmp(m_Bones[m_Bones[iParentIndex]->Get_ParentIndex()]->Get_Name(), "Bip001Spine1"))
			{
				_float4x4 mat = *m_Bones[iParentIndex]->Get_TransformationMatrix();
				_wstring strBoneName = StringToWString(m_Bones[iParentIndex]->Get_Name()) + TEXT(" Play Animation Matrix");
				OutPutDebugMatrix(strBoneName, mat);
				iParentIndex = m_Bones[iParentIndex]->Get_ParentIndex();
			}
		}
#endif // _DEBUG

		
	}

	// 5. Unmap?쇰줈 留덈Т由ы빀?덈떎.
	m_pContext->Unmap(m_Buffers[BUFFER_STAGING], 0);
}

void CModel::Compute_RootAnimation(_float fRootMotionRate)
{
	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(m_Bones[m_iRootBoneIndex]->Get_TransformationMatrix()));

	_matrix RootBoneMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_Bones[m_iRootBoneIndex]->Set_TransformationMatrix(RootBoneMatrix);

	// Axis 議곗젙 (-y => +z)
	_float fTemp = vTranslation.m128_f32[2];
	vTranslation.m128_f32[0] = vTranslation.m128_f32[0] * -1.f;
	vTranslation.m128_f32[2] = vTranslation.m128_f32[1] * -1.f;
	vTranslation.m128_f32[1] = fTemp * -1.f;

	// Animation 蹂寃??? PreRootPosition??蹂寃쎈맂 Animation 泥섏쓬 KeyFrame Root Position?쇰줈 蹂寃?
	if (true == m_isChangeAnimation)
	{
		m_isChangeAnimation = false;
		XMStoreFloat4(&m_vPreRootPosition, vTranslation);
	}

	//_float fDistance = XMVector4Length(vTranslation - XMLoadFloat4(&m_vPreRootPosition)).m128_f32[0];
	//// ???꾩튂? ?ш쾶 踰쀬뼱?섎㈃ ?덉쇅泥섎━
	//if (fDistance > 150.f)
	//	XMStoreFloat4(&m_vPreRootPosition, vTranslation);

	//m_RootMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, (vTranslation - XMLoadFloat4(&m_vPreRootPosition)) * 0.1f);
	m_RootMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), (vTranslation - XMLoadFloat4(&m_vPreRootPosition)) * fRootMotionRate);
	
	XMStoreFloat4(&m_vPreRootRotation, vRotation);
	XMStoreFloat4(&m_vPreRootPosition, vTranslation);
}

HRESULT CModel::Ready_Bone(ifstream& InputFile, _int iParentIndex)
{
	_uint iNumChild = {};
	InputFile.read(reinterpret_cast<_char*>(&iNumChild), sizeof(_uint));
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szName[MAX_PATH] = {};
	InputFile.read(szName, iLength);
	_float4x4 TransformationMatrix = {};
	InputFile.read(reinterpret_cast<_char*>(&TransformationMatrix), sizeof(_float4x4));
	XMStoreFloat4x4(&TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&TransformationMatrix)));

	CBone* pBone = CBone::Create(szName, XMLoadFloat4x4(&TransformationMatrix), iParentIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_int iIndex = m_Bones.size() - 1;
	// Root Bone Index ???
	if (0 == strcmp(szName, "Root"))
		m_iRootBoneIndex = iIndex;

	for (size_t i = 0; i < iNumChild; ++i)
	{
		if (FAILED(Ready_Bone(InputFile, iIndex)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CModel::Ready_Mesh(ifstream& InputFile)
{
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));
	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), InputFile);
		ASSERT_CRASH(pMesh);
		m_Meshes.push_back(pMesh);
	}
	
	return S_OK;
}

HRESULT CModel::Ready_Material(const _char* pFilePath)
{
	_char szMaterialFilePath[MAX_PATH] = {};
	_char szMaterialDrivePath[MAX_PATH] = {};
	_char szMaterialDirPath[MAX_PATH] = {};
	_char szMaterialFileName[MAX_PATH] = {};
	_splitpath_s(pFilePath, szMaterialDrivePath, MAX_PATH, szMaterialDirPath, MAX_PATH, szMaterialFileName, MAX_PATH, nullptr, 0);
	
	strcpy_s(szMaterialFilePath, szMaterialDrivePath);
	strcat_s(szMaterialFilePath, szMaterialDirPath);
	strcat_s(szMaterialFilePath, "Mat/");
	strcat_s(szMaterialFilePath, szMaterialFileName);
	strcat_s(szMaterialFilePath, ".json");

	ifstream MaterialFile(szMaterialFilePath);
	if (false == MaterialFile.is_open())
	{
		MSG_BOX("Failed Open : Material");
		return E_FAIL;
	}

	json MaterialsData;
	MaterialFile >> MaterialsData;

	m_iNumMaterials = MaterialsData["NumMaterial"];

	for (auto& MaterialData : MaterialsData["Materials"])
	{
		CMeshMaterial* pMeshMaterial = CMeshMaterial::Create(m_pDevice, m_pContext, szMaterialFilePath, MaterialData);
		if (nullptr == pMeshMaterial)
			return E_FAIL;
		m_Materials.push_back(pMeshMaterial);
	}

	MaterialFile.close();

	return S_OK;
}

HRESULT CModel::Ready_Animation(const _char* pFilePath)
{
	_char szDrivePath[MAX_PATH] = {};
	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};

	_splitpath_s(pFilePath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szFilePath[MAX_PATH] = {};
	strcpy_s(szFilePath, szDrivePath);
	strcat_s(szFilePath, szDirPath);
	strcat_s(szFilePath, "Animation/");
	strcat_s(szFilePath, szFileName);
	strcat_s(szFilePath, "_Anim.dat");

	ifstream AnimationFile(szFilePath, ios::binary);

	AnimationFile.read(reinterpret_cast<_char*>(&m_iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < m_iNumAnimations; ++i)
	{
		CAnimation* pAnimation = CAnimation::Create(AnimationFile, m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;
		m_Animations.emplace(pAnimation->Get_Name(), pAnimation);
#ifdef _DEBUG
		m_AnimationNames.push_back(pAnimation->Get_Name());
#endif
	}

	AnimationFile.close();


	// Compute Shader 怨꾩궛???꾪븳 Animation Index ???
	
	_uint iAnimIdx = 0;
	m_AnimationNameToIndex.clear();
	for (auto& pair : m_Animations)
		m_AnimationNameToIndex.emplace(pair.first, iAnimIdx++);
		

	return S_OK;
}


HRESULT CModel::Ready_Shared_Buffers()
{
	ASSERT_CRASH(m_pDevice);

	// ?좊땲硫붿씠??紐⑤뜽???꾨땲硫??앹꽦?섏? ?딆쓬.
	if (MODELTYPE::ANIM != m_eType)
		return S_OK;

	HRESULT hr = S_OK;

	m_Buffers.resize(BUFFER_END);
	m_SRVs.resize(SRV_END);
	m_UAVs.resize(UAV_END);

	//  --- 1. ?곗씠???섏쭛???꾪븳 vector 以鍮?---

	vector<ANIMINFO>        vAllAnimInfos;		  // Depth1
	vector<GPU_CHANNELINFO> vAllChannelBoneInfos; // Depth2
	vector<GPU_KEYFRAME>    vAllKeyframes;        // Depth3

	// Depth1??????ㅼ젙.
	for (const auto& Pair : m_Animations)
	{
		CAnimation* pAnimation = Pair.second;
		ANIMINFO animInfo = {};

		// ?쒖옉 ?몃뜳?? 媛쒖닔, 吏?띿떆媛?
		animInfo.iStartChannelIndexOffset = static_cast<_uint>(vAllChannelBoneInfos.size()); // ?쒖감 ?먯깋 AnimInfo?먯꽌 0遺???ъ깮.
		animInfo.iNumChannels = static_cast<_uint>(pAnimation->Get_Channels().size());  // 紐⑤뱺 梨꾨꼸??媛쒖닔
#ifdef _DEBUG
		animInfo.fDuration = pAnimation->Get_Duration();
#endif
		// Depth2??????ㅼ젙.
		for (const auto& pChannel : pAnimation->Get_Channels())
		{
			// ?쒖옉 ?ㅽ봽?덉엫(?꾩쟻 ?몃뜳??, ?ㅽ봽?덉엫 媛쒖닔, 梨꾨꼸??愿由ы븯??堉??몃뜳??
			GPU_CHANNELINFO channelInfo = {};
			channelInfo.iStartKeyframeOffset = static_cast<_uint>(vAllKeyframes.size());
			channelInfo.iNumKeyframes = pChannel->Get_NumKeyframes();
			channelInfo.iBoneIndex = pChannel->Get_BoneIndex();

			// Depth3??????ㅼ젙.
			for (const auto& keyframe : pChannel->Get_Keyframes())
			{
				// ?ㅽ봽?덉엫??????뺣낫 蹂듭궗. Scale, Rotation, Translation, ?몃옓 ?꾩튂.
				GPU_KEYFRAME gpuKeyframe = {};
				gpuKeyframe.vScale = _float4(keyframe.vScale.x, keyframe.vScale.y, keyframe.vScale.z, 1.f);
				gpuKeyframe.vRotation = keyframe.vRotation;
				gpuKeyframe.vTranslation = _float4(keyframe.vTranslation.x, keyframe.vTranslation.y, keyframe.vTranslation.z, 1.f);
				gpuKeyframe.fTrackPosition = keyframe.fTrackPosition;
				vAllKeyframes.push_back(gpuKeyframe);
			}
			vAllChannelBoneInfos.push_back(channelInfo);
		}
		vAllAnimInfos.push_back(animInfo);
	}


	// --- 2. ?섏쭛???곗씠?곕줈 ?ㅼ젣 GPU 踰꾪띁 ?앹꽦 ---
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(GPU_KEYFRAME) * static_cast<_uint>(vAllKeyframes.size());
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(GPU_KEYFRAME);
	D3D11_SUBRESOURCE_DATA subresourceData = { vAllKeyframes.data() };
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_KEY_FRAME]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_KEY_FRAME], nullptr, &m_SRVs[SRV_KEY_FRAME]);
	if (FAILED(hr)) return E_FAIL;

	// 2-2. ?좊땲硫붿씠???뺣낫 踰꾪띁 (g_AllAnimInfos)
	bufferDesc.ByteWidth = sizeof(ANIMINFO) * static_cast<_uint>(vAllAnimInfos.size());
	bufferDesc.StructureByteStride = sizeof(ANIMINFO);
	subresourceData.pSysMem = vAllAnimInfos.data();
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_ANIM_INFO], nullptr, &m_SRVs[SRV_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;

	// 2-3. 堉?梨꾨꼸)蹂??뺣낫 踰꾪띁 (g_ChannelInfos)
	bufferDesc.ByteWidth = sizeof(GPU_CHANNELINFO) * static_cast<_uint>(vAllChannelBoneInfos.size());
	bufferDesc.StructureByteStride = sizeof(GPU_CHANNELINFO);
	subresourceData.pSysMem = vAllChannelBoneInfos.data();
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_BONE_CHANNEL]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_BONE_CHANNEL], nullptr, &m_SRVs[SRV_BONE_CHANNEL]);
	if (FAILED(hr)) return E_FAIL;

	

	return S_OK;
}

HRESULT CModel::Ready_Instance_Buffers()
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC bufferDesc = {};
	// 2-4. 理쒖쥌 濡쒖뺄 ?됰젹 異쒕젰(Output) 踰꾪띁 (g_OutLocalMatrices)
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(_float4x4) * static_cast<_uint>(m_Bones.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(_float4x4);
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateUnorderedAccessView(m_Buffers[BUFFER_FINAL_BONEMATRIX], nullptr, &m_UAVs[UAV_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_FINAL_BONEMATRIX], nullptr, &m_SRVs[SRV_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;

	// 2-5. 留??꾨젅???낅뜲?댄듃???곸닔 踰꾪띁 (AnimationInfo)
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(ANIMATION_CBINFO);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_ANIM_INFOCB]);
	if (FAILED(hr)) return E_FAIL;

	// 2-6. GPU -> CPU 蹂듭궗瑜??꾪븳 Staging 踰꾪띁
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(_float4x4) * static_cast<_uint>(m_Bones.size());
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_STAGING]);
	if (FAILED(hr)) return E_FAIL;

	return S_OK;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, PreTransformMatrix, pFilePath)))
	{
		MSG_BOX("Failed to Create : Model");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pClone = new CModel(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);
	m_Bones.clear();

	for (auto& pPair : m_Animations)
		Safe_Release(pPair.second);
	m_Animations.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);
	m_Materials.clear();


	/* GPU Buffer ?댁슜 ?쒓굅 */
	for (auto& pBuffer : m_Buffers)
		Safe_Release(pBuffer);
	m_Buffers.clear();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();

	for (auto& pUAV : m_UAVs)
		Safe_Release(pUAV);
	m_UAVs.clear();

}
