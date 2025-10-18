#include "EnginePch.h"
#include "VIBuffer_Mesh.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype)
    : CVIBuffer { Prototype }
{
}

HRESULT CVIBuffer_Mesh::Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath)
{
    ifstream EMeshFile(pFilePath, ios::binary);
    if (false == EMeshFile.is_open())
    {
        MSG_BOX("Failed Open : Effect_Mesh");
        return E_FAIL;
    }

    //처음에 읽는 정보는 매쉬개수, 로드파일에서 매쉬개수 먼저 읽음.
    //이펙트 매쉬는 단일 매쉬라 이 정보가 필요없어서 read로 넘겨줘야함.
    _uint MeshIndex = {};
    EMeshFile.read(reinterpret_cast<_char*>(&MeshIndex), sizeof(_uint));

    VTXMESH* pVertices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXMESH[m_iNumVertices];

    _uint* pIndices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    pIndices = new _uint[m_iNumIndices];

    //매쉬 버퍼가 텍스처 개수 알고있어야하나?
    //매쉬 이펙트에 넣어줄거 같은데 일단 데이터 내부에서 읽을려면 넘겨줘야함 나중에 고민해보고 수정하자
    _uint MaterialIndex = {};
    EMeshFile.read(reinterpret_cast<_char*>(&MaterialIndex), sizeof(_uint));

    EMeshFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
    EMeshFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    for (size_t i = 0; i < m_iNumVertices; ++i)
    {
        XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
    }

    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXMESH);

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC VBDesc = {};

    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = sizeof(VTXMESH);
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);


    //인덱스 버퍼 생성
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_iIndexStride = sizeof(_uint);

    D3D11_BUFFER_DESC IBDesc = {};

    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = sizeof(_uint);
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Initialize_Clone(void* pArg)
{
	return S_OK;
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix)
{
    CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, pFilePath)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Mesh::Clone(void* pArg)
{
    CVIBuffer_Mesh* pClone = new CVIBuffer_Mesh(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_Mesh (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Mesh::Free()
{
    __super::Free();
}
