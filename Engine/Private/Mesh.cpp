#include "EnginePch.h"
#include "Mesh.h"

#include "Bone.h"
#include "ShapeKey.h"
#include "Shader.h"


#include"VIBuffer_Cube.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
    : CVIBuffer { Prototype }
    , m_VertexPositions { Prototype.m_VertexPositions }
    , m_Indices { Prototype.m_Indices }
	, m_pMorphNormalSRV { Prototype.m_pMorphNormalSRV }
	, m_pMorphPosSRV{ Prototype.m_pMorphPosSRV }
	, m_ShapeKeys(Prototype.m_ShapeKeys)
	, m_iNumAnimMeshes { Prototype.m_iNumAnimMeshes}
	, m_pRestPoseVertices { Prototype.m_pRestPoseVertices }
{
	Safe_AddRef(m_pMorphPosSRV);
	Safe_AddRef(m_pMorphNormalSRV);

	for (auto& pKey : m_ShapeKeys)
		Safe_AddRef(pKey);
}

HRESULT CMesh::Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos)
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
		if (FAILED(Ready_Mesh_Map(PreTransformMatrix, InputFile, MinPos, MaxPos)))
			return E_FAIL;
	}
	else if (MODELTYPE::CHARACTER == eType)
	{
		if (FAILED(Ready_Mesh_Character(Bones, PreTransformMatrix, InputFile)))
			return E_FAIL;
	}

    return S_OK;
}

HRESULT CMesh::Initialize_Clone(void* pArg)
{
    return S_OK;
}


void CMesh::Update_Morph_CPU(const vector<_float>& vShapeKeyWeights)
{
	if (nullptr == m_pRestPoseVertices)
		return;

	// 1. 작업을 위해 원본(Rest Pose)을 임시 버퍼로 복사해옵니다.
	//    (매 프레임 원본에서 다시 계산해야 누적되지 않고 정확함)
	vector<VTXANIMMESH> vTempVertices(m_iNumVertices);
	memcpy(vTempVertices.data(), m_pRestPoseVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	// 2. 활성화된 쉐이프 키들을 순회하며 Delta 값을 더해줍니다.
	for (auto& pKey : m_ShapeKeys)
	{
		// 이 키의 글로벌 인덱스를 가져옴 (Model의 Weight 배열 인덱스)
		_uint iGlobalIndex = pKey->Get_GlobalWeightIndex();

		// 범위 체크
		if (iGlobalIndex >= vShapeKeyWeights.size()) continue;

		_float fWeight = vShapeKeyWeights[iGlobalIndex];

		// 가중치가 0이면 계산할 필요 없음
		if (fWeight <= 0.001f) continue;

		// --- 실제 연산 (Linear Interpolation) ---
		const vector<_float3>& DeltaPos = pKey->Get_DeltaPositions();
		const vector<_float3>& DeltaNormals = pKey->Get_DeltaNormals();
		_uint iNumKeyVerts = pKey->Get_NumVertices();

		// 정점 개수가 안 맞으면 큰일남
		if (iNumKeyVerts != m_iNumVertices) continue;

		for (_uint i = 0; i < m_iNumVertices; ++i)
		{
			// Pos += Delta * Weight
			vTempVertices[i].vPosition.x += DeltaPos[i].x * fWeight;
			vTempVertices[i].vPosition.y += DeltaPos[i].y * fWeight;
			vTempVertices[i].vPosition.z += DeltaPos[i].z * fWeight;

			// Normal += DeltaNormal * Weight (노말도 변해야 그림자가 자연스러움)
			if (!DeltaNormals.empty())
			{
				vTempVertices[i].vNormal.x += DeltaNormals[i].x * fWeight;
				vTempVertices[i].vNormal.y += DeltaNormals[i].y * fWeight;
				vTempVertices[i].vNormal.z += DeltaNormals[i].z * fWeight;
			}
		}
	}

	// 3. 노말 벡터 정규화 (옵션: 퀄리티 위해선 하는게 좋음)
	
	for (auto& Vtx : vTempVertices)
		XMStoreFloat3(&Vtx.vNormal, XMVector3Normalize(XMLoadFloat3(&Vtx.vNormal)));

	// 4. 계산이 끝난 정점들을 실제 GPU 버퍼(VB)에 덮어씌웁니다 (Map/Unmap).
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	ZeroMemory(&MappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	// DYNAMIC 버퍼는 MAP_WRITE_DISCARD를 써야 빠름 (기존 내용 버리고 새로 씀)
	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
		return;

	memcpy(MappedSubResource.pData, vTempVertices.data(), sizeof(VTXANIMMESH) * m_iNumVertices);
	m_pContext->Unmap(m_pVB, 0);
	
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

// 
HRESULT CMesh::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
    for (size_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
    }

    return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

HRESULT CMesh::Bind_MorphSRV(CShader* pShaderCom)
{
	if (FAILED(pShaderCom->Bind_SRV("g_MorphDeltaPositions", m_pMorphPosSRV)))
		return E_FAIL;

	if (FAILED(pShaderCom->Bind_SRV("g_MorphDeltaNormals", m_pMorphNormalSRV)))
		return E_FAIL;
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

HRESULT CMesh::Ready_Mesh_Character(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{

	VTXANIMMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	// 1. 기본 메쉬 정보 저장 Vertcies ~ mNumBones
	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));
	InputFile.read(reinterpret_cast<_char*>(&m_iNumBones), sizeof(_uint));

	// 2. Shape key 정보 저장 시작.
	InputFile.read(reinterpret_cast<_char*>(&m_iNumAnimMeshes), sizeof(_uint));

	// 3. Shape Key 개수만큼 순회돌기.
	for (_uint i = 0; i < m_iNumAnimMeshes; ++i)
	{
		// 1. Shape Key 이름 저장하기.
		_uint iLength = {};
		_char szName[MAX_PATH] = {};
		InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
		InputFile.read(szName, iLength);

		// 2. 길이만큼만 생성해서 문자열로 저장.
		_string strKeyName = string(szName, iLength);
		
		// 3. 정점 개수 저장.
		_uint iNumVertices = 0;
		InputFile.read(reinterpret_cast<_char*>(&iNumVertices), sizeof(_uint));

		// 정점 개수가 다르면 에러.
		if (iNumVertices != m_iNumVertices)
			CRASH("Facial Vertices Not Equal");

		// 4. Delta Position;
		vector<_float3> vecDeltaPos(iNumVertices);
		InputFile.read(reinterpret_cast<_char*>(vecDeltaPos.data()), sizeof(_float3) * iNumVertices);

		// 5. Delta Normal 
		_bool bHasNormal = { false };
		InputFile.read(reinterpret_cast<_char*>(&bHasNormal), sizeof(_bool));

		vector<_float3> vecDeltaNormal(iNumVertices, _float3(0, 0, 0)); // 값 초기화.
		if (bHasNormal) // 노말이 존재한다면?
			InputFile.read(reinterpret_cast<_char*>(vecDeltaNormal.data()), sizeof(_float3) * iNumVertices);

		// 6. CShapeKey 생성 및 등록
		CShapeKey* pShapeKey = CShapeKey::Create(m_pDevice, m_pContext, szName, iNumVertices, vecDeltaPos, vecDeltaNormal, PreTransformMatrix);

		// 7. ShapeKey를 등록.
		m_ShapeKeys.emplace_back(pShapeKey);
		//m_ShapeKeys.emplace(strKeyName, pShapeKey);
	}


	// 8. 본 내용 저장.
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

	// CPU 연산을 위해 원본 데이터를 힙 메모리에 백업.
	m_pRestPoseVertices = new VTXANIMMESH[m_iNumVertices];
	memcpy(m_pRestPoseVertices, pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	//VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.Usage = D3D11_USAGE_DYNAMIC; // 수정 가능.
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//VBDesc.CPUAccessFlags = 0;
	VBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	

	Safe_Delete_Array(pVertices);

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
#pragma endregion


	if (FAILED(Ready_MorphBuffers()))
		return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos)
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

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		// Mesh Shape??Container
		m_VertexPositions.push_back(pVertices[i].vPosition);
#ifdef _DEBUG

        MaxPos[0] = max(pVertices[i].vPosition.x, MaxPos[0]);
		MaxPos[1] = max(pVertices[i].vPosition.y, MaxPos[1]);
		MaxPos[2] = max(pVertices[i].vPosition.z, MaxPos[2]);

        MinPos[0] = min(pVertices[i].vPosition.x, MinPos[0]);
		MinPos[1] = min(pVertices[i].vPosition.y, MinPos[1]);
		MinPos[2] = min(pVertices[i].vPosition.z, MinPos[2]);
#endif
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

	// Mesh Shape??Container
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

// VtxAnimMesh에 추가할 MorphBuffer
HRESULT CMesh::Ready_MorphBuffers()
{

	if (m_ShapeKeys.empty()) return S_OK;

	_uint iTotalDataCount = m_iNumAnimMeshes * m_iNumVertices;

	// 1. Flattening (직렬화)용 임시 메모리 할당
	_float3* pFlatDeltaPos = new _float3[iTotalDataCount];
	_float3* pFlatDeltaNormal = new _float3[iTotalDataCount];

	ZeroMemory(pFlatDeltaPos, sizeof(_float3) * iTotalDataCount);
	ZeroMemory(pFlatDeltaNormal, sizeof(_float3) * iTotalDataCount);

	// 2. CShapeKey 순회하며 데이터 복사
	for (_uint i = 0; i < m_ShapeKeys.size(); ++i)
	{
		const vector<_float3>& vecPos = m_ShapeKeys[i]->Get_DeltaPositions();
		const vector<_float3>& vecNormal = m_ShapeKeys[i]->Get_DeltaNormals();

		_uint iOffset = i * m_iNumVertices;

		memcpy(&pFlatDeltaPos[iOffset], vecPos.data(), sizeof(_float3) * m_iNumVertices);
	
		// Normal 데이터가 존재한다면 복사
		if (!vecNormal.empty() && vecNormal.size() == m_iNumVertices)
			memcpy(&pFlatDeltaNormal[iOffset], vecNormal.data(), sizeof(_float3) * m_iNumVertices);
		
	}

	// 3. D3D11 Buffer & SRV 생성 (이전과 동일)
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	BufferDesc.ByteWidth = sizeof(_float3) * iTotalDataCount;
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(_float3);

	D3D11_SUBRESOURCE_DATA InitData;

	// (1) Position Buffer
	InitData.pSysMem = pFlatDeltaPos;
	ID3D11Buffer* pPosBuffer = nullptr;
	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &pPosBuffer))) return E_FAIL;

	// Position SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = iTotalDataCount;

	if (FAILED(m_pDevice->CreateShaderResourceView(pPosBuffer, &SRVDesc, &m_pMorphPosSRV))) return E_FAIL;
	Safe_Release(pPosBuffer);

	// (2) Normal Buffer
	InitData.pSysMem = pFlatDeltaNormal;
	ID3D11Buffer* pNormalBuffer = nullptr;
	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &pNormalBuffer))) return E_FAIL;

	// Normal SRV
	if (FAILED(m_pDevice->CreateShaderResourceView(pNormalBuffer, &SRVDesc, &m_pMorphNormalSRV))) return E_FAIL;
	Safe_Release(pNormalBuffer);

	// 4. 임시 메모리 해제
	Safe_Delete_Array(pFlatDeltaPos);
	Safe_Delete_Array(pFlatDeltaNormal);

	return S_OK;
}




CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, Bones, PreTransformMatrix, InputFile,MinPos,MaxPos)))
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


	// 1. 힙에 백업해둔 원본 정점 데이터 삭제
	if (!m_isClone)
		Safe_Delete_Array(m_pRestPoseVertices);

	// 2. 가지고 있는 ShapeKey 객체들 해제.
	Safe_Release(m_pMorphPosSRV);
	Safe_Release(m_pMorphNormalSRV);

	for (auto& pShapeKey : m_ShapeKeys)
		Safe_Release(pShapeKey);

	m_ShapeKeys.clear();
}
