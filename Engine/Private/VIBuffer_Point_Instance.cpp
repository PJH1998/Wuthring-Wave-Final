#include "EnginePch.h"
#include "VIBuffer_Point_Instance.h"
#include "GameInstance.h"

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance { pDevice, pContext }
{
}

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype)
	: CVIBuffer_Instance{ Prototype }
	, m_vPivot{ Prototype.m_vPivot }
	, m_pSpeeds{ Prototype.m_pSpeeds }
	, m_isLoop{ Prototype.m_isLoop }
	, m_pCBBuffer { Prototype.m_pCBBuffer}
	, m_pSRV { Prototype.m_pSRV }
	, m_pSRVBuffer { Prototype.m_pSRVBuffer}
{
	Safe_AddRef(m_pCBBuffer);
	Safe_AddRef(m_pSRVBuffer);
	Safe_AddRef(m_pSRV);
}

HRESULT CVIBuffer_Point_Instance::Initialize_Prototype(const INSTANCE_DESC* pDesc)
{
	const POINT_INSTANCE_DESC* pPointDesc = static_cast<const POINT_INSTANCE_DESC*>(pDesc);

	m_vPivot = pPointDesc->vPivot;
	m_isLoop = pPointDesc->IsLoop;

	m_iInstanceVertexStride = sizeof(VTXINSTANCE_PARTICLE);
	m_iNumInstance = pPointDesc->iNumInstance;
	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumVertexBuffers = 2;
	m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;


	D3D11_BUFFER_DESC		VBDesc{};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	VTXPOS* pVertices = new VTXPOS[m_iNumVertices];

	pVertices[0].vPosition = _float3(0.0f, 0.0f, 0.f);

	D3D11_SUBRESOURCE_DATA	VBInitialData{};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.MiscFlags = 0;
	m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

	m_pVBInstanceVertices = new VTXINSTANCE_PARTICLE[m_iNumInstance];
	m_pSpeeds = new _float[m_iNumInstance];

	//SRV???ㅼ뼱媛??뺣낫 援ъ“泥댁뿉 媛쒖닔留뚰겮 ???
	PARTICLE_SRV* pSRV = new PARTICLE_SRV[m_iNumInstance];

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

		_float		fScale = m_pGameInstance->Rand(pPointDesc->vSize.x, pPointDesc->vSize.y);
		_float		fLifeTime = m_pGameInstance->Rand(pPointDesc->vLifeTime.x, pPointDesc->vLifeTime.y);
		m_pSpeeds[i] = m_pGameInstance->Rand(pPointDesc->vSpeed.x, pPointDesc->vSpeed.y);
		//SRV?곗씠?곗슜 ???
		pSRV[i].fSpeed = m_pSpeeds[i];

		pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
		pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
		pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
		pInstanceVertices[i].vTranslation = _float4(
			m_pGameInstance->Rand(pPointDesc->vCenter.x - pPointDesc->vRange.x * 0.5f, pPointDesc->vCenter.x + pPointDesc->vRange.x * 0.5f),
			m_pGameInstance->Rand(pPointDesc->vCenter.y - pPointDesc->vRange.y * 0.5f, pPointDesc->vCenter.y + pPointDesc->vRange.y * 0.5f),
			m_pGameInstance->Rand(pPointDesc->vCenter.z - pPointDesc->vRange.z * 0.5f, pPointDesc->vCenter.z + pPointDesc->vRange.z * 0.5f),
			1.f
		);

		pInstanceVertices[i].vLifeTime = _float2(0.f, fLifeTime);

		//SRV?곗씠??珥덇린 ?꾩튂 ??μ슜
		pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
	}
	
	//SRV??踰꾪띁 ?앹꽦
	D3D11_BUFFER_DESC SRV_BufferDesc = {};
	SRV_BufferDesc.StructureByteStride = sizeof(PARTICLE_SRV);
	SRV_BufferDesc.ByteWidth = SRV_BufferDesc.StructureByteStride * m_iNumInstance;
	SRV_BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;				//遺덈?
	SRV_BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		//由ъ냼??
	SRV_BufferDesc.CPUAccessFlags = 0; 
	SRV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA SRVInitialData{};
	SRVInitialData.pSysMem = pSRV;

	if (FAILED(m_pDevice->CreateBuffer(&SRV_BufferDesc, &SRVInitialData, &m_pSRVBuffer)))
		return E_FAIL;

	Safe_Delete_Array(pSRV);

	//SRV 踰꾪띁瑜??듯빐 由ъ냼?ㅻ럭 ?앹꽦
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = SRV_BufferDesc.ByteWidth / SRV_BufferDesc.StructureByteStride;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pSRVBuffer, &SRVDesc, &m_pSRV)))
		return E_FAIL;

	//CB 踰꾪띁 ?앹꽦
	PARTICLE_CB* pCB = new PARTICLE_CB;
	pCB->fTimeDelta = 0.1f;
	pCB->vPivot = m_vPivot;
	pCB->IsLoop = m_isLoop ? 1 : 0;
	pCB->fSpreadWeight = pPointDesc->fSpreadWeight;
	pCB->fDropWeight = pPointDesc->fDropWeight;
	pCB->fRotationWeight = pPointDesc->fRotationWeight;
	pCB->fGravity = pPointDesc->fGravity;


	D3D11_BUFFER_DESC CB_BufferDesc = {};
	CB_BufferDesc.StructureByteStride = 0;
	CB_BufferDesc.ByteWidth = sizeof(PARTICLE_CB);				 //16諛붿씠??諛곗닔濡?留욎텣 援ъ“泥??꾩슂
	CB_BufferDesc.Usage = D3D11_USAGE_DYNAMIC ;					//?먯＜蹂??
	CB_BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//酉??놁쓬
	CB_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CB_BufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA CBInitialData{};
	CBInitialData.pSysMem = pCB;

	if (FAILED(m_pDevice->CreateBuffer(&CB_BufferDesc, &CBInitialData, &m_pCBBuffer)))
		return E_FAIL;

	Safe_Delete(pCB);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	//UAV ?대줎?먯꽌 ?앹꽦?댁쨾?쇳븿 (?묎컳? ?ㅼ젙媛믪쓣 媛吏??뚰떚?댁쓣 ?대줎?댁꽌 ?щ윭媛?留뚮뱾?덉쓣 ?? 媛숈? UAV瑜?怨듭쑀?섎㈃ 紐⑤뱺 ?뚰떚?댁씠 ?숈씪???吏곸엫??媛吏寃???)
	// SRV? CB???꾨줈?좏??낆뿉???앹꽦?댁쨾????
	// SRV??遺덈???媛쒕퀎?몄뒪?댁뒪 媛믩뱾?대씪 ?꾨줈?좏??낆뿉 留뚮뱾?댁꽌 ?대줎?쇰━ 怨듭쑀?대룄 臾몄젣 ?놁쓬,
	// CB??蹂?섎뒗 媛?(??꾨뜽?) 媛숈? 蹂?섎뱾???ㅼ뼱媛吏留? 留??꾨젅?꾨쭏??留? ?몃㏊?쇰줈 媛??ㅼ젙?댁쨾?쇳븯???꾨줈?좏??낆뿉 留뚮뱾?대룄 ?곴??놁쓬

	
	//?ш린??UAV 踰꾪띁 留뚮뱾怨? 由ъ냼?ㅻ럭 留뚮뱾?댁쨾?쇳븿.
	D3D11_BUFFER_DESC UAV_BufferDesc = {};
	UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
	UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
	UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;						//UAV???뷀뤃??
	UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;			//?ㅻⅨ怨녹뿉?쒕룄 ?쎌쓣嫄곕㈃  | D3D11_BIND_SHADER_RESOURCE ?댁쨾?쇳븿
	UAV_BufferDesc.CPUAccessFlags = 0;			
	UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;	


	D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
	UAVInitialDesc.pSysMem = m_pVBInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, &UAVInitialDesc, &m_pUABuffer)))
		return E_FAIL;

	//UAV 踰꾪띁瑜??듯빐 由ъ냼?ㅻ럭 ?앹꽦
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	UAVDesc.Buffer.NumElements = UAV_BufferDesc.ByteWidth / UAV_BufferDesc.StructureByteStride;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUABuffer, &UAVDesc, &m_pUAV)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Bind_Resources()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride,
	};

	_uint		iOffsets[] = {
		0,
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Render()
{
	m_pContext->DrawInstanced(1, m_iNumInstance, 0, 0);

	return S_OK;
}

void CVIBuffer_Point_Instance::Bind_CSResources(CComputeShader* pCShader, _float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	PARTICLE_CB* pCB = static_cast<PARTICLE_CB*>(SubResource.pData);

	pCB->fTimeDelta = fTimeDelta;

	m_pContext->Unmap(m_pCBBuffer, 0);

	pCShader->Set_ConstantBuffer("CB", m_pCBBuffer);

	pCShader->Set_SRV("g_ParticleStatic", m_pSRV);

	pCShader->Set_UAV("g_ParticleState", m_pUAV);

	pCShader->Dispatch(128, 1, 1);

	//GPU?먯꽌 蹂듭궗 吏꾪뻾?? ?대??먯꽌 ?곗궛?묒뾽???앸궗?붿? ?뺤씤?섍퀬 蹂듭궗 吏꾪뻾?댁??ㅺ퀬 ??
	m_pContext->CopyResource(m_pVBInstance, m_pUABuffer);
}

void CVIBuffer_Point_Instance::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXINSTANCE_PARTICLE* pVertices = static_cast<VTXINSTANCE_PARTICLE*>(SubResource.pData);


	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector	vMoveDir = XMVector3Normalize(XMVectorSetW(XMLoadFloat4(&pVertices[i].vTranslation) - XMLoadFloat3(&m_vPivot), 0.f));

		XMStoreFloat4(&pVertices[i].vTranslation, XMLoadFloat4(&pVertices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);
		pVertices[i].vLifeTime.x += fTimeDelta;

		if (true == m_isLoop)
		{
			if (pVertices[i].vLifeTime.x >= pVertices[i].vLifeTime.y)
			{
				pVertices[i].vLifeTime.x = 0.f;
				pVertices[i].vTranslation = pInstanceVertices[i].vTranslation;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Point_Instance::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXINSTANCE_PARTICLE* pVertices = static_cast<VTXINSTANCE_PARTICLE*>(SubResource.pData);


	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector	vMoveDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);

		XMStoreFloat4(&pVertices[i].vTranslation, XMLoadFloat4(&pVertices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);
		pVertices[i].vLifeTime.x += fTimeDelta;

		if (true == m_isLoop)
		{
			if (pVertices[i].vLifeTime.x >= pVertices[i].vLifeTime.y)
			{
				pVertices[i].vLifeTime.x = 0.f;
				pVertices[i].vTranslation = pInstanceVertices[i].vTranslation;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Point_Instance::Rotation(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXINSTANCE_PARTICLE* pVertices = static_cast<VTXINSTANCE_PARTICLE*>(SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector vTranslation = XMLoadFloat4(&pVertices[i].vTranslation);
		_vector vPivot = XMLoadFloat3(&m_vPivot);

		_vector vLocal = vTranslation - vPivot;

		_vector vAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		_float fAngle = m_pSpeeds[i] * fTimeDelta;

		_matrix matRot = XMMatrixRotationAxis(vAxis, fAngle);
		vLocal = XMVector3TransformNormal(vLocal, matRot);

		vTranslation = vLocal + vPivot;

		pVertices[i].vLifeTime.x += fTimeDelta;

		if (true == m_isLoop)
		{
			if (pVertices[i].vLifeTime.x >= pVertices[i].vLifeTime.y)
			{
				pVertices[i].vLifeTime.x = 0.f;
				pVertices[i].vTranslation = pInstanceVertices[i].vTranslation;
			}
		}
	}
}

CVIBuffer_Point_Instance* CVIBuffer_Point_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc)
{
	CVIBuffer_Point_Instance* pInstance = new CVIBuffer_Point_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pDesc)))
	{
		MSG_BOX("Failed to Create : CVIBuffer_Point_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Point_Instance::Clone(void* pArg)
{
	CVIBuffer_Point_Instance* pClone = new CVIBuffer_Point_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CVIBuffer_Point_Instance");
		Safe_Release(pClone);
	}

	return pClone;
}

void CVIBuffer_Point_Instance::Free()
{
	__super::Free();

	if (false == m_isClone)
	{
		Safe_Delete_Array(m_pSpeeds);
	}

	Safe_Release(m_pSRV);
	Safe_Release(m_pCBBuffer);
	Safe_Release(m_pSRVBuffer);
	Safe_Release(m_pUABuffer);
	Safe_Release(m_pUAV);
}
