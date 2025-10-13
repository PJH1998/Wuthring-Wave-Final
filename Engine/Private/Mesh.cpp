#include "EnginePch.h"
#include "Mesh.h"

#include "Bone.h"
#include "Shader.h"

#include"VIBuffer_Cube.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
    : CVIBuffer { Prototype }
    , m_VertexPositions { Prototype.m_VertexPositions },
    m_Indices { Prototype.m_Indices }
    ,m_Cube{Prototype.m_Cube}
{
}

HRESULT CMesh::Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
    if (MODELTYPE::NONANIM == eType)
    {
        if (FAILED(Ready_Mesh_NonAnim(PreTransformMatrix, InputFile)))
            return E_FAIL;
    }
    else if(MODELTYPE::ANIM == eType)
    {
        if (FAILED(Ready_Mesh_Anim(Bones, PreTransformMatrix, InputFile)))
            return E_FAIL;
    }
	else if (MODELTYPE::MAP == eType)
	{
		if (FAILED(Ready_Mesh_Map(PreTransformMatrix, InputFile)))
			return E_FAIL;
	}

    return S_OK;
}

HRESULT CMesh::Initialize_Clone(void* pArg)
{
    return S_OK;
}

#ifdef _DEBUG
_bool CMesh::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
    _float fMin = FLT_MAX;
    for (size_t i = 0; i < m_Indices.size() - 2; i += 3)
    {
        _float3 vPos[3] = {
            m_VertexPositions[m_Indices[i]],
            m_VertexPositions[m_Indices[i+1]],
            m_VertexPositions[m_Indices[i+2]],
        };
        _float fDistance = {};
        if (true == TriangleTests::Intersects(vRayPos, vRayDir, 
            XMVectorSetW(XMLoadFloat3(&vPos[0]), 1.f), 
            XMVectorSetW(XMLoadFloat3(&vPos[1]), 1.f),
            XMVectorSetW(XMLoadFloat3(&vPos[2]), 1.f), fDistance))
        {
            if (fMin > fDistance)
                fMin = fDistance;
        }
    }
    if (fMin < FLT_MAX)
    {
        *pDistance = fMin;
        return true;
    }

    return false;
}
#endif

HRESULT CMesh::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
    for (size_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
    }

    return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

HRESULT CMesh::Ready_Mesh_NonAnim(_fmatrix PreTransformMatrix, ifstream& InputFile)
{
    VTXMESH* pVertices = { nullptr };
    _uint* pIndices = { nullptr };

    InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXMESH[m_iNumVertices];
    InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    pIndices = new _uint[m_iNumIndices];
    InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

    InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    for(size_t i = 0; i < m_iNumVertices; ++i)
    {
        XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
    }

    m_iVertexStride = sizeof(VTXMESH);
    m_iNumVertexBuffers = 1;

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

HRESULT CMesh::Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
    VTXANIMMESH* pVertices = { nullptr };
    _uint* pIndices = { nullptr };

    InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXANIMMESH[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    pIndices = new _uint[m_iNumIndices];
    InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));
    InputFile.read(reinterpret_cast<_char*>(&m_iNumBones), sizeof(_uint));
    for (size_t i = 0; i < m_iNumBones; ++i)
    {
        _uint iLength = {};
        // Bone Name Length
        InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
        _char szName[MAX_PATH] = {};
        // Bone Name
        InputFile.read(szName, iLength);
        auto iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
            return 0 == strcmp(szName, pBone->Get_Name());
            });

        if (iter == Bones.end())
            return E_FAIL;

        m_BoneIndices.push_back(iter - Bones.begin());

        // OffsetMatrix
        _float4x4 OffsetMatrix = {};
        InputFile.read(reinterpret_cast<_char*>(&OffsetMatrix), sizeof(_float4x4));
        XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
        m_OffsetMatrices.push_back(OffsetMatrix);
    }

    if (0 == m_iNumBones)
    {
        _float4x4 OffsetMatrix = {};
        XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
        m_OffsetMatrices.push_back(OffsetMatrix);
        m_BoneIndices.push_back(0);
    }

    InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXANIMMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    m_iVertexStride = sizeof(VTXANIMMESH);
    m_iNumVertexBuffers = 1;

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

HRESULT CMesh::Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile)
{
	VTXMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXMESH[m_iNumVertices];
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    _float3 MinPos = _float3(FLT_MAX, FLT_MAX, FLT_MAX);
    _float3 MaxPos = _float3(FLT_MIN, FLT_MIN, FLT_MIN);

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		// Mesh Shape¿ë Container
		m_VertexPositions.push_back(pVertices[i].vPosition);
        MaxPos.x = max(pVertices[i].vPosition.x, MaxPos.x);
        MaxPos.y = max(pVertices[i].vPosition.y, MaxPos.y);
        MaxPos.z = max(pVertices[i].vPosition.z, MaxPos.z);

        MinPos.x = min(pVertices[i].vPosition.x, MinPos.x);
        MinPos.y = min(pVertices[i].vPosition.y, MinPos.y);
        MinPos.z = min(pVertices[i].vPosition.z, MinPos.z);
	}

    _float3 vCorner[CORNER::END];

    vCorner[LTN] = _float3(MinPos.x, MaxPos.y, MinPos.z);
    
    vCorner[RTN] = _float3(MaxPos.x, MaxPos.y, MinPos.z);
    
    vCorner[RBN] = _float3(MaxPos.x, MinPos.y, MinPos.z);
    
    vCorner[LBN] = _float3(MinPos.x, MinPos.y, MinPos.z);
    
    vCorner[LTF] = _float3(MinPos.x, MaxPos.y, MaxPos.z);
    
    vCorner[RTF] = _float3(MaxPos.x, MaxPos.y, MaxPos.z);
    
    vCorner[RBF] = _float3(MaxPos.x, MinPos.y, MaxPos.z);
    
    vCorner[LBF] = _float3(MinPos.x, MinPos.y, MaxPos.z);

    m_Cube.Center = _float3((MinPos.x + MaxPos.x)/2.f, (MinPos.y + MaxPos.y) / 2.f, (MinPos.z + MaxPos.z) / 2.f);
    m_Cube.Extents = _float3((MaxPos.x - m_Cube.Center.x), (MaxPos.y - m_Cube.Center.y) , (MaxPos.z - m_Cube.Center.z));

	m_iVertexStride = sizeof(VTXMESH);
	m_iNumVertexBuffers = 1;

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

	// Mesh Shape¿ë Container
	for (size_t i = 0; i < m_iNumIndices; ++i)
		m_Indices.push_back(pIndices[i]);

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

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, Bones, PreTransformMatrix, InputFile)))
	{
		MSG_BOX("Failed to Create : Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh::Clone(void* pArg)
{
	CMesh* pClone = new CMesh(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mesh (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMesh::Free()
{
	__super::Free();
}
