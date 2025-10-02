#include "EnginePch.h"
#include "Shadow.h"

#include "Shader.h"

CShadow::CShadow()
{
}

void CShadow::Update_Transform(const _fvector& vAt)
{
	XMStoreFloat3(&m_MainShadowDesc.vAt, vAt);
	XMStoreFloat3(&m_MainShadowDesc.vEye, vAt - XMLoadFloat3(&m_vLookDir) * m_fDistance);

	XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)], XMMatrixLookAtLH(
		XMVectorSetW(XMLoadFloat3(&m_MainShadowDesc.vEye), 1.f), 
		XMVectorSetW(XMLoadFloat3(&m_MainShadowDesc.vAt), 0.f), 
		XMVectorSet(0.f, 1.f, 0.f, 0.f)));
}

HRESULT CShadow::Bind_Shadow_Resource(class CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pFarName)
{
	if (FAILED(pShader->Bind_Matrix(pViewName, Get_Matrix(D3DTS::VIEW))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix(pProjName, Get_Matrix(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Value(pFarName, &m_MainShadowDesc.fFar, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

HRESULT CShadow::Initialize(_float fWidth, _float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	return S_OK;
}

HRESULT CShadow::Ready_ShadowLight(const SHADOW_LIGHT_DESC& Desc)
{
	memcpy(&m_MainShadowDesc, &Desc, sizeof(SHADOW_LIGHT_DESC));
	XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)], XMMatrixLookAtLH(XMVectorSetW(XMLoadFloat3(&Desc.vEye), 1.f), XMVectorSetW(XMLoadFloat3(&Desc.vAt), 0.f), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	//XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)], XMMatrixPerspectiveFovLH(Desc.fFovy, m_fWidth / m_fHeight, Desc.fNear, Desc.fFar));
	XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)], XMMatrixOrthographicLH(m_fWidth, m_fHeight, Desc.fNear, Desc.fFar));

	_vector vDistance = XMLoadFloat3(&m_MainShadowDesc.vAt) - XMLoadFloat3(&m_MainShadowDesc.vEye);
	m_fDistance = XMVectorGetX(XMVector3Length(vDistance));
	XMStoreFloat3(&m_vLookDir, XMVector3Normalize(vDistance));

	return S_OK;
}

CShadow* CShadow::Create(_float fWidth, _float fHeight)
{
	CShadow* pInstance = new CShadow();

	if (FAILED(pInstance->Initialize(fWidth, fHeight)))
	{
		MSG_BOX("Failed to Create : Shadow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShadow::Free()
{
	__super::Free();
}
