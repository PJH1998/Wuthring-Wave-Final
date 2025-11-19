#include "EditorPch.h"
#include "ModelLoader.h"

CModelLoader::CModelLoader()
{
}

HRESULT CModelLoader::Initialize()
{
	return S_OK;
}

void CModelLoader::Update()
{
	ImGui::Begin("Model Load");

	if (ImGui::RadioButton("NonAnim", m_iAnim == 0)) m_iAnim = 0;
	if (ImGui::RadioButton("Anim", m_iAnim == 1)) m_iAnim = 1;
	if (ImGui::RadioButton("Character", m_iAnim == 2)) m_iAnim = 2;
	if (0 == m_iAnim) m_eType = MODELTYPE::NONANIM;
	else if(1 == m_iAnim) m_eType = MODELTYPE::ANIM;
	else if(2 == m_iAnim) m_eType = MODELTYPE::CHARACTER;

	if (ImGui::Button("Load FBX"))
		m_isShowLoadFile = !m_isShowLoadFile;
	ImGui::SameLine();
	if (ImGui::Button("Save Model"))
		m_isShowSaveFile = !m_isShowSaveFile;

	ImGui::Checkbox("LoadAll_For_Map", &m_isLoadAll);
	
	if(true == m_isShowLoadFile)
		Load_File();
	if (true == m_isShowSaveFile)
		Save_File();

	// Model Info
	Show_Info();

	ImGui::End();
}

HRESULT CModelLoader::Save_Dat_Anim(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	ofstream file(pFileName, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Model Save Fail");
		return E_FAIL;
	}

#pragma region BONE
	aiNode* pRoot = m_pAIScene->mRootNode;
	if (FAILED(Save_Bone(file, pRoot)))
		return E_FAIL;
#pragma endregion

#pragma region MESH
	file.write(reinterpret_cast<const _char*>(&m_pAIScene->mNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = m_pAIScene->mMeshes[i];

		file.write(reinterpret_cast<const _char*>(&pMesh->mNumVertices), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumFaces), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mMaterialIndex), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumBones), sizeof(_uint));

		
		VTXANIMMESH* Vertices = new VTXANIMMESH[pMesh->mNumVertices];
		ZeroMemory(Vertices, sizeof(VTXANIMMESH) * pMesh->mNumVertices);

		for (size_t j = 0; j < pMesh->mNumVertices; ++j)
		{
			memcpy(&Vertices[j].vPosition, &pMesh->mVertices[j], sizeof(_float3));
			memcpy(&Vertices[j].vNormal, &pMesh->mNormals[j], sizeof(_float3));
			memcpy(&Vertices[j].vTangent, &pMesh->mTangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vBinormal, &pMesh->mBitangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vTexcoord, &pMesh->mTextureCoords[0][j], sizeof(_float2));
		}

		for (size_t j = 0; j < pMesh->mNumBones; ++j)
		{
			aiBone* pBone = pMesh->mBones[j];
			aiString strBoneName = pBone->mName;
			_uint iLength = strBoneName.length;
			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strBoneName.data), iLength);
			file.write(reinterpret_cast<const _char*>(&pBone->mOffsetMatrix), sizeof(_float4x4));
			// Bone Weight
			for (size_t k = 0; k < pBone->mNumWeights; ++k)
			{
				aiVertexWeight VertexWeight = pBone->mWeights[k];
				_uint iVertexID = VertexWeight.mVertexId;
				if (0 == Vertices[iVertexID].vBlendWeight.x)
				{
					Vertices[iVertexID].vBlendIndex.x = j;
					Vertices[iVertexID].vBlendWeight.x = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.y)
				{
					Vertices[iVertexID].vBlendIndex.y = j;
					Vertices[iVertexID].vBlendWeight.y = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.z)
				{
					Vertices[iVertexID].vBlendIndex.z = j;
					Vertices[iVertexID].vBlendWeight.z = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.w)
				{
					Vertices[iVertexID].vBlendIndex.w = j;
					Vertices[iVertexID].vBlendWeight.w = VertexWeight.mWeight;
				}
			}
		}

		file.write(reinterpret_cast<const _char*>(Vertices), sizeof(VTXANIMMESH) * pMesh->mNumVertices);
		Safe_Delete_Array(Vertices);

		_uint* Indices = new _uint[pMesh->mNumFaces * 3];
		_uint iIndex = {};
		for (size_t j = 0; j < pMesh->mNumFaces; ++j)
		{
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[0];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[1];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[2];
		}
		file.write(reinterpret_cast<const _char*>(Indices), sizeof(_uint) * pMesh->mNumFaces * 3);
		Safe_Delete_Array(Indices);
	}

	file.close();
#pragma endregion

	return S_OK;
}

HRESULT CModelLoader::Save_Dat_Character(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	ofstream file(pFileName, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Model Save Fail");
		return E_FAIL;
	}

#pragma region BONE 계층 구조 저장
	aiNode* pRoot = m_pAIScene->mRootNode;
	if (FAILED(Save_Bone(file, pRoot)))
		return E_FAIL;
#pragma endregion

#pragma region MESH 및 Shape key 데이터 저장.
	file.write(reinterpret_cast<const _char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // 전체 메쉬 개수.

	for (size_t i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = m_pAIScene->mMeshes[i];

		// 1. 기본 메쉬 정보 저장.
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumVertices), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumFaces), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mMaterialIndex), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumBones), sizeof(_uint));

		// 2. Shape Key (Morph Target) 정보 저장 시작 =. Shape Key (Morph Target) 개수 저장.
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumAnimMeshes), sizeof(_uint)); 

		for (size_t k = 0; k < pMesh->mNumAnimMeshes; ++k)
		{
			aiAnimMesh* pAnimMesh = pMesh->mAnimMeshes[k];

			// 쉐이프 키 이름 저장
			aiString strShapeName = pAnimMesh->mName;
			_uint iNameLen = strShapeName.length;
			file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strShapeName.data), iNameLen);

			// 변위(Delta) 데이터 개수 저장(보통 기본 메쉬 정점 수와 같음)
			file.write(reinterpret_cast<const _char*>(&pAnimMesh->mNumVertices), sizeof(_uint));

			// 쉐이프 키는 '최종 위치'가 아닌 '이동해야 할 거리(Delta)'를 담고 있습니다.
			file.write(reinterpret_cast<const _char*>(pAnimMesh->mVertices), sizeof(_float3) * pAnimMesh->mNumVertices);

			//	법선 변위(Delta Normal) 저장(데이터가 있는지 확인)
			if (pAnimMesh->mNormals)
				file.write(reinterpret_cast<const _char*>(pAnimMesh->mNormals), sizeof(_float3) * pAnimMesh->mNumVertices);
			else
			{
				// 여기서는 간단히 0으로 채운 벡터를 씀 (안전장치)
				_float3 vZero = { 0.f, 0.f, 0.f };
				for (_uint z = 0; z < pAnimMesh->mNumVertices; ++z)
					file.write(reinterpret_cast<const _char*>(&vZero), sizeof(_float3)); // 법선 데이터가 없다면 0으로 채운 더미 데이터를 넣거나, 로드 시 처리해야 함.
			}
		}
		// --- Shape Key 저장 끝 ---


		// 정점(Vertex) 데이터 생성 및 저장
		VTXANIMMESH* Vertices = new VTXANIMMESH[pMesh->mNumVertices];
		ZeroMemory(Vertices, sizeof(VTXANIMMESH) * pMesh->mNumVertices);

		for (size_t j = 0; j < pMesh->mNumVertices; ++j)
		{
			memcpy(&Vertices[j].vPosition, &pMesh->mVertices[j], sizeof(_float3));
			memcpy(&Vertices[j].vNormal, &pMesh->mNormals[j], sizeof(_float3));
			memcpy(&Vertices[j].vTangent, &pMesh->mTangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vBinormal, &pMesh->mBitangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vTexcoord, &pMesh->mTextureCoords[0][j], sizeof(_float2));
		}

		// Bone Weight 계산 ( 기존과 동일 )
		for (size_t j = 0; j < pMesh->mNumBones; ++j)
		{
			aiBone* pBone = pMesh->mBones[j];
			aiString strBoneName = pBone->mName;
			_uint iLength = strBoneName.length;
			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strBoneName.data), iLength);
			file.write(reinterpret_cast<const _char*>(&pBone->mOffsetMatrix), sizeof(_float4x4));

			// Bone Weight
			for (size_t k = 0; k < pBone->mNumWeights; ++k)
			{
				aiVertexWeight VertexWeight = pBone->mWeights[k];
				_uint iVertexID = VertexWeight.mVertexId;
				if (0 == Vertices[iVertexID].vBlendWeight.x)
				{
					Vertices[iVertexID].vBlendIndex.x = j;
					Vertices[iVertexID].vBlendWeight.x = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.y)
				{
					Vertices[iVertexID].vBlendIndex.y = j;
					Vertices[iVertexID].vBlendWeight.y = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.z)
				{
					Vertices[iVertexID].vBlendIndex.z = j;
					Vertices[iVertexID].vBlendWeight.z = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.w)
				{
					Vertices[iVertexID].vBlendIndex.w = j;
					Vertices[iVertexID].vBlendWeight.w = VertexWeight.mWeight;
				}
			}
		}

		// 정점 및 인덱스 버퍼 쓰기.
		file.write(reinterpret_cast<const _char*>(Vertices), sizeof(VTXANIMMESH) * pMesh->mNumVertices);
		Safe_Delete_Array(Vertices);

		_uint* Indices = new _uint[pMesh->mNumFaces * 3];
		_uint iIndex = {};
		for (size_t j = 0; j < pMesh->mNumFaces; ++j)
		{
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[0];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[1];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[2];
		}
		file.write(reinterpret_cast<const _char*>(Indices), sizeof(_uint) * pMesh->mNumFaces * 3);
		Safe_Delete_Array(Indices);
	}

#pragma endregion

	return S_OK;


}

HRESULT CModelLoader::Save_Animation(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szAnimFilePath[MAX_PATH] = {};
	strcpy_s(szAnimFilePath, szDirPath);
	strcat_s(szAnimFilePath, "Animation/");
	strcat_s(szAnimFilePath, szFileName);
	strcat_s(szAnimFilePath, "_Anim.dat");

	filesystem::path dir = filesystem::path(szAnimFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szAnimFilePath, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Animation Save Fail");
		return E_FAIL;
	}

	_uint iNumAnimations = m_pAIScene->mNumAnimations;
	// Num Animation
	file.write(reinterpret_cast<const _char*>(&iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < iNumAnimations; ++i)
	{
		aiAnimation* pAnimation = m_pAIScene->mAnimations[i];
		aiString strName = pAnimation->mName;
		_uint iLength = strName.length;
		// Animation Name
		file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
		file.write(strName.data, iLength);

		_float fDuration = pAnimation->mDuration;
		// Animation Duration (吏?띿떆媛?
		file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));

		_float fTickPerSecond = pAnimation->mTicksPerSecond;
		// Animation TickPerSecond (珥덈떦 ?대룞??
		file.write(reinterpret_cast<const _char*>(&fTickPerSecond), sizeof(_float));

		_uint iNumChannels = pAnimation->mNumChannels;
		// Num Channel
		file.write(reinterpret_cast<const _char*>(&iNumChannels), sizeof(_uint));

		for (size_t j = 0; j < iNumChannels; ++j)
		{
			aiNodeAnim* pChannel = pAnimation->mChannels[j];
			aiString strChannelName = pChannel->mNodeName;
			_uint iChannelNameLength = strChannelName.length;
			// Channel(Bone) Name
			file.write(reinterpret_cast<const _char*>(&iChannelNameLength), sizeof(_uint));
			file.write(strChannelName.data, iChannelNameLength);

			_uint iNumKeyFrame = max(pChannel->mNumPositionKeys, max(pChannel->mNumRotationKeys, pChannel->mNumScalingKeys));
			// Num KeyFrame
			file.write(reinterpret_cast<const _char*>(&iNumKeyFrame), sizeof(_uint));

			_float3 vScale = {};
			_float4 vRotation = {};
			_float3 vTranslation = {};

			for (size_t k = 0; k < iNumKeyFrame; ++k)
			{
				KEYFRAME KeyFrame = {};
				if (k < pChannel->mNumScalingKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mScalingKeys[k].mTime;
					memcpy(&vScale, &pChannel->mScalingKeys[k].mValue, sizeof(_float3));
				}
				if (k < pChannel->mNumRotationKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mRotationKeys[k].mTime;
					//_string temp = "MO1AnjinMd00601";
					//if (0 == temp.compare(pChannel->mNodeName.C_Str()))
					//{
					//	vRotation.x = 0.f;
					//	vRotation.y = 0.f;
					//	vRotation.z = 0.f;
					//	vRotation.w = 1.f;
					//}
					//else
					{
						vRotation.x = pChannel->mRotationKeys[k].mValue.x;
						vRotation.y = pChannel->mRotationKeys[k].mValue.y;
						vRotation.z = pChannel->mRotationKeys[k].mValue.z;
						vRotation.w = pChannel->mRotationKeys[k].mValue.w;
					}
				}
				if (k < pChannel->mNumPositionKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mPositionKeys[k].mTime;
					memcpy(&vTranslation, &pChannel->mPositionKeys[k].mValue, sizeof(_float3));
				}
				KeyFrame.vScale = vScale;
				KeyFrame.vRotation = vRotation;
				KeyFrame.vTranslation = vTranslation;
				file.write(reinterpret_cast<const _char*>(&KeyFrame), sizeof(KEYFRAME));
			}
		}
	}

	file.close();

	return S_OK;
}

HRESULT CModelLoader::Save_Animation_Character(const _char* pFileName)
{

	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szAnimFilePath[MAX_PATH] = {};
	strcpy_s(szAnimFilePath, szDirPath);
	strcat_s(szAnimFilePath, "Animation/");
	strcat_s(szAnimFilePath, szFileName);
	strcat_s(szAnimFilePath, "_Anim.dat");

	filesystem::path dir = filesystem::path(szAnimFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szAnimFilePath, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Animation Save Fail");
		return E_FAIL;
	}

	_uint iNumAnimations = m_pAIScene->mNumAnimations;
	// Num Animation
	file.write(reinterpret_cast<const _char*>(&iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < iNumAnimations; ++i)
	{
		aiAnimation* pAnimation = m_pAIScene->mAnimations[i];
		aiString strName = pAnimation->mName;

		// 2. Main Animation 정보 저장
		_uint iLength = strName.length;
		// Animation Name
		file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
		file.write(strName.data, iLength);

		_float fDuration = pAnimation->mDuration;
		// Animation Duration
		file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));

		_float fTickPerSecond = pAnimation->mTicksPerSecond;
		// Animation TickPerSecond
		file.write(reinterpret_cast<const _char*>(&fTickPerSecond), sizeof(_float));

		_uint iNumChannels = pAnimation->mNumChannels;
		// Num Channel
		file.write(reinterpret_cast<const _char*>(&iNumChannels), sizeof(_uint));

		// 3. Channel 정보 저장.
		for (size_t j = 0; j < iNumChannels; ++j)
		{
			aiNodeAnim* pChannel = pAnimation->mChannels[j];
			aiString strChannelName = pChannel->mNodeName;
			_uint iChannelNameLength = strChannelName.length;
			
			// Channel(Bone) Name
			file.write(reinterpret_cast<const _char*>(&iChannelNameLength), sizeof(_uint));
			file.write(strChannelName.data, iChannelNameLength);

			_uint iNumKeyFrame = max(pChannel->mNumPositionKeys, max(pChannel->mNumRotationKeys, pChannel->mNumScalingKeys));
			// Num KeyFrame
			file.write(reinterpret_cast<const _char*>(&iNumKeyFrame), sizeof(_uint));

			_float3 vScale = {};
			_float4 vRotation = {};
			_float3 vTranslation = {};

			for (size_t k = 0; k < iNumKeyFrame; ++k)
			{
				KEYFRAME KeyFrame = {};
				if (k < pChannel->mNumScalingKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mScalingKeys[k].mTime;
					memcpy(&vScale, &pChannel->mScalingKeys[k].mValue, sizeof(_float3));
				}
				if (k < pChannel->mNumRotationKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mRotationKeys[k].mTime;
					
					{
						vRotation.x = pChannel->mRotationKeys[k].mValue.x;
						vRotation.y = pChannel->mRotationKeys[k].mValue.y;
						vRotation.z = pChannel->mRotationKeys[k].mValue.z;
						vRotation.w = pChannel->mRotationKeys[k].mValue.w;
					}
				}
				if (k < pChannel->mNumPositionKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mPositionKeys[k].mTime;
					memcpy(&vTranslation, &pChannel->mPositionKeys[k].mValue, sizeof(_float3));
				}
				KeyFrame.vScale = vScale;
				KeyFrame.vRotation = vRotation;
				KeyFrame.vTranslation = vTranslation;
				file.write(reinterpret_cast<const _char*>(&KeyFrame), sizeof(KEYFRAME));
			}
		}

		// 1. MorphMeshChannels 채널을 확인하고 있다면 데이터를 저장합니다.
		if (pAnimation->mNumMorphMeshChannels > 0)
		{
			// 1. 개수 확인.
			_uint iNumMorphMeshChannels = pAnimation->mNumMorphMeshChannels;
			file.write(reinterpret_cast<const _char*>(&iNumMorphMeshChannels), sizeof(_uint));

			for (size_t k = 0; k < iNumMorphMeshChannels; ++k)
			{
				aiMeshMorphAnim* pMorphChannel = pAnimation->mMorphMeshChannels[k];

				// 2. Shape Key 이름 찾기.
				aiString strCurveName = pMorphChannel->mName;
				_uint iNameLen = strCurveName.length;

				file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
				file.write(strCurveName.data, iNameLen);

				// 3. KeyFrame 개수 저장.
				_uint iNumKeys = pMorphChannel->mNumKeys;
				file.write(reinterpret_cast<const _char*>(&iNumKeys), sizeof(_uint));

				// 4. 키 프레임 데이터 (Time, Value) 저장.
				for (size_t key = 0; key < iNumKeys; ++key)
				{
					// mTime(double), mValues(unsigned int*), mWeights(double*)
					aiMeshMorphKey MorphKey = pMorphChannel->mKeys[key];

					KEYFRAME_CURVE KeyFrameCurve = {};
					_float fTrackPosition = static_cast<_float>(MorphKey.mTime);
					_float fValue = 0.f;

					// MorphKey 구조
					
					if (MorphKey.mNumValuesAndWeights > 0)
					{
						fValue = static_cast<_float>(MorphKey.mWeights[0]);
						fValue /= 100.f; // fWeight 정규화 /100.f
						fValue = max(0.0f, min(fValue, 1.0f)); // Clamp
					}

					KeyFrameCurve.fTrackPosition = fTrackPosition;
					KeyFrameCurve.fValue = fValue;
					KeyFrameCurve.iInterpolationType = ENUM_CLASS(KEY_INPTEROLATION::LINEAR);

					file.write(reinterpret_cast<const _char*>(&KeyFrameCurve), sizeof(KEYFRAME_CURVE));
				}
			}
		}
		else
		{
			// 1. 개수 확인. 없다면 0 저장.
			_uint iZero = 0;
			file.write(reinterpret_cast<const _char*>(&iZero), sizeof(_uint));
		}
	}

	file.close();

	return S_OK;
}

HRESULT CModelLoader::Save_Dat_NonAnim(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	ofstream file(pFileName, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Model Save Fail");
		return E_FAIL;
	}

	file.write(reinterpret_cast<const _char*>(&m_pAIScene->mNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = m_pAIScene->mMeshes[i];

		file.write(reinterpret_cast<const _char*>(&pMesh->mNumVertices), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumFaces), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mMaterialIndex), sizeof(_uint));

		VTXMESH* Vertices = new VTXMESH[pMesh->mNumVertices];
	
		for (size_t j = 0; j < pMesh->mNumVertices; ++j)
		{
			memcpy(&Vertices[j].vPosition, &pMesh->mVertices[j], sizeof(_float3));
			memcpy(&Vertices[j].vNormal, &pMesh->mNormals[j], sizeof(_float3));
			memcpy(&Vertices[j].vTangent, &pMesh->mTangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vBinormal, &pMesh->mBitangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vTexcoord, &pMesh->mTextureCoords[0][j], sizeof(_float2));
		}
		
		file.write(reinterpret_cast<const _char*>(Vertices), sizeof(VTXMESH) * pMesh->mNumVertices);
		Safe_Delete_Array(Vertices);

		_uint* Indices = new _uint[pMesh->mNumFaces * 3];
		_uint iIndex = {};
		for (size_t j = 0; j < pMesh->mNumFaces; ++j)
		{
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[0];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[1];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[2];
		}
		file.write(reinterpret_cast<const _char*>(Indices), sizeof(_uint) * pMesh->mNumFaces * 3);
		Safe_Delete_Array(Indices);
	}

	file.close();

	return S_OK;
}

HRESULT CModelLoader::Save_Material(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szMatFilePath[MAX_PATH] = {};
	strcpy_s(szMatFilePath, szDirPath);
	strcat_s(szMatFilePath, "Mat/");
	strcat_s(szMatFilePath, szFileName);
	strcat_s(szMatFilePath, ".json");

	filesystem::path dir = filesystem::path(szMatFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szMatFilePath);
	if (false == file.is_open())
	{
		MSG_BOX("Material Save Fail");
		return E_FAIL;
	}

	json OutData;
	OutData["NumMaterial"] = m_pAIScene->mNumMaterials;
	OutData["Materials"] = json::array();
	for (size_t i = 0; i < m_pAIScene->mNumMaterials; ++i)
	{
		aiMaterial* pMaterial = m_pAIScene->mMaterials[i];
		json MaterialData;
		Save_Texture(MaterialData, pMaterial, aiTextureType::aiTextureType_DIFFUSE);
		Save_Texture(MaterialData, pMaterial, aiTextureType::aiTextureType_NORMALS);
		OutData["Materials"].push_back(MaterialData);
	}

	file << OutData.dump(4);

	file.close();

	return S_OK;
}

void CModelLoader::Load_File()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Editor/Bin/Resource/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("FBX File Load", "Import File", ".fbx", config);

	ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
	ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린

	if (ImGuiFileDialog::Instance()->Display(
		"FBX File Load", ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			if (!m_isLoadAll)
			{
				_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
				m_strModelName = ImGuiFileDialog::Instance()->GetCurrentFileName();

				_uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
				if (MODELTYPE::NONANIM == m_eType)
					iFlag |= aiProcess_PreTransformVertices;
				m_pAIScene = m_Importer.ReadFile(strFilePath.c_str(), iFlag);
				if (nullptr == m_pAIScene)
				{
					MSG_BOX("AIScene Not Found");
					return;
				}
			}
			else
			{
				_string strCurrentFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

				_uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
				if (MODELTYPE::NONANIM == m_eType)
					iFlag |= aiProcess_PreTransformVertices;
				for (const auto& entry : filesystem::directory_iterator(strCurrentFilePath)) {
					if (entry.is_regular_file()) {
						if (entry.path().extension() == ".fbx")
						{
							_string strFilePath = entry.path().string();
							m_pAIScene = m_Importer.ReadFile(strFilePath.c_str(), iFlag);

							if (nullptr == m_pAIScene)
							{
								MSG_BOX("寃쎈줈 ?섎せ??");
								return;
							}
							//_string SaveFilePath = "../../Client/Bin/Resource/Map/Asphodel_Barrens/Tetragon_Hnuter's_Den/";
							_string SaveFilePath = "../../Client/Bin/Resource/Map/Test/";

							


							_string FileName = entry.path().filename().string();
							_string FolderPath;
							
							size_t CutPos = FileName.find("_LOD");

							if (CutPos != std::string::npos)
								FolderPath = FileName.substr(0, CutPos);
							SaveFilePath += "/" + FolderPath + "/";

							filesystem::create_directories(SaveFilePath);

							if (FileName.find(".mo") != string::npos)
								CutPos = FileName.find(".mo");
							else if (FileName.find(".ao") != string::npos)
								CutPos = FileName.find(".ao");
							if (CutPos != std::string::npos)
								FileName = FileName.substr(0, CutPos);

							SaveFilePath +=  FileName + ".dat";

							Save_Dat_NonAnim(SaveFilePath.c_str());
							Save_Material(SaveFilePath.c_str());
						}
					}
				}
			}
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowLoadFile = false;
	}
}

void CModelLoader::Save_File()
{
	if (nullptr == m_pAIScene)
		return;

	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/";
	config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

	ImGuiFileDialog::Instance()->OpenDialog("Save Model", "Export File", ".dat", config);

	ImVec2 vMinSize = ImVec2(600, 400); 
	ImVec2 vMaxSize = ImVec2(800, 400); 

	if (ImGuiFileDialog::Instance()->Display("Save Model"
		, ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize
	)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			if (MODELTYPE::NONANIM == m_eType)
				Save_Dat_NonAnim(strFilePath.c_str());
			else if (MODELTYPE::ANIM == m_eType)
			{
				Save_Dat_Anim(strFilePath.c_str());
				Save_Animation(strFilePath.c_str());
			}
			else if (MODELTYPE::CHARACTER == m_eType)
			{
				Save_Dat_Character(strFilePath.c_str()); // 캐릭터 전용 포맷(쉐이프키 포함) 저장
				Save_Animation_Character(strFilePath.c_str()); // 애니메이션 데이터도 필요하다면 저장 
			}
			Save_Material(strFilePath.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowSaveFile = false;
	}
}

void CModelLoader::Show_Info()
{
	if (nullptr == m_pAIScene)
		return;

	_uint iNumMesh = m_pAIScene->mNumMeshes;
	_uint iNumMat = m_pAIScene->mNumMaterials;
	_uint iNumAnim = m_pAIScene->mNumAnimations;

	_char szInfo[MAX_PATH] = {};
	sprintf_s(szInfo, "Mesh : %d / Mat : %d / Anim : %d", iNumMesh, iNumMat, iNumAnim);

	ImGui::PushID(1000);
	ImGui::Text(m_strModelName.c_str());
	ImGui::PopID();
	ImGui::PushID(1001);
	ImGui::Text(szInfo);
	ImGui::PopID();
}

HRESULT CModelLoader::Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType)
{
	json TypeData;
	_uint iTextureCnt = pMaterial->GetTextureCount(eType);
	TypeData["TextureCnt"] = iTextureCnt;
	TypeData["FileName"] = json::array();
	for (size_t k = 0; k < iTextureCnt; ++k)
	{
		aiString strFilePath = {};
		pMaterial->GetTexture(eType, k, &strFilePath);
		_char szName[MAX_PATH] = {};
		_char szExt[MAX_PATH] = {};
		_splitpath_s(strFilePath.data, nullptr, 0, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
		strcat_s(szName, szExt);
		TypeData["FileName"].push_back(szName);
	}
	if(aiTextureType::aiTextureType_DIFFUSE == eType)
		MaterialData["Diffuse"] = TypeData;
	if (aiTextureType::aiTextureType_NORMALS == eType)
		MaterialData["Normal"] = TypeData;

	return S_OK;
}

HRESULT CModelLoader::Save_Bone(ofstream& OutPut, const aiNode* pNode)
{
	OutPut.write(reinterpret_cast<const _char*>(&pNode->mNumChildren), sizeof(_uint));
	aiString strName = pNode->mName;
	_uint iLength = strName.length;
	OutPut.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
	OutPut.write(strName.data, iLength);
	OutPut.write(reinterpret_cast<const _char*>(&pNode->mTransformation), sizeof(_float4x4));
	//OutPut.write(reinterpret_cast<const _char*>(&pNode->mOffsetMatrix), sizeof(_float4x4));
	for (size_t i = 0; i < pNode->mNumChildren; ++i)
	{
		if (FAILED(Save_Bone(OutPut, pNode->mChildren[i])))
			return E_FAIL;
	}

	return S_OK;
}

CModelLoader* CModelLoader::Create()
{
	CModelLoader* pInstance = new CModelLoader();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : ModelLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModelLoader::Free()
{
	__super::Free();

	m_Importer.FreeScene();
}
