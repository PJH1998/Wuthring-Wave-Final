#pragma once
#include "Component.h"


NS_BEGIN(Engine)
class ENGINE_DLL CComputeShader final : public CComponent
{
private:
    explicit CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    explicit CComputeShader(CComputeShader& Prototype);
    virtual ~CComputeShader() = default;

#pragma region 기본 함수들
public:
    // .hlsl 파일을 로드하고 .cso 파일을 만든 뒤 리플렉션 정보를 파싱합니다.
    virtual HRESULT Initialize_Prototype(const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, const _string& strEntryPoint);
    virtual HRESULT Initialize_Clone(void* pArg);
#pragma endregion

public:
    // numThreads 컴파일 정보를 가져옵니다.
    const COMPUTESHADER_INFO& Get_ThreadInfo() const { return m_ThreadInfo; }

public:
    // 이름으로 리소스를 바인딩합니다.
    void Set_SRV(const string& strName, ID3D11ShaderResourceView* pSRV);
    void Set_UAV(const string& strName, ID3D11UnorderedAccessView* pUAV);
    void Set_ConstantBuffer(const string& strName, ID3D11Buffer* pCB);

    // 계산 셰이더를 실행합니다.
    void Dispatch(_uint iThreadGroupCountX, _uint iThreadGroupCountY, _uint iThreadGroupCountZ);


private:
    // 셰이더 로딩 시 리소스 정보를 미리 분석합니다.
    HRESULT Ready_Reflection(ID3DBlob* pCSBlob);

    // 사용이 끝난 리소스를 정리합니다.
    void Clear_Resources();

private:
    ID3D11ComputeShader* m_pComputeShader = nullptr;

    // 실제 사용하기 위해서 numThreads 정보를 저장합니다.
    COMPUTESHADER_INFO m_ThreadInfo = {};

    // 리플렉션으로 얻어온 리소스 정보 (이름 -> 바인딩 슬롯 번호)
    map<string, _uint>          m_SRV_BindPoints;
    map<string, _uint>          m_UAV_BindPoints;
    map<string, _uint>          m_CB_BindPoints;

    // Dispatch 직전에 실제 바인딩할 리소스 목록 (슬롯 번호 -> 리소스 포인터)
    map<_uint, ID3D11ShaderResourceView*>    m_SRVs_To_Bind; // Dispatch 할때 바인딩 됩니다.
    map<_uint, ID3D11UnorderedAccessView*>   m_UAVs_To_Bind; // Dispatch 할때 바인딩 됩니다.
    map<_uint, ID3D11Buffer*>                m_CBs_To_Bind;  // Dispatch 할때 바인딩 됩니다.

public:
    static CComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, _string strEntryPoint);
    virtual		CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END
