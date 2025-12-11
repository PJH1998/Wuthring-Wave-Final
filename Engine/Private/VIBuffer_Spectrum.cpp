#include "EnginePch.h"
#include "VIBuffer_Spectrum.h"

CVIBuffer_Spectrum::CVIBuffer_Spectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Spectrum::CVIBuffer_Spectrum(const CVIBuffer_Spectrum& Prototype)
	:CVIBuffer { Prototype }
    , m_iMaxSamples { Prototype.m_iMaxSamples }
    , m_fSize { Prototype.m_fSize }
{
}

HRESULT CVIBuffer_Spectrum::Initialize_Prototype(VB_SPECTRUM_DESC* pDesc)
{
    m_iMaxSamples = pDesc->MaxSamples;
    m_fSize = pDesc->fSize;

    m_iNumVertices = 2 * pDesc->MaxSamples;
    m_iVertexStride = sizeof(VTXPOSTEX);
    m_iNumVertexBuffers = 1;
    m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	return S_OK;
}

HRESULT CVIBuffer_Spectrum::Initialize_Clone(void* pArg)
{
	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DYNAMIC;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	//인덱스 없이 버퍼로만 그릴거임.
	//그릴 때는 VertexCount 를 설정 해줘야함 (바인딩 되어있는 버터가 몇개인지 알려주기)
	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, nullptr, &m_pVB)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Spectrum::Render()
{
    m_pContext->Draw(m_iVtxCount, 0);

    return S_OK;
}

HRESULT CVIBuffer_Spectrum::Bind_Resources()
{
    ID3D11Buffer* Buffers[] = {
      m_pVB,
    };

    _uint Strides[] = {
        m_iVertexStride,
    };

    _uint Offsets[] = {
        0,
    };

    m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, Buffers, Strides, Offsets);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

    return S_OK;
}

void CVIBuffer_Spectrum::Update_Spectrum(deque<SAMPLE_DESC>& vSamples, _int SampleCount, const _float4* vCamPos)
{
    if (SampleCount < 2)
        return;

	if (SampleCount > m_iMaxSamples)
	{
		_uint iExcessCount = SampleCount - m_iMaxSamples;

		for (_int i = 0; i < iExcessCount; ++i)
		{
			vSamples.pop_front();
		}
	}

	m_iVtxCount = 2 * vSamples.size();

    D3D11_MAPPED_SUBRESOURCE	Resource{};

    m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);

    VTXPOSTEX* pVertices = static_cast<VTXPOSTEX*>(Resource.pData);
    
    for (size_t i = 0; i < vSamples.size(); i++)
    {
        SAMPLE_DESC Desc = vSamples[i];
        XMVECTOR Dir = {};
        XMVECTOR ViewDir = {};
        XMVECTOR vSide = {};

        if (i > 0)
        {
            SAMPLE_DESC PrevDesc = vSamples[i - 1];
            Dir = XMLoadFloat3(&Desc.vPos) - XMLoadFloat3(&PrevDesc.vPos);
        }
        else
        {
            SAMPLE_DESC NextDesc = vSamples[i + 1];
            Dir = XMLoadFloat3(&NextDesc.vPos) - XMLoadFloat3(&Desc.vPos);
        }

        ViewDir = XMVector3Normalize((XMLoadFloat4(vCamPos) -
            XMVectorSet(Desc.vPos.x, Desc.vPos.y, Desc.vPos.z, 1.f)));

        vSide =  XMVector3Normalize(XMVector3Cross(ViewDir, Dir));

        _float3 vPosUp = Desc.vPos;
        _float3 vPosDown = Desc.vPos;

        XMStoreFloat3(&vPosUp, XMLoadFloat3(&vPosUp) - vSide * (m_fSize * 0.5f));
        XMStoreFloat3(&vPosDown, XMLoadFloat3(&vPosDown) + vSide * (m_fSize * 0.5f));

        float fUV_V = (_float)i / (vSamples.size() - 1);

        pVertices[2 * i].vPosition = vPosUp;
        pVertices[2 * i + 1].vPosition = vPosDown;

        pVertices[2 * i].vTexcoord = _float2(0.f, fUV_V);
        pVertices[2 * i + 1].vTexcoord = _float2(1.f, fUV_V);
    }

    m_pContext->Unmap(m_pVB, 0);
}

CVIBuffer_Spectrum* CVIBuffer_Spectrum::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, VB_SPECTRUM_DESC* pDesc)
{
    CVIBuffer_Spectrum* pInstance = new CVIBuffer_Spectrum(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_Spectrum");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Spectrum::Clone(void* pArg)
{
    CVIBuffer_Spectrum* pClone = new CVIBuffer_Spectrum(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_Spectrum (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Spectrum::Free()
{
    __super::Free();
}
