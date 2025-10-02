#include "EnginePch.h"
#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pBoneName);
	m_iParentBoneIndex = iParentBoneIndex;

	XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreTransformationMatrix * XMLoadFloat4x4(&m_TransformationMatrix));
		return;
	}
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, 
		XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

void CBone::Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const _float4x4* pBoneMatrix, const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreTransformationMatrix * XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(pBoneMatrix));
		return;
	}
	XMStoreFloat4x4(&m_CombinedTransformationMatrix,
		XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

CBone* CBone::Create(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pBoneName, TransformationMatrix, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Create : Bone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone* CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{
	__super::Free();
}
