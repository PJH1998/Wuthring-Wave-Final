#include "EditorPch.h"
#include "GltfLoader.h"

CGltfLoader::CGltfLoader()
{
}

HRESULT CGltfLoader::Initialize()
{
	return S_OK;
}

void CGltfLoader::Update()
{
	ImGui::Begin("GLTF Model Load");

	if (ImGui::RadioButton("Anim", m_iAnim == 1)) m_iAnim = 1;
	if (ImGui::RadioButton("Character", m_iAnim == 2)) m_iAnim = 2;

	if(1 == m_iAnim) m_eType = MODELTYPE::ANIM;
	else if(2 == m_iAnim) m_eType = MODELTYPE::CHARACTER;

	if (ImGui::Button("Load Gltf"))
		m_isShowLoadFile = !m_isShowLoadFile;
	ImGui::SameLine();
	if (ImGui::Button("Save Model"))
		m_isShowSaveFile = !m_isShowSaveFile;

	if(true == m_isShowLoadFile)
		Load_File();
	if (true == m_isShowSaveFile)
		Save_File();

	// GLTF Model Info
	Show_Info();

	ImGui::End();
}

HRESULT CGltfLoader::Save_Dat_Anim(const _char* pFileName)
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


HRESULT CGltfLoader::Save_Dat_Character(const _char* pFileName)
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

		// 최적화 1 루프 밖에서 임시 버퍼 미리 할당 (최대 버텍스 개수만큼)
		_float3* pDeltaPosBuffer = new _float3[pMesh->mNumVertices];
		_float3* pDeltaNormalBuffer = new _float3[pMesh->mNumVertices];

		for (size_t k = 0; k < pMesh->mNumAnimMeshes; ++k)
		{
			aiAnimMesh* pAnimMesh = pMesh->mAnimMeshes[k];
			
			// 1. 이름
			aiString strShapeName = pAnimMesh->mName;
			_uint iNameLen = strShapeName.length;
			file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strShapeName.data), iNameLen);

			// 2. 정점 개수
			_uint iNumVertices = pAnimMesh->mNumVertices;
			file.write(reinterpret_cast<const _char*>(&iNumVertices), sizeof(_uint));

			// 3. Delta Position (변화량 위치만 저장)
			// 델타 값을 담을 임시 배열 할당.
			for (_uint v = 0; v < iNumVertices; ++v)
			{
				_float fDeltaX = pAnimMesh->mVertices[v].x - pMesh->mVertices[v].x; // x 변화량만 저장
				_float fDeltaY = pAnimMesh->mVertices[v].y - pMesh->mVertices[v].y; // y 변화량만 저장
				_float fDeltaZ = pAnimMesh->mVertices[v].z - pMesh->mVertices[v].z; // z 변화량만 저장

				pDeltaPosBuffer[v].x = fDeltaX;
				pDeltaPosBuffer[v].y = fDeltaY;
				pDeltaPosBuffer[v].z = fDeltaZ;
			}

			// 계산된 Delta 값 저장.
			file.write(reinterpret_cast<const _char*>(pDeltaPosBuffer), sizeof(_float3) * iNumVertices);

			// 4. Delta Normal에 대한 존재 확인하기.
			_bool bHasNormal = (pAnimMesh->mNormals != nullptr);
			file.write(reinterpret_cast<const _char*>(&bHasNormal), sizeof(_bool));

			// 5. Normal이 존재한다면?
			if (bHasNormal)
			{
				const aiVector3D* pBaseNorm = pMesh->mNormals;
				const aiVector3D* pTargetNorm = pAnimMesh->mNormals;

				for (_uint v = 0; v < iNumVertices; ++v)
				{
					// Delta Normal 계산
					pDeltaNormalBuffer[v].x = pTargetNorm[v].x - pBaseNorm[v].x;
					pDeltaNormalBuffer[v].y = pTargetNorm[v].y - pBaseNorm[v].y;
					pDeltaNormalBuffer[v].z = pTargetNorm[v].z - pBaseNorm[v].z;
				}
				// 5. 계산된 Delta Normal 일괄 저장
				file.write(reinterpret_cast<const _char*>(pDeltaNormalBuffer), sizeof(_float3) * iNumVertices);
			}
				
		}
		// [최적화 1 종료] 루프가 다 끝난 뒤 메모리 해제
		Safe_Delete_Array(pDeltaPosBuffer);
		Safe_Delete_Array(pDeltaNormalBuffer);

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

HRESULT CGltfLoader::Save_Animation(const _char* pFileName)
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
		// Animation Duration
		file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));

		_float fTickPerSecond = pAnimation->mTicksPerSecond;
		// Animation TickPerSecond
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

HRESULT CGltfLoader::Save_Animation_Character(const _char* pFileName)
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
	strcat_s(szAnimFilePath, "_MorphAnim.dat");

	filesystem::path dir = filesystem::path(szAnimFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szAnimFilePath, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Animation Save Fail");
		return E_FAIL;
	}

	//_uint iNumAnimations = m_pAIScene->mNumAnimations;
	/*file.write(reinterpret_cast<const _char*>(&iNumAnimations), sizeof(_uint));*/

	map<_string, ANIM_SET> mapAnimGroups;

	// 1. animMap에 읽을 파일들 저장하기.
	for (size_t i = 0; i < m_pAIScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnim = m_pAIScene->mAnimations[i];
		string strFullName = pAnim->mName.C_Str();
		string strBaseName = strFullName;

#pragma region 1. Curve애니메이션과 아닌 애니메이션을 구분해서 map에 기록.
		// 1. 접미사 처리: _Curves가 붙어있다면 Morph용 애니메이션으로 간주하고 이름을 분리
		_bool isMorphOnly = false;
		size_t findRes = strFullName.find("_Curves");
		if (findRes != string::npos)
		{
			strBaseName = strFullName.substr(0, findRes); // "Attack01_Curves" -> "Attack01"
			isMorphOnly = true;
		}

		// 2. 맵에 넣기
		if (isMorphOnly)
		{
			mapAnimGroups[strBaseName].pMorphAnim = pAnim;
		}
		else
		{
			// _Curves가 없으면 일단 Main으로 등록
			mapAnimGroups[strBaseName].pMainAnim = pAnim;

			// Main 안에 Morph 채널도 같이 들어있는 경우
			if (pAnim->mNumMorphMeshChannels > 0 && mapAnimGroups[strBaseName].pMorphAnim == nullptr)
				mapAnimGroups[strBaseName].pMorphAnim = pAnim;
		}
#pragma endregion
	}


	// 2. 정렬된 Map을 순회하며 파일 쓰기.
	_uint iNumMergedAnims = static_cast<_uint>(mapAnimGroups.size());
	file.write(reinterpret_cast<const _char*>(&iNumMergedAnims), sizeof(_uint));

	for (auto& Pair : mapAnimGroups)
	{
		_float fTimeScale = {};
		

#pragma region 2. map을 순회하면서 Main Anmation 정보를 저장.
		if (nullptr != Pair.second.pMainAnim)
		{
			aiAnimation* pAnimation = Pair.second.pMainAnim; // MainAnimations;
			aiString strName = pAnimation->mName;


			_uint iLength = strName.length;

			// Animation Name 저장.
			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(strName.data, iLength);

			// 24.f 고정 => 기존 TickPerSecond.
			_float fTargetFPS = 24.0f;
			_float fSourceFPS = static_cast<_float>(pAnimation->mTicksPerSecond);
			if (fSourceFPS <= 0.1f) fSourceFPS = 1000.0f;

			// 변환 비율 계산 
			fTimeScale = fTargetFPS / fSourceFPS;
			_float fDuration = static_cast<_float>(pAnimation->mDuration) * fTimeScale;

			file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));
			file.write(reinterpret_cast<const _char*>(&fTargetFPS), sizeof(_float));

			_uint iNumChannels = pAnimation->mNumChannels;
			// Num Channel
			file.write(reinterpret_cast<const _char*>(&iNumChannels), sizeof(_uint));


			// 4. Channel 정보 저장.
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
						KeyFrame.fTrackPosition = pChannel->mScalingKeys[k].mTime * fTimeScale;
						memcpy(&vScale, &pChannel->mScalingKeys[k].mValue, sizeof(_float3));
					}
					if (k < pChannel->mNumRotationKeys)
					{
						KeyFrame.fTrackPosition = pChannel->mRotationKeys[k].mTime * fTimeScale;

						{
							vRotation.x = pChannel->mRotationKeys[k].mValue.x;
							vRotation.y = pChannel->mRotationKeys[k].mValue.y;
							vRotation.z = pChannel->mRotationKeys[k].mValue.z;
							vRotation.w = pChannel->mRotationKeys[k].mValue.w;
						}
					}
					if (k < pChannel->mNumPositionKeys)
					{
						KeyFrame.fTrackPosition = pChannel->mPositionKeys[k].mTime * fTimeScale;
						memcpy(&vTranslation, &pChannel->mPositionKeys[k].mValue, sizeof(_float3));

						// .gltf의 경우에는 0.01f 설정
					}

					KeyFrame.vScale = vScale;
					KeyFrame.vRotation = vRotation;
					KeyFrame.vTranslation = vTranslation;
					file.write(reinterpret_cast<const _char*>(&KeyFrame), sizeof(KEYFRAME));
				}
			}
		}
		else
			continue; // MainAnim이 없으면 기록도 하지않음 => Curves만 들어간 경우.
#pragma endregion

#pragma region 3. map을 순회하면서 Morph Animation 정보를 저장.
		map<string, vector<KEYFRAME_CURVE>> mapMorphCurves;

		// Key: 쉐이프키 이름 ("Smile"), Value: 해당 키의 시간별 변화량 목록 => 0.f[FRAME0], 1.f[FRAME1] ....
		if (nullptr != Pair.second.pMorphAnim)
		{
			aiAnimation* pMorphAnimation = Pair.second.pMorphAnim;
			// 1. MorphMeshChannels 채널을 확인하고 있다면 데이터를 저장합니다. => 무조건 하나만 나옴 Object가 하나라.
			if (pMorphAnimation->mNumMorphMeshChannels > 0)
			{
				for (size_t i = 0; i < pMorphAnimation->mNumMorphMeshChannels; ++i)
				{
					aiMeshMorphAnim* pMeshMorphAnim = pMorphAnimation->mMorphMeshChannels[i];

					// 1. 채널 이름으로 타겟 메쉬 찾기 => 그냥 다 똑같으므로. m_pAIScene의 첫번째 메시 가져오기.
					aiMesh* pTargetMesh = FindMeshByMorphChannelName(pMeshMorphAnim->mName);
					//aiMesh* pTargetMesh = m_pAIScene->mMeshes[0];
					if (nullptr == pTargetMesh) continue;

					// 2. 시간(Keys)을 기준으로 먼저 순회합니다. Assimp는 시간 -> 활성화된 쉐이프키 목록 순서로 저장
					// 183 TrackPosition 이면. 1 TrackPosition => 105 ShapeKey 이렇게 저장됨. Assimp 에는.
					for (_uint keyIdx = 0; keyIdx < pMeshMorphAnim->mNumKeys; ++keyIdx)
					{
						// ShapeKey 목록을 순회.
						const aiMeshMorphKey& MorphKey = pMeshMorphAnim->mKeys[keyIdx];

						// 이 시간대에 변화가 있는 모든 쉐이프 키들을 순회 
						for (unsigned int v = 0; v < MorphKey.mNumValuesAndWeights; ++v)
						{
							// mValues[v] == 쉐이프키 인덱스.
							_uint iShapeIdx = MorphKey.mValues[v];

							// 인덱스
							if (iShapeIdx >= pTargetMesh->mNumAnimMeshes) continue;

							// 인덱스로부터 쉐이프 키 이름("Smile") 추출
							aiAnimMesh* pAnimMesh = pTargetMesh->mAnimMeshes[iShapeIdx];
							string strShapeKeyName = pAnimMesh->mName.C_Str();

							// "Basis" 등 불필요한 키 제외
							if (strShapeKeyName == "Basis" || strShapeKeyName.empty()) continue;

							// 가중치 처리 => Weight는 
							_float fWeight = static_cast<_float>(MorphKey.mWeights[v]); // 메시 ShapeKey에 대한 가중치 저장.

							//_float fWeight = static_cast<_float>(MorphKey.mWeights[v]);
							//if (fWeight > 0.f) fWeight /= 100.f; // 정규화 (0~100 -> 0~1)
							//fWeight = max(0.0f, min(fWeight, 1.0f));

							// 맵에 데이터 추가 
							KEYFRAME_CURVE KeyFrame = {};
							KeyFrame.fTrackPosition = static_cast<_float>(MorphKey.mTime) * fTimeScale;
							KeyFrame.fValue = fWeight;

							mapMorphCurves[strShapeKeyName].push_back(KeyFrame);
						}
					}
				}
			}
		}

		// 정리된 데이터를 파일에 저장 => if 문 밖에서 확인하기. => MorphCurve 데이터가 없으면 0이라도 기록해두기 위함.
		_uint iTotalCurves = static_cast<_uint>(mapMorphCurves.size());
		file.write(reinterpret_cast<const _char*>(&iTotalCurves), sizeof(_uint));

		// 맵을 순회 하면서 데이터 저장.
		for (auto& Pair : mapMorphCurves)
		{
			string strCurveName = Pair.first;     // 이름
			auto& vecKeys = Pair.second;          // 키프레임들

			// 이름 저장
			_uint iNameLen = static_cast<_uint>(strCurveName.length());
			file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
			file.write(strCurveName.data(), iNameLen);

			// 키 개수 및 데이터 저장
			_uint iNumKeys = static_cast<_uint>(vecKeys.size());
			file.write(reinterpret_cast<const _char*>(&iNumKeys), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(vecKeys.data()), sizeof(KEYFRAME_CURVE) * iNumKeys);
		}
#pragma endregion

	}

	file.close();

	return S_OK;
}

HRESULT CGltfLoader::Save_Material(const _char* pFileName)
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

void CGltfLoader::Load_File()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Editor/Bin/Resource/Player/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("GLTF File Load", "Import File", ".gltf", config);

	ImVec2 vMinSize = ImVec2(600, 400);
	ImVec2 vMaxSize = ImVec2(800, 400);

	if (ImGuiFileDialog::Instance()->Display(
		"GLTF File Load", ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
			m_strModelName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			// 임포트한 파일의 폴더 경로만 추출해서 저장.
			filesystem::path pathObj(strFilePath);
			m_strImportDir = pathObj.parent_path().string();

			_uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
			m_pAIScene = m_Importer.ReadFile(strFilePath.c_str(), iFlag);
			if (nullptr == m_pAIScene)
			{
				MSG_BOX("AIScene Not Found");
				return;
			}
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowLoadFile = false;
	}
}

void CGltfLoader::Save_File()
{
	if (nullptr == m_pAIScene)
		return;

	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Model/Player/";
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

			if (MODELTYPE::ANIM == m_eType)
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

void CGltfLoader::Show_Info()
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

aiMesh* CGltfLoader::FindMeshByMorphChannelName(const aiString& strMorphChannelName)
{
	string strNodeName = strMorphChannelName.C_Str();

	// 1. *0 제거
	size_t starPos = strNodeName.find('*');
	if (starPos != std::string::npos)
		strNodeName = strNodeName.substr(0, starPos);

	// 2. 타겟 노드 찾기.
	aiNode* pTargetNode = Find_Node(m_pAIScene->mRootNode, strNodeName);


	// 3. 노드를 못 찾음 (이름 불일치 등)
	if (nullptr == pTargetNode)
		return nullptr;

	// 4. 찾은 Node에 연결된 메쉬 인덱스들을 순회하며 Shape Key가 있는 메쉬를 찾음
	//    (Blender의 Mesh 오브젝트 하나가 Material 개수만큼 쪼개져 있으므로 순회 필요)
	for (unsigned int i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		_uint iMeshIndex = pTargetNode->mMeshes[i];
		aiMesh* pMesh = m_pAIScene->mMeshes[iMeshIndex];


		// Shape Key(AnimMeshes) 데이터가 존재하는지 확인
		if (pMesh->mNumAnimMeshes > 0)
			return pMesh; // 진짜 데이터를 가진 메쉬 반환
	}

	return nullptr;
}

aiNode* CGltfLoader::Find_Node(aiNode* pNode, const _string& strNodeName)
{
	if (!pNode) return nullptr;

	// 현재 노드 이름과 찾는 이름이 같으면 반환
	if (strNodeName == pNode->mName.C_Str())
		return pNode;

	// 자식 노드들 순회
	for (_uint i = 0; i < pNode->mNumChildren; ++i)
	{
		aiNode* pResult = Find_Node(pNode->mChildren[i], strNodeName);
		if (pResult) return pResult;
	}

	return nullptr;
}


HRESULT CGltfLoader::Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType)
{
	json TypeData;
	_uint iTextureCnt = pMaterial->GetTextureCount(eType);
	TypeData["TextureCnt"] = iTextureCnt;
	TypeData["FileName"] = json::array();
	for (size_t k = 0; k < iTextureCnt; ++k)
	{
		aiString strFilePath = {};
		pMaterial->GetTexture(eType, k, &strFilePath);

		// 1. 파일명 추출.
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

HRESULT CGltfLoader::Save_Bone(ofstream& OutPut, const aiNode* pNode)
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

CGltfLoader* CGltfLoader::Create()
{
	CGltfLoader* pInstance = new CGltfLoader();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : ModelLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGltfLoader::Free()
{
	__super::Free();
	m_Importer.FreeScene();
}
