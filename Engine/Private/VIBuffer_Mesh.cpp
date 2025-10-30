#include "EnginePch.h"
#include "VIBuffer_Mesh.h"
#include "GameInstance.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype)
    : CVIBuffer_Instance{ Prototype }
    , m_pCBBuffer{ Prototype.m_pCBBuffer }
    , m_pSRV{ Prototype.m_pSRV }
    , m_pSRVBuffer{ Prototype.m_pSRVBuffer }
{
    Safe_AddRef(m_pCBBuffer);
    Safe_AddRef(m_pSRVBuffer);
    Safe_AddRef(m_pSRV);
}

HRESULT CVIBuffer_Mesh::Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath, const INSTANCE_DESC* pDesc)
{
    const MESH_FXINSTANCE_DESC* pMeshDesc = static_cast<const MESH_FXINSTANCE_DESC*>(pDesc);

    m_iInstanceVertexStride = sizeof(VTXINSTACNE_FXMESH);     //�ϴ� �������ΰ��� �����ϴٸ�, �̰� �ᵵ ��. �ȵǸ� ����ü �߰�������.
    m_iNumInstance = pMeshDesc->iNumInstance;

    ifstream EMeshFile(pFilePath, ios::binary);
    if (false == EMeshFile.is_open())
    {
        MSG_BOX("Failed Open : Effect_Mesh");
        return E_FAIL;
    }

    //ó���� �д� ������ �Ž�����, �ε����Ͽ��� �Ž����� ���� ����.
    //����Ʈ �Ž��� ���� �Ž��� �� ������ �ʿ��� read�� �Ѱ������.
    _uint MeshIndex = {};
    EMeshFile.read(reinterpret_cast<_char*>(&MeshIndex), sizeof(_uint));

    VTXMESH* pVertices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXMESH[m_iNumVertices];

    _uint* pIndices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    m_iNumIndexPerInstance = m_iNumIndices;
    pIndices = new _uint[m_iNumIndices];

    //�Ž� ���۰� �ؽ�ó ���� �˰��־���ϳ�?
    //�Ž� ����Ʈ�� �־��ٰ� ������ �ϴ� ������ ���ο��� �������� �Ѱ������ ���߿� ����غ��� ��������
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

    m_iNumVertexBuffers = 2;
    m_iVertexStride = sizeof(VTXMESH);

    // ���ؽ� ���� ����
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


    //�ε��� ���� ����
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

    //�Ž� �ν��Ͻ��ؼ� ����غ����� �ϴ���.
    //Ŭ���� ���� �������� Desc ������ ���� (�׸����)
    m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
    m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
    m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    m_VBInstanceDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    m_VBInstanceDesc.MiscFlags = 0;
    m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride; //����ü ������

    m_pVBInstanceVertices = new VTXINSTACNE_FXMESH[m_iNumInstance];
    FXMESH_SRV* pSRV = new FXMESH_SRV[m_iNumInstance];

    if (pMeshDesc->IsSpawnBox)
    {
        for (size_t i = 0; i < m_iNumInstance; i++)
        {
            VTXINSTACNE_FXMESH* pInstanceVertices = static_cast<VTXINSTACNE_FXMESH*>(m_pVBInstanceVertices);

            _float fScale = m_pGameInstance->Rand(pMeshDesc->vSize.x, pMeshDesc->vSize.y);

            //���� �ӵ� �ٸ����Ұ���
            pSRV[i].fSpeed = m_pGameInstance->Rand(pMeshDesc->vSpeed.x, pMeshDesc->vSpeed.y);

            //�Ž� ���� �پ��ϰ� �ʿ��ҵ�.
            //��ƼŬó�� ���������� ��ѷ����� �͵� �ʿ��ϰ� , Ư�� ������ ����ָ� �� �Ž��� ���� �����Ǵ� �� ���·� ���� �Ǹ鼭 Ư�� ������ �� �� �ְ� ������Ұ� ������ ?
            //�ϴ� �������� ���߿� ���������� �߰����ְ�, ��ƼŬó���� ����°� �׽�Ʈ ���� ��������.

            //������ ���
            pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
            pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
            pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
            pInstanceVertices[i].vTranslation = _float4(
                m_pGameInstance->Rand(pMeshDesc->vCenter.x - pMeshDesc->vRange.x * 0.5f, pMeshDesc->vCenter.x + pMeshDesc->vRange.x * 0.5f),
                m_pGameInstance->Rand(pMeshDesc->vCenter.y - pMeshDesc->vRange.y * 0.5f, pMeshDesc->vCenter.y + pMeshDesc->vRange.y * 0.5f),
                m_pGameInstance->Rand(pMeshDesc->vCenter.z - pMeshDesc->vRange.z * 0.5f, pMeshDesc->vCenter.z + pMeshDesc->vRange.z * 0.5f),
                1.f
            );

            pInstanceVertices[i].vLifeTime = _float2(0.f, pMeshDesc->fLifeTime);

            pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
        }
    }
    else if (pMeshDesc->IsSpawnRing)
    {
        for (size_t i = 0; i < m_iNumInstance; i++)
        {
            VTXINSTACNE_FXMESH* pInstanceVertices = static_cast<VTXINSTACNE_FXMESH*>(m_pVBInstanceVertices);

            _float fScale = m_pGameInstance->Rand(pMeshDesc->vSize.x, pMeshDesc->vSize.y);

            //���� �ӵ� �ٸ����Ұ���
            pSRV[i].fSpeed = m_pGameInstance->Rand(pMeshDesc->vSpeed.x, pMeshDesc->vSpeed.y);

            //���� ������ ���� �ּ�, �ִ�
            _float fMin = pMeshDesc->fRmin;
            _float fMax = pMeshDesc->fRmax;

            _float fAngle = {};
            if (!pMeshDesc->IsRingAngle)
            {
                //���� ���� X,Z�� �������� ������ ���ֱ� ���� �ޱ��� 0 ~ 360���� ������ ����.
                fAngle = m_pGameInstance->Rand(0.f, XM_2PI);
            }
            else
            {
                _int Index = i;
                _float fStartRadian = XMConvertToRadians(pMeshDesc->fDegreeAngle.x);
                _float fSweepRadian = XMConvertToRadians(pMeshDesc->fDegreeAngle.y);

                fAngle = fStartRadian + ((_float)Index  / (m_iNumInstance - 1)) * fSweepRadian;
            }
            //������ �ּ�,�ִ뿡 ���� MIN~MAX�� �������� ���� �� �ְ� ���ֱ� ���� ��.
            _float fRatio = m_pGameInstance->Rand(0.f, 1.f);

            //sqrt�� �������� ������ִ� �Լ�, sqrt(4) -> 2 / ���⼭ ���� Radius�� ������ �������� ���� ����.
            _float fRadius = sqrt(fRatio * ((fMax * fMax) - (fMin * fMin)) + (fMin * fMin));

            //Angle�� ���� 0 ~ 360��, / 0�̸� cos�� 1, sin 0 / 180�̸� -1 , 0 / ��, �̰����� ���� ������ �� �Ʒ� ������ �������� ��.
            _float fPosX = fRadius * cosf(fAngle);
            _float fPosZ = fRadius * sinf(fAngle);

            pInstanceVertices[i].vTranslation = _float4(
                pMeshDesc->vCenter.x + fPosX,
                pMeshDesc->vCenter.y,                   //���Ͱ��ϴ� �����ϰ� ����, ������ �ְ������ �� �ϳ� �� �޾ƿ;���.
                pMeshDesc->vCenter.z + fPosZ,
                1.f
            );

            //������ , ȸ�� ���.
            if (pMeshDesc->IsInWard)
            {
                _vector vPos = XMVectorSet(pInstanceVertices[i].vTranslation.x, pInstanceVertices[i].vTranslation.y, pInstanceVertices[i].vTranslation.z, 0.f);
                _vector vCenter = XMVectorSet(pMeshDesc->vCenter.x, pMeshDesc->vCenter.y, pMeshDesc->vCenter.z, 0.f);

                _vector vLook = XMVector3Normalize(vPos - vCenter);

                _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

                _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
                vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight)); 

                if (pMeshDesc->fPitch != 0.f)
                {
                    _float Pitch = XMConvertToRadians(pMeshDesc->fPitch);

                    _matrix Rot = XMMatrixRotationAxis(vRight, Pitch);

                    vUp = XMVector3TransformNormal(vUp, Rot);
                    vLook = XMVector3TransformNormal(vLook, Rot);

                    vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
                    vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));
                }

                vRight = XMVector3Normalize(vRight) * fScale;
                vUp = XMVector3Normalize(vUp) * fScale;
                vLook = XMVector3Normalize(vLook) * fScale;

                XMStoreFloat4(&pInstanceVertices[i].vRight, vRight);
                XMStoreFloat4(&pInstanceVertices[i].vUp, vUp);
                XMStoreFloat4(&pInstanceVertices[i].vLook, vLook);

            }
            else 
            {
                pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
                pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
                pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
            };

            pInstanceVertices[i].vLifeTime = _float2(0.f, pMeshDesc->fLifeTime);

            pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
        }
    }

    //SRV�� ���� ����, �ν��Ͻ� ��ü�� ���� �������� ��, �Һ��� ��
    D3D11_BUFFER_DESC SRV_BufferDesc = {};
    SRV_BufferDesc.StructureByteStride = sizeof(FXMESH_SRV);
    SRV_BufferDesc.ByteWidth = SRV_BufferDesc.StructureByteStride * m_iNumInstance;
    SRV_BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;				//�Һ�
    SRV_BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		//���ҽ�
    SRV_BufferDesc.CPUAccessFlags = 0;
    SRV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA SRVInitialData{};
    SRVInitialData.pSysMem = pSRV;

    if (FAILED(m_pDevice->CreateBuffer(&SRV_BufferDesc, &SRVInitialData, &m_pSRVBuffer)))
        return E_FAIL;

    Safe_Delete_Array(pSRV);

    //SRV ���۸� ���� ���ҽ��� ����
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.FirstElement = 0;
    SRVDesc.Buffer.NumElements = SRV_BufferDesc.ByteWidth / SRV_BufferDesc.StructureByteStride;

    if (FAILED(m_pDevice->CreateShaderResourceView(m_pSRVBuffer, &SRVDesc, &m_pSRV)))
        return E_FAIL;

    //CB ���� ����, �������� ���� ���� �������ɰ�
    FXMESH_CB* pCB = new FXMESH_CB;
    pCB->vPivot = pMeshDesc->vPivot;
    pCB->fTimeDelta = 0.1f;
    pCB->IsLoop = pMeshDesc->IsLoop ? 1 : 0;
    pCB->fSpreadWeight = pMeshDesc->fSpreadWeight;
    pCB->fDropWeight = pMeshDesc->fDropWeight;
    pCB->fRotattionWeight = pMeshDesc->fRotationWeight;

    D3D11_BUFFER_DESC CB_BufferDesc = {};
    CB_BufferDesc.StructureByteStride = 0;
    CB_BufferDesc.ByteWidth = sizeof(FXMESH_CB);				 //16����Ʈ ����� ���� ����ü �ʿ�
    CB_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;					//���ֺ���
    CB_BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//�� ����
    CB_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    CB_BufferDesc.MiscFlags = 0;


    D3D11_SUBRESOURCE_DATA CBInitialData{};
    CBInitialData.pSysMem = pCB;

    if (FAILED(m_pDevice->CreateBuffer(&CB_BufferDesc, &CBInitialData, &m_pCBBuffer)))
        return E_FAIL;

    Safe_Delete(pCB);

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    D3D11_BUFFER_DESC UAV_BufferDesc = {};
    UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTACNE_FXMESH);
    UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
    UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    UAV_BufferDesc.CPUAccessFlags = 0;
    UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
    UAVInitialDesc.pSysMem = m_pVBInstanceVertices;

    if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, &UAVInitialDesc, &m_pUAVBuffer)))
        return E_FAIL;

    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    UAVDesc.Buffer.FirstElement = 0;
    UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    UAVDesc.Buffer.NumElements = UAV_BufferDesc.ByteWidth / UAV_BufferDesc.StructureByteStride;

    if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUAVBuffer, &UAVDesc, &m_pUAV)))
        return E_FAIL;

	return S_OK;
}

void CVIBuffer_Mesh::Bind_CSResources(CComputeShader* pCShader, _float fTimeDelta)
{
    D3D11_MAPPED_SUBRESOURCE	SubResource{};

    //������� ������ �� ������ ���⼭ �����������
    m_pContext->Map(m_pCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

    FXMESH_CB* pCB = static_cast<FXMESH_CB*>(SubResource.pData);

    pCB->fTimeDelta = fTimeDelta;

    m_pContext->Unmap(m_pCBBuffer, 0);
    //

    pCShader->Set_ConstantBuffer("CB", m_pCBBuffer);

    pCShader->Set_SRV("g_FXMeshStatic", m_pSRV);

    pCShader->Set_UAV("g_FXMeshState", m_pUAV);

    pCShader->Dispatch(128, 1, 1);

    //GPU���� ���� ������. ���ο��� �����۾��� �������� Ȯ���ϰ� ���� �������شٰ� ��.
    m_pContext->CopyResource(m_pVBInstance, m_pUAVBuffer);
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix, const INSTANCE_DESC* pDesc)
{
    CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, pFilePath, pDesc)))
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

    Safe_Release(m_pSRV);
    Safe_Release(m_pCBBuffer);
    Safe_Release(m_pSRVBuffer);
    Safe_Release(m_pUAVBuffer);
    Safe_Release(m_pUAV);
}
