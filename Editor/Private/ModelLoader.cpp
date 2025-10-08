#include "EditorPch.h"
#include "ModelLoader.h"

CModelLoader::CModelLoader()
{
}

HRESULT CModelLoader::Initialize(MODELTYPE eType, const _char* pModelFilePath)
{
	if (nullptr == pModelFilePath)
		return S_OK;

	_char szFilePath[MAX_PATH] = "../Bin/Resource/FBX/";
	strcat_s(szFilePath, pModelFilePath);
	strcat_s(szFilePath, ".fbx");

	m_eType = eType;

	_uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
	if (MODELTYPE::NONANIM == eType)
		iFlag |= aiProcess_PreTransformVertices;
	m_pAIScene = m_Importer.ReadFile(szFilePath, iFlag);
	if (nullptr == m_pAIScene)
	{
		MSG_BOX("Model Load Fail");
		return E_FAIL;
	}
	
	return S_OK;
}

HRESULT CModelLoader::Save_Dat_Anim(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szBasePath[MAX_PATH] = "../Bin/dat/";
	_char szExt[MAX_PATH] = ".dat";
	strcat_s(szBasePath, pFileName);
	strcat_s(szBasePath, szExt);

	ofstream file(szBasePath, ios::binary);

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

HRESULT CModelLoader::Save_Animation(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szBasePath[MAX_PATH] = "../Bin/dat/Animation/";
	_char szExt[MAX_PATH] = "_Anim.dat";
	strcat_s(szBasePath, pFileName);
	strcat_s(szBasePath, szExt);

	ofstream file(szBasePath, ios::binary);

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
		// Animation Duration (지속시간)
		file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));

		_float fTickPerSecond = pAnimation->mTicksPerSecond;
		// Animation TickPerSecond (초당 이동량)
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
					vRotation.x = pChannel->mRotationKeys[k].mValue.x;
					vRotation.y = pChannel->mRotationKeys[k].mValue.y;
					vRotation.z = pChannel->mRotationKeys[k].mValue.z;
					vRotation.w = pChannel->mRotationKeys[k].mValue.w;
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

	return S_OK;
}

HRESULT CModelLoader::Save_Dat_NonAnim(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szBasePath[MAX_PATH] = "../Bin/dat/";
	_char szExt[MAX_PATH] = ".dat";
	strcat_s(szBasePath, pFileName);
	strcat_s(szBasePath, szExt);

	ofstream file(szBasePath, ios::binary);

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

	_char szBasePath[MAX_PATH] = "../Bin/json/Material/";
	_char szExt[MAX_PATH] = ".json";
	strcat_s(szBasePath, pFileName);
	strcat_s(szBasePath, szExt);

	ofstream file(szBasePath);
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

	for (size_t i = 0; i < pNode->mNumChildren; ++i)
	{
		if (FAILED(Save_Bone(OutPut, pNode->mChildren[i])))
			return E_FAIL;
	}

	return S_OK;
}

CModelLoader* CModelLoader::Create(MODELTYPE eType, const _char* pModelFilePath)
{
	CModelLoader* pInstance = new CModelLoader();

	if (FAILED(pInstance->Initialize(eType, pModelFilePath)))
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
