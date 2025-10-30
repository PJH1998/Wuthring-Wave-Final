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
	m_SRVs { Prototype.m_SRVs },
	m_isRibAnimation { Prototype.m_isRibAnimation }
{
	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pBone : Prototype.m_Bones)
		m_Bones.push_back(pBone->Clone());

	for (auto& Pair : Prototype.m_Animations)
		m_Animations.emplace(Pair.first, Pair.second->Clone());

	//  Prototype 생성 Buffer와 Instance 생성 Buffer가 다르기 때문에 nullptr 체크를 해줍니다.
	for (auto& pBuffer : m_Buffers)
	{
		if (nullptr != pBuffer)
			Safe_AddRef(pBuffer);
	}

	//  SRV는 Prototype, Instance 모두 동일하게 사용.
	for (auto& pSRV : m_SRVs)
	{
		if (nullptr != pSRV)
			Safe_AddRef(pSRV);
	}

	// 크기만 지정.
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

void CModel::Sync_RootNode(CTransform* pOwnerTransform, _float fTimeDelta)
{
	//_vector vPrePosition = pOwnerTransform->Get_State(STATE::POSITION);

	_matrix matWorld = pOwnerTransform->Get_WorldMatrix();
	_matrix ResultMatrix = m_RootMatrix * pOwnerTransform->Get_WorldMatrix();

	

	/*_vector vScale, vRotation, vPosition;
	XMMatrixDecompose(&vScale, &vRotation, &vPosition, ResultMatrix);*/

	pOwnerTransform->Set_WorldMatrix(ResultMatrix);
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
	if ((iNextBoneIndex >= m_Bones.size()) || (m_Bones[iNextBoneIndex]->Get_ParentIndex() != iBoneIndex))
		iFlag |= ImGuiTreeNodeFlags_Leaf;
	else
		iFlag |= (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);

	if (ImGui::TreeNodeEx(m_Bones[iBoneIndex]->Get_Name(), iFlag))
	{
		//Selecting Interaction 
		if (ImGui::IsItemClicked())
		{
			std::cout << "selected : " << m_Bones[iBoneIndex]->Get_Name() << std::endl;
			m_iSelectIndex = iBoneIndex;
		}

		while (iNextBoneIndex < m_Bones.size() && m_Bones[iNextBoneIndex]->Get_ParentIndex() == iBoneIndex)
		{
			Bind_Bone_to_GUI(iNextBoneIndex, TransformMatrix);
		}

		ImGui::TreePop();
	}
	iBoneIndex = iNextBoneIndex;
	return S_OK;
}
void CModel::Render_Gizmo(_fmatrix TransformMatrix)
{
	_fmatrix BoneLocalMatrix = XMLoadFloat4x4(m_Bones[m_iSelectIndex]->Get_CombinedTransformationMatrix());
	m_pGameInstance->Render_Gizmo(BoneLocalMatrix * TransformMatrix);
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

void CModel::Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback)
{
	for (auto& pair : m_Animations)
	{
		const _string& animName = pair.first;
		CAnimation* pAnimation = pair.second;

		_string filePath = strNotifyFolderPath + "/" + animName + ".json";


		ifstream inputFile(filePath);
		// 1. 열리면?
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
		// 1. Instance 전용 버퍼 생성.
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
	// 다른 Animation 들어올 시, 이전 Animation 저장
	//if (m_strPreAnimation != strAnimationName)
	//{
	//	m_isChangeAnimation = true;
	//	m_strPreAnimation = strAnimationName;
	//}

	// Animation 종료 시, 다음 Animation 처음 KeyFrame과 Blend => 사실상 안쓰고 있음.
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

		// Root Node Translation 조정
		if (true == isRootMotion)
			Compute_RootAnimation(fRootMotionRate);
	}



	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);


	return false;
}


_bool CModel::Play_Animation_GPU(CComputeShader* pComputeShaderCom, const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isRootMotion, _bool isRootMotionRotate, _bool isRootMotionTranslate, _float fRootMotionRate)
{
	ASSERT_CRASH(pComputeShaderCom);
	ASSERT_CRASH(pTrackPosition);

	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return false;

	// RootMotion에서 바뀐 애니메이션에 대한 로직 처리.
	if (m_strPreAnimation != strAnimationName)
	{
		m_isChangeAnimation = true;
		m_strPreAnimation = strAnimationName;
		Clear_Animation(strAnimationName);
	}


	// 1. 현재 애니메이션의 Track Position 업데이트
	//    (애니메이션 종료 여부 판단은 기존 로직 활용 가능)
	_float fTrackPosition = 0.f;

	// 2. 현재 트랙 포지션을 가져옵니다. (트랙 포지션은 애니메이션 클래스에서 갱신을 받습니다.)
	_bool bIsAnimationEnd = iter->second->Update_TrackPosition(fTimeDelta, &fTrackPosition);
	*pTrackPosition = fTrackPosition;

	// 3. 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromCompute(pComputeShaderCom, fTrackPosition, strAnimationName);
	// Root Node Translation 조정
	if (true == isRootMotion)
		Compute_RootAnimation(fRootMotionRate, isRootMotionRotate, isRootMotionTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();
	

	// 4. 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (bIsAnimationEnd)
	{
		Clear_Animation(strAnimationName);
		return true; // 애니메이션 종료
	}

#
	// 5. Combined는 한번만.
	for (_uint i = 0; i < m_Bones.size(); i++)
	{
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
	}




	return false;
}

_bool CModel::Play_Animation(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend, _bool isRootMotion, _float fRootMotionRate)
{
	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return false;

	if (m_strPreAnimation != strAnimationName)
	{
		m_isChangeAnimation = true;
		m_strPreAnimation = strAnimationName;
	}


	return false;
}



//void CModel::Play_RibAnimation(const _string& strRibAnimationName, _float fTimeDelta)
//{
//	auto iter = m_Animations.find(strRibAnimationName);
//	if (iter == m_Animations.end())
//		return;
//
//	iter->second->Update_TransformationMatrices(fTimeDelta, m_Bones);
//
//	for (auto& pBone : m_Bones)
//		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
//	
//}

void CModel::Play_RibAnimation(const _string& strRibAnimationName, _float fTrackPosition)
{
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
		return;

	iter->second->Update_RibTransformationMatrices(fTrackPosition, m_Bones);
	//iter->second->Update_TransformationMatrices(fTrackPosition, m_Bones);

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
}

void CModel::Clear_Animation(const _string& strAnimationName, _float fTrackPosition)
{
	if (strAnimationName == "")
		return;

	m_isChangeAnimation = true;
	m_Animations[strAnimationName]->Set_CurrentTrackPosition(fTrackPosition);
	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_RootMatrix = XMMatrixIdentity();
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
	// 1. GPU의 출력 버퍼(m_pFinalBoneMatrix_Buffer) 내용을 Staging 버퍼로 복사합니다.
	m_pContext->CopyResource(m_Buffers[BUFFER_STAGING], m_Buffers[BUFFER_FINAL_BONEMATRIX]);

	// 2. Staging 버퍼를 CPU가 읽을 수 있도록 Map 합니다.
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	HRESULT hr = m_pContext->Map(m_Buffers[BUFFER_STAGING], 0, D3D11_MAP_READ, 0, &MappedSubResource);
	if (FAILED(hr))
		return;

	// 3. 맵핑된 메모리에서 로컬 행렬 데이터를 CPU 변수로 복사합니다.
	vector<_float4x4> vLocalMatrices(m_Bones.size());
	memcpy(vLocalMatrices.data(), MappedSubResource.pData, sizeof(_float4x4) * m_Bones.size());

	// 4. m_Bones 배열에 GPU가 계산한 최신 로컬 행렬을 적용합니다.
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		_matrix FinalMatrix = XMLoadFloat4x4(&vLocalMatrices[i]);
		m_Bones[i]->Set_TransformationMatrix(FinalMatrix);
	}

	// 5. Unmap으로 마무리합니다.
	m_pContext->Unmap(m_Buffers[BUFFER_STAGING], 0);
}

void CModel::FetchLocalMatrices_FromCompute(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName)
{
	ASSERT_CRASH(pComputeShaderCom);

	// 1. 상수 버퍼(CB) 업데이트
	// - 셰이더에서 현재 애니메이션 정보를 찾기 위한 인덱스와 현재 재생 시간을 전달
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pContext->Map(m_Buffers[BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	_bool IsRibAnimUsed = false;
	// 애니메이션 정보 CB 구조체 => 현재 AnimIndex와 TrackPosition을 소유.
	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
 	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_AnimationNameToIndex[strAnimationName]; /* 애니메이션 이름(strAnimationName)에 해당하는 인덱스 */;
	pAnimCBInfo->IsRibAnimUsed = false;
	pAnimCBInfo->iRibbonAnimIndex = 0;


	// 2. Ribbon 애니메이션이 존재한다면 정보 바인딩
	_string strRibAnimationName = "Rib_" + strAnimationName;
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
	{
		IsRibAnimUsed = false;
	}
	else
	{
		IsRibAnimUsed = true;
		pAnimCBInfo->iRibbonAnimIndex = m_AnimationNameToIndex[strRibAnimationName];
	}

	pAnimCBInfo->IsRibAnimUsed = IsRibAnimUsed;
	m_pContext->Unmap(m_Buffers[BUFFER_ANIM_INFOCB], 0);

	// 3. Compute Shader에 리소스 바인딩
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_SRVs[SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_SRVs[SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_SRVs[SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_SRV("g_InverseBindPoses", m_SRVs[SRV_INVERSEBIND_POSE]); // 아직 .hlsl에 없음
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_UAVs[UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_Buffers[BUFFER_ANIM_INFOCB]);

	// EX) 뼈 504개, 팀 크기 64명
	// 4. Compute Shader 실행 (Dispatch)
	// - 총 뼈 개수만큼 스레드를 생성하도록 스레드 그룹 수를 조절
	// - 예: 셰이더 스레드 그룹 크기가 64일 때, (총 뼈 개수 + 63) / 64
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);

	// 5. GPU의 출력 버퍼(m_pFinalBoneMatrix_Buffer) 내용을 Staging 버퍼로 복사합니다.
	m_pContext->CopyResource(m_Buffers[BUFFER_STAGING], m_Buffers[BUFFER_FINAL_BONEMATRIX]);

	// 6. Staging 버퍼를 CPU가 읽을 수 있도록 Map 합니다.
	D3D11_MAPPED_SUBRESOURCE ReadMappedSubResource;
	HRESULT hr = m_pContext->Map(m_Buffers[BUFFER_STAGING], 0, D3D11_MAP_READ, 0, &ReadMappedSubResource);
	if (FAILED(hr))
		return;

	// 7. 맵핑된 메모리에서 로컬 행렬 데이터를 CPU 변수로 복사합니다.
	vector<_float4x4> vLocalMatrices(m_Bones.size());
	memcpy(vLocalMatrices.data(), ReadMappedSubResource.pData, sizeof(_float4x4) * m_Bones.size());

	// 8. m_Bones 배열에 GPU가 계산한 최신 로컬 행렬을 적용합니다.
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		_matrix FinalMatrix = XMLoadFloat4x4(&vLocalMatrices[i]);
		m_Bones[i]->Set_TransformationMatrix(FinalMatrix);
	}

	// 9. Unmap으로 마무리합니다.  
	m_pContext->Unmap(m_Buffers[BUFFER_STAGING], 0);
}

void CModel::Compute_RootAnimation(_float fRootMotionRate, _bool isRootMotionRotation, _bool isRootMotionTranslate)
{
	// 1. GPU 계산 로컬본 전체 가져오기.
	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(m_Bones[m_iRootBoneIndex]->Get_TransformationMatrix()));

	// 2. 스켈레톤의 루트 본은 '제자리'에서 (Scale, Rotation)만 하도록
	_matrix RootBoneLocalMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_Bones[m_iRootBoneIndex]->Set_TransformationMatrix(RootBoneLocalMatrix);

	// 축 변환 쿼터니언 생성
	//_matrix matConversion = XMMatrixRotationX(XM_PIDIV2) * XMMatrixScaling(-1.f, 1.f, 1.f);
	_matrix matConversion = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(XM_PI) * XMMatrixScaling(-1.f, 1.f, 1.f);
	_vector qConversion = XMQuaternionRotationMatrix(matConversion);

	// 현재 프레임의 T, R을 '엔진 좌표계'로 변환
	_vector vConvertedTranslation = XMVector3Transform(vTranslation, matConversion);
	_vector vConvertedRotation = XMQuaternionMultiply(qConversion, vRotation);

	// 이동 변화량 계산
	_vector vLocalTranslate = vConvertedTranslation - XMLoadFloat4(&m_vPreRootPosition);
	// 회전 변화량 계산
	_vector vRotationDelta = XMQuaternionMultiply(vConvertedRotation, XMQuaternionInverse(XMLoadFloat4(&m_vPreRootRotation)));

	// RootMotion 회전 껐으면 0으로.
	if (!isRootMotionRotation)
		vRotationDelta = XMQuaternionIdentity();

	if (!isRootMotionTranslate)
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f);


	// 애니메이션 변경 시 순간이동 방지
	if (true == m_isChangeAnimation)
	{
		m_isChangeAnimation = false;
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f); 
		vRotationDelta = XMQuaternionIdentity();           
	}

	// 행렬은 이제 '엔진 좌표계' 기준.
	m_RootMatrix = XMMatrixAffineTransformation(
		//XMVectorSet(1.f, 1.f, 1.f, 1.f), // 스케일 델타 (없음)
		XMVectorSet(1.f, 1.f, 1.f, 1.f), // 스케일 델타 (없음)
		XMVectorSet(0.f, 0.f, 0.f, 1.f), // 원점
		vRotationDelta,                  // 회전 델타
		vLocalTranslate * fRootMotionRate // 이동 델타

	
	);
	
	// 다음 프레임을 위해 '변환된' T, R 값을 저장합니다.
	XMStoreFloat4(&m_vPreRootPosition, vConvertedTranslation);
	XMStoreFloat4(&m_vPreRootRotation, vConvertedRotation);
}

//void CModel::Compute_RootAnimation(_float fRootMotionRate)
//{
//	_vector vScale{}, vRotation{}, vTranslation{};
//	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(m_Bones[m_iRootBoneIndex]->Get_TransformationMatrix()));
//
//	_matrix RootBoneMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));
//	m_Bones[m_iRootBoneIndex]->Set_TransformationMatrix(RootBoneMatrix);
//
//	// Axis 조정 (-y => +z)
//	// 이거 빼면 제생각에. 안해도 되지않을까 설정을
//	// 우리가 생각하는 정면은 +z인데 블렌더에서는 -y라서 문제다.
//	_float fTemp = vTranslation.m128_f32[2];
//	vTranslation.m128_f32[0] = vTranslation.m128_f32[0] * -1.f;
//	vTranslation.m128_f32[2] = vTranslation.m128_f32[1];
//	vTranslation.m128_f32[1] = fTemp;
//
//	
//
//	// Animation 변경 시, PreRootPosition을 변경된 Animation 처음 KeyFrame Root Position으로 변경
//	if (true == m_isChangeAnimation)
//	{
//		m_isChangeAnimation = false;
//		XMStoreFloat4(&m_vPreRootPosition, vTranslation);
//	}
//
//	m_RootMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), (vTranslation - XMLoadFloat4(&m_vPreRootPosition)) * fRootMotionRate);
//	
//	XMStoreFloat4(&m_vPreRootRotation, vRotation);
//	XMStoreFloat4(&m_vPreRootPosition, vTranslation);
//}



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
	// Root Bone Index 저장
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


	// Compute Shader 계산을 위한 Animation Index 저장.
	
	_uint iAnimIdx = 0;
	m_AnimationNameToIndex.clear();
	for (auto& pair : m_Animations)
		m_AnimationNameToIndex.emplace(pair.first, iAnimIdx++);
		

	return S_OK;
}



// Prototype에서 Shared Buffers 생성.
HRESULT CModel::Ready_Shared_Buffers()
{
	ASSERT_CRASH(m_pDevice);

	// 애니메이션 모델이 아니면 생성하지 않음.
	if (MODELTYPE::ANIM != m_eType)
		return S_OK;

	HRESULT hr = S_OK;

	m_Buffers.resize(BUFFER_END);
	m_SRVs.resize(SRV_END);
	m_UAVs.resize(UAV_END);

	//  --- 1. 데이터 수집을 위한 vector 준비 ---
	vector<ANIMINFO>        vAllAnimInfos;		  // Depth1
	vector<GPU_CHANNELINFO> vAllChannelBoneInfos; // Depth2
	vector<GPU_KEYFRAME>    vAllKeyframes;        // Depth3

	// Depth1에 대한 설정.
	for (const auto& Pair : m_Animations)
	{
		CAnimation* pAnimation = Pair.second;
		ANIMINFO animInfo = {};

		// 시작 인덱스, 개수, 지속시간.
		animInfo.iStartChannelIndexOffset = static_cast<_uint>(vAllChannelBoneInfos.size()); // 순차 탐색 AnimInfo에서 0부터 재생.
		animInfo.iNumChannels = static_cast<_uint>(pAnimation->Get_Channels().size());  // 모든 채널의 개수
		animInfo.fDuration = pAnimation->Get_Duration();

		

		// Depth2에 대한 설정.
		for (const auto& pChannel : pAnimation->Get_Channels())
		{
			// 시작 키프레임(누적 인덱스), 키프레임 개수, 채널이 관리하는 뼈 인덱스
			GPU_CHANNELINFO channelInfo = {};
			channelInfo.iStartKeyframeOffset = static_cast<_uint>(vAllKeyframes.size());
			channelInfo.iNumKeyframes = pChannel->Get_NumKeyframes();
			channelInfo.iBoneIndex = pChannel->Get_BoneIndex();

			// Depth3에 대한 설정.
			for (const auto& keyframe : pChannel->Get_Keyframes())
			{
				// 키프레임에 대한 정보 복사. Scale, Rotation, Translation, 트랙 위치.
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


	// --- 2. 수집된 데이터로 실제 GPU 버퍼 생성 ---

	D3D11_BUFFER_DESC bufferDesc = {};
	// 2-1. 애니메이션 키프레임 정보 버퍼.
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

	// 2-2. 애니메이션 정보 버퍼 (g_AllAnimInfos)
	bufferDesc.ByteWidth = sizeof(ANIMINFO) * static_cast<_uint>(vAllAnimInfos.size());
	bufferDesc.StructureByteStride = sizeof(ANIMINFO);
	subresourceData.pSysMem = vAllAnimInfos.data();
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_ANIM_INFO], nullptr, &m_SRVs[SRV_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;

	// 2-3. 뼈(채널)별 정보 버퍼 (g_ChannelInfos)
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
	// 2-4. 최종 로컬 행렬 출력(Output) 버퍼 (g_OutLocalMatrices)
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

	// 2-5. 매 프레임 업데이트할 상수 버퍼 (AnimationInfo)
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(ANIMATION_CBINFO);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_ANIM_INFOCB]);
	if (FAILED(hr)) return E_FAIL;

	// 2-6. GPU -> CPU 복사를 위한 Staging 버퍼
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


	/* GPU Buffer 내용 제거 */
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
