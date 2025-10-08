#include"EnginePch.h"
#include "Mesh_Instance.h"

CMesh_Instance::CMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CVIBuffer_Instance(pDevice,pContext)
{
}

CMesh_Instance::CMesh_Instance(const CMesh_Instance& Prototype)
    :CVIBuffer_Instance(Prototype)
{
}

HRESULT CMesh_Instance::Initialize_Prototype(_fmatrix PreTransformMatrix, ifstream& InputFile)
{
    VTXMESH* pVertices = { nullptr };
    _uint* pIndices = { nullptr };

    m_iVertexStride = sizeof(VTXMESH);
    m_iNumVertexBuffers = 2;

    InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXMESH[m_iNumVertices];
    InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    pIndices = new _uint[m_iNumIndices];
    InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

    InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    for (size_t i = 0; i < m_iNumVertices; ++i)
    {
        XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
#ifdef _DEBUG
        m_VertexPositions.push_back(pVertices[i].vPosition);
#endif
    }

    D3D11_BUFFER_DESC   VBDesc = {};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);
#pragma endregion

#ifdef _DEBUG
    for (size_t i = 0; i < m_iNumIndices; ++i)
        m_Indices.push_back(pIndices[i]);
#endif

#pragma region INDEX
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    
    D3D11_BUFFER_DESC IBDesc = {};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);
    return S_OK;
}

HRESULT CMesh_Instance::Initialize_Clone(void* pArg)
{
    MESH_INST_DESC* pDesc = static_cast<MESH_INST_DESC*>(pArg);
    
    m_iNumInstance = pDesc->iNumInstance;
    
    m_iInstanceVertexStride = sizeof(VTXINSTANCE_MESH);
    m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
    m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
    m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_VBInstanceDesc.MiscFlags = 0;
    m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

    m_pVBInstanceVertices = new VTXINSTANCE_MESH[m_iNumInstance];
    VTXINSTANCE_MESH* pVBInstanceVertices = static_cast<VTXINSTANCE_MESH*>(m_pVBInstanceVertices);

    for (_uint i = 0; i < m_iNumInstance; ++i)
    {
        memcpy(&pVBInstanceVertices->vRight, &pDesc->pTransformMatrix[i].m[0], sizeof(_float4));
        memcpy(&pVBInstanceVertices->vUp, &pDesc->pTransformMatrix[i].m[1], sizeof(_float4));
        memcpy(&pVBInstanceVertices->vLook, &pDesc->pTransformMatrix[i].m[2], sizeof(_float4));
        memcpy(&pVBInstanceVertices->vTranslation, &pDesc->pTransformMatrix[i].m[3], sizeof(_float4));
    }

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    return S_OK;
}

//HRESULT CMesh_Instance::Render()
//{
//    return S_OK;
//}
//
//HRESULT CMesh_Instance::Bind_Resources()
//{
//    return S_OK;
//}

CMesh_Instance* CMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
    CMesh_Instance* pInstance = new CMesh_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, InputFile)))
    {
        MSG_BOX("Failed to Create : Mesh_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CMesh_Instance::Clone(void* pArg)
{
    CMesh_Instance* pClone = new CMesh_Instance(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : Mesh_Instance (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}
void CMesh_Instance::Free()
{
    __super::Free();
}
