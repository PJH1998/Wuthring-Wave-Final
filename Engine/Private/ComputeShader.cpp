#include "EnginePch.h"
#include "ComputeShader.h"

CComputeShader::CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{pDevice, pContext}
{
}

// To_Bind 맵들은 Dispatch를 호출할 때마다 채워지고 비워짐
CComputeShader::CComputeShader(CComputeShader& Prototype)
    : CComponent(Prototype)
    , m_pComputeShader { Prototype.m_pComputeShader }
    , m_SRV_BindPoints{ Prototype.m_SRV_BindPoints }
    , m_UAV_BindPoints{ Prototype.m_UAV_BindPoints }
    , m_CB_BindPoints{ Prototype.m_CB_BindPoints }
{
    // 공유하는 셰이더 객체의 참조 카운트를 늘려준다.
    Safe_AddRef(m_pComputeShader);

    //for (auto& pair : m_SRVs_To_Bind)
    //    Safe_AddRef(pair.second);
    //
    //for (auto& pair : m_UAVs_To_Bind)
    //    Safe_AddRef(pair.second);
    //
    //for (auto& pair : m_CBs_To_Bind)
    //    Safe_AddRef(pair.second);
}

// 셰이더 만듭니다.
HRESULT CComputeShader::Initialize_Prototype(const _tchar* pFilePath)
{
    // HLSL 파일로 생성.
    _uint iHlslFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    // 디버그 빌드에서는 디버깅 정보를 포함하고 최적화를 건너뜁니다.
    iHlslFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    // 릴리즈 빌드에서는 최적화를 켭니다.
    iHlslFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

    ID3DBlob* pCSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    // 1. D3DCompileFromFile을 사용해 HLSL 파일을 직접 컴파일합니다.
    HRESULT hr = D3DCompileFromFile(pFilePath, // 예: L"../Shader/GPUSkinning.hlsl"
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",     // 진입점 함수 이름
        "cs_5_0",   // 셰이더 타겟 프로파일
        iHlslFlag, 0,
        &pCSBlob,   // 컴파일 성공 시 바이트코드가 담길 Blob
        &pErrorBlob // 컴파일 실패 시 에러 메시지가 담길 Blob
    );

    // 2. 컴파일 실패 시 에러 메시지 출력 (매우 중요!) 
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            // 에러 메시지를 char*로 변환하여 MessageBox나 로그로 출력합니다.
            const char* errorMessage = (char*)pErrorBlob->GetBufferPointer();
            MessageBoxA(nullptr, errorMessage, "HLSL Compile Error", MB_OK);
            Safe_Release(pErrorBlob);
        }
        return E_FAIL;
    }

    // 3. 셰이더 객체를 생성합니다. => Blob을 hlsl로 컴파일해서 Compute Shader 객체를 만드는데 사용
    if (FAILED(m_pDevice->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &m_pComputeShader)))
    {
        Safe_Release(pCSBlob);
        return E_FAIL;
    }

    // 4. Blob을 이용해서  셰이더 리플렉션으로 리소스 정보를 분석. => Blob을 hlsl로 컴파일해서 Compute Shader 객체를 만드는데 사용
    if (FAILED(Ready_Reflection(pCSBlob)))
    {
        MSG_BOX("Create Compute Shader File Read Failed");
        Safe_Release(pCSBlob);
        return E_FAIL;
    }

    // 1. 컴파일된 셰이더(.cso) 파일을 읽어옵니다. => 이거는 필수 hlsl이 아님.
    //if (FAILED(D3DReadFileToBlob(pFilePath, &pCSBlob)))
    //{
    //    MSG_BOX("Create Compute Shader File Read Failed");
    //    return E_FAIL;
    //}

    // 2. 셰이더 객체를 생성합니다.
    //if (FAILED(m_pDevice->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &m_pComputeShader)))
    //{
    //    MSG_BOX("Create Compute Shader File Read Failed");
    //    Safe_Release(pCSBlob);
    //    return E_FAIL;
    //}

    // 3. 셰이더 리플렉션으로 리소스 정보를 분석.
    //if (FAILED(Ready_Reflection(pCSBlob)))
    //{
    //    MSG_BOX("Create Compute Shader File Read Failed");
    //    Safe_Release(pCSBlob);
    //    return E_FAIL;
    //}

    Safe_Release(pCSBlob);

    return S_OK;
}

HRESULT CComputeShader::Initialize_Clone(void* pArg)
{
    return S_OK;
}

// 여기다가 그래픽카드가 읽어야할 데이터를 던져줌.
void CComputeShader::Set_SRV(const string& strName, ID3D11ShaderResourceView* pSRV)
{
    auto iter = m_SRV_BindPoints.find(strName);
    if (iter != m_SRV_BindPoints.end())
        m_SRVs_To_Bind[iter->second] = pSRV;
}

// 여기다가 그래픽카드가 출력할 데이터를 담을 포인터를 던져줌.
void CComputeShader::Set_UAV(const string& strName, ID3D11UnorderedAccessView* pUAV)
{
    auto iter = m_UAV_BindPoints.find(strName);
    if (iter != m_UAV_BindPoints.end())
        m_UAVs_To_Bind[iter->second] = pUAV;
}

// 상수 버퍼 (아직 명확히 이해안됨)
void CComputeShader::Set_ConstantBuffer(const string& strName, ID3D11Buffer* pCB)
{
    auto iter = m_CB_BindPoints.find(strName);
    if (iter != m_CB_BindPoints.end())
        m_CBs_To_Bind[iter->second] = pCB;
}

// 그래픽 카드 스레드 할당해서 작업 진행 
// x, y, z 는 ex) 8, 1, 1 이면 => 8 * 1 * 1
// x, y, z 는 ex) 1, 8, 1, 이면 => 1 * 8 * 1
// x, y, z 는 ex) 8, 8, 1, 이면 => 8 * 8 * 1 로 할당 (총 스레드 개수
// x, y, z 역할은 지금 내가 명확하게 설명이 안되서. => 좀 더 쳐보면서 공부를 
void CComputeShader::Dispatch(_uint iThreadGroupCountX, _uint iThreadGroupCountY, _uint iThreadGroupCountZ)
{
    // 1. 셰이더 설정
    m_pContext->CSSetShader(m_pComputeShader, nullptr, 0);

    // 2. 등록된 리소스 바인딩
    for (auto& Pair : m_SRVs_To_Bind)
        m_pContext->CSSetShaderResources(Pair.first, 1, &Pair.second);
    for (auto& Pair : m_UAVs_To_Bind)
        m_pContext->CSSetUnorderedAccessViews(Pair.first, 1, &Pair.second, nullptr);
    for (auto& Pair : m_CBs_To_Bind)
        m_pContext->CSSetConstantBuffers(Pair.first, 1, &Pair.second);

    // 3. 실행
    m_pContext->Dispatch(iThreadGroupCountX, iThreadGroupCountY, iThreadGroupCountZ);

    // 4. 리소스 정리 (매우 중요!)
    Clear_Resources();
}

/*
* 
*/
HRESULT CComputeShader::Ready_Reflection(ID3DBlob* pCSBlob)
{
    ID3D11ShaderReflection* pReflection = nullptr;
    if (FAILED(D3DReflect(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflection)))
        return E_FAIL;

    D3D11_SHADER_DESC shaderDesc;
    pReflection->GetDesc(&shaderDesc);

    for (_uint i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC bindDesc;
        pReflection->GetResourceBindingDesc(i, &bindDesc);

        string strName = bindDesc.Name;

        switch (bindDesc.Type)
        {
        case D3D_SIT_CBUFFER:
            m_CB_BindPoints[strName] = bindDesc.BindPoint;
            break;
        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
        case D3D_SIT_STRUCTURED:
            m_SRV_BindPoints[strName] = bindDesc.BindPoint;
            break;
        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
            m_UAV_BindPoints[strName] = bindDesc.BindPoint;
            break;
        }
    }

    Safe_Release(pReflection);
    return S_OK;
}

// 다음 Dispatch를 위해 바인딩된 리소스 정보 초기화
void CComputeShader::Clear_Resources()
{
    // UAV 바인딩 해제 (다른 셰이더가 이 리소스를 SRV로 읽을 수 있게 함)
    for (auto& Pair : m_UAVs_To_Bind)
    {
        ID3D11UnorderedAccessView* pNullUAV = nullptr;
        m_pContext->CSSetUnorderedAccessViews(Pair.first, 1, &pNullUAV, nullptr);
    }

    m_SRVs_To_Bind.clear();
    m_UAVs_To_Bind.clear();
    m_CBs_To_Bind.clear();
}

CComputeShader* CComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath)
{
    CComputeShader* pInstance = new CComputeShader(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype(pFilePath)))
    {
        MSG_BOX("Failed to Create : CComputeShader");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CComputeShader::Clone(void* pArg)
{
    CComputeShader* pClone = new CComputeShader(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Clone : ComputeShader");
        Safe_Release(pClone);
    }

    return pClone;
}

void CComputeShader::Free()
{
    CComponent::Free();

    for (auto& pair : m_SRVs_To_Bind)
        Safe_Release(pair.second);

    m_SRVs_To_Bind.clear();

    for (auto& pair : m_UAVs_To_Bind)
        Safe_Release(pair.second);

    m_UAVs_To_Bind.clear();

    for (auto& pair : m_CBs_To_Bind)
        Safe_Release(pair.second);

    m_CBs_To_Bind.clear();

    Safe_Release(m_pComputeShader);
}
