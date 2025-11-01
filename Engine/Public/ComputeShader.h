#pragma once
#include "Component.h"


NS_BEGIN(Engine)
class ENGINE_DLL CComputeShader final : public CComponent
{
private:
    explicit CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    explicit CComputeShader(CComputeShader& Prototype);
    virtual ~CComputeShader() = default;

#pragma region 湲곕낯 ?⑥닔??
public:
    // .hlsl ?뚯씪??濡쒕뱶?섍퀬 .cso ?뚯씪??留뚮뱺 ??由ы뵆?됱뀡 ?뺣낫瑜??뚯떛?⑸땲??
    virtual HRESULT Initialize_Prototype(const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, const _string& strEntryPoint);
    virtual HRESULT Initialize_Clone(void* pArg);
#pragma endregion

public:
    // numThreads 而댄뙆???뺣낫瑜?媛?몄샃?덈떎.
    const COMPUTESHADER_INFO& Get_ThreadInfo() const { return m_ThreadInfo; }

public:
    // ?대쫫?쇰줈 由ъ냼?ㅻ? 諛붿씤?⑺빀?덈떎.
    void Set_SRV(const string& strName, ID3D11ShaderResourceView* pSRV);
    void Set_UAV(const string& strName, ID3D11UnorderedAccessView* pUAV);
    void Set_ConstantBuffer(const string& strName, ID3D11Buffer* pCB);
	void Set_Sampler(_uint iSlotIndex, ID3D11SamplerState* pSampler);
    // 
    void Dispatch(_uint iThreadGroupCountX, _uint iThreadGroupCountY, _uint iThreadGroupCountZ);


private:
    HRESULT Ready_Reflection(ID3DBlob* pCSBlob);
    void Clear_Resources();

private:
    ID3D11ComputeShader* m_pComputeShader = nullptr;

    // 
    COMPUTESHADER_INFO m_ThreadInfo = {};

    // 
    map<string, _uint>          m_SRV_BindPoints;
    map<string, _uint>          m_UAV_BindPoints;
    map<string, _uint>          m_CB_BindPoints;

    // 
    map<_uint, ID3D11ShaderResourceView*>   m_SRVs_To_Bind;
    map<_uint, ID3D11UnorderedAccessView*>  m_UAVs_To_Bind;
    map<_uint, ID3D11Buffer*>               m_CBs_To_Bind; 
	map<_uint, ID3D11SamplerState*>			m_SAMPLERs_To_Bind;
public:
    static CComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, _string strEntryPoint);
    virtual		CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END
