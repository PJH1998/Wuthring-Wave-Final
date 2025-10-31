#include "EnginePch.h"
#include "RendererSubResource.h"
#include "GameInstance.h"


CRendererSubResource::CRendererSubResource(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance { CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CRendererSubResource::Initialize()
{
    m_iNumLUT_Textures = 4;

    //SSAO
    m_iNumKernel = 16;
    m_fRadius = 1.f;
    m_fMaxDistance = 5.f;
    m_fOutDistance = 500.f;
    
    // SSAO_Blur
    m_fSSAO_MinDepthDistance = 5.f; 

    m_vFogDepthDistance = _float2(1000.f, 5000.f);
    m_vFogHeightDistance = _float2(0.f, 100.f);

    m_vFogColor = _float4(1.f, 1.f, 1.f, 1.f);

    m_iNumWeights = 5;
    m_fIntensity = 0.25f;

    m_fDofDepth = 50.f;
    m_fDofRange = 100.f;
    m_fDofScale = 0.3f;

    if (FAILED(Ready_Shader_Filters()))
        CRASH("Failed Ready Shader Filters");

    if (FAILED(Ready_LUT_SRV()))
        CRASH("Failed Ready LUT_SRV");

    if (FAILED(Ready_SSAO_SampleVector()))
        CRASH("Failed Ready SampleVector");

    if (FAILED(Ready_CS_Sampler()))
        CRASH("Failed Ready Sampler");

    if (FAILED(Ready_BlurWeights()))
        CRASH("Failed Ready BlurWeights");

    return S_OK;
}

HRESULT CRendererSubResource::Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
    if (FAILED(m_pRampTexture->Bind_Shader_Resource(pShader, pConstantName, iTextureIndex)))
        CRASH("Failed Bind RampTexture");

    return S_OK;
}

HRESULT CRendererSubResource::Bind_LUT_Texture(CShader* pShader, _uint iLUT_Index)
{
    if(FAILED(pShader->Bind_Texture("g_LUT_Texture", m_pLUT_SRV)))
        CRASH("Failed Bind LUT_Texture");

    _uint iIndex = iLUT_Index >= m_iNumLUT_Textures ? 0 : iLUT_Index;

    if (FAILED(pShader->Bind_Value("g_iLutIndex", &iIndex, sizeof(_uint))))
        CRASH("Failed Bind LUT_Index");

    return S_OK;
}
HRESULT CRendererSubResource::Bind_SSAO_Resources(CShader* pShader)
{
    if (FAILED(pShader->Bind_Matrix("g_CamViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Render Fail");

    if (FAILED(pShader->Bind_Matrix("g_CamProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Render Fail");

    if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(pShader, "g_NoiseTexture")))
        CRASH("Failed Bind NoiseTexture");

    if (FAILED(pShader->Bind_Value("g_vSampleVector", m_SSAO_SampleVector.data(), sizeof(_float4) * m_iNumKernel)))
        CRASH("Failed Bind Sample Vector");

    if (FAILED(pShader->Bind_Value("g_iSampleSize", &m_iNumKernel, sizeof(_uint))))
        CRASH("Failed Bind g_iSampleSize");

    if (FAILED(pShader->Bind_Value("g_fSSAO_Radius", &m_fRadius, sizeof(_float))))
        CRASH("Failed Bind g_fSSAO_Radius");

    if (FAILED(pShader->Bind_Value("g_fSSAO_MaxDistance", &m_fMaxDistance, sizeof(_float))))
        CRASH("Failed Bind g_fSSAO_MaxDistance");

    if (FAILED(pShader->Bind_Value("g_fSSAO_OutDistance", &m_fOutDistance, sizeof(_float))))
        CRASH("Failed Bind g_fSSAO_OutDistance");
    
    return S_OK;
}

HRESULT CRendererSubResource::Bind_Fog_Resources(CShader* pShader)
{
    if (FAILED(m_pFogNoiseTexture->Bind_Shader_Resource(pShader, "g_FogNoiseTexture")))
        CRASH("Failed Bind FogNoiseTexture");

    if (FAILED(pShader->Bind_Value("g_vFogDepthDistance", &m_vFogDepthDistance, sizeof(_float2))))
        CRASH("Failed Bind Fog Distance");

    if (FAILED(pShader->Bind_Value("g_vFogHeightDistance", &m_vFogHeightDistance, sizeof(_float2))))
        CRASH("Failed Bind Fog Distance");

    if (FAILED(pShader->Bind_Value("g_vFogColor", &m_vFogColor, sizeof(_float4))))
        CRASH("Failed Bind Fog Color");

    m_fFogTime = fmodf(m_fFogTime + 1.f, 1920.f); // ������ ������ 128 x 128

    if (FAILED(pShader->Bind_Value("g_fFogTime", &m_fFogTime, sizeof(_float))))
        CRASH("Failed Bind Fog Time");

    return S_OK;
}

HRESULT CRendererSubResource::Bind_Dof_Resource(CShader* pShader)
{
    if (FAILED(pShader->Bind_Value("g_fFocusDepth", &m_fDofDepth, sizeof(_float))))
        CRASH("Failed Bind Fog Distance");
    if (FAILED(pShader->Bind_Value("g_fFocusRange", &m_fDofRange, sizeof(_float))))
        CRASH("Failed Bind Fog Distance");
    if (FAILED(pShader->Bind_Value("g_fFocusMinCoc", &m_fDofScale, sizeof(_float))))
        CRASH("Failed Bind Fog Distance");

    return S_OK;
}

HRESULT CRendererSubResource::Add_SSAO_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight)
{
    SSAO_BLUR_DATA Data = {};
    Data.fSSAO_MinDepthDistance = m_fSSAO_MinDepthDistance;
    Data.fWidth = fWidth;
    Data.fHeight = fHeight;

    if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "SSAO_BLUR_DATA", reinterpret_cast<void*>( &Data ), sizeof(SSAO_BLUR_DATA))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRendererSubResource::Add_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iBlurWeight)
{
	if (iBlurWeight >= m_iNumWeights)
		return E_FAIL;

	BLUR_DATA Data = {};
	Data.vSize = _float2(fWidth, fHeight);
	Data.iRadius = m_Weights[iBlurWeight].first;

	if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "BLUR_DATA", reinterpret_cast<void*>(&Data), sizeof(BLUR_DATA))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_SRVData(strRCSTag, "g_Weights", m_WeightSRVs[iBlurWeight])))
		return E_FAIL;

	return S_OK;
}

HRESULT CRendererSubResource::Add_Bloom_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iUpIndex)
{
    BLOOM_UP_DATA Data = {};
    Data.vSize = _float2(fWidth, fHeight);
    Data.fIntensity = ( 1.f - static_cast<_float>( ( iUpIndex + 1 ) ) * m_fIntensity );

    if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "BLOOM_DATA", reinterpret_cast<void*>( &Data ), sizeof(BLOOM_UP_DATA))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRendererSubResource::Ready_Shader_Filters()
{
    m_pRampTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/Color_Ramp%d.png"), 3);
    ASSERT_CRASH(m_pRampTexture);

    m_pLUT_Texture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/LUT_%d.png"), m_iNumLUT_Textures);
    ASSERT_CRASH(m_pLUT_Texture);

    m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/SSAO_Noise.png"), 1);
    ASSERT_CRASH(m_pNoiseTexture);
    
    m_pFogNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/T_PerlinNoise.png"), 1);

    return S_OK;
}

HRESULT CRendererSubResource::Ready_LUT_SRV()
{
    ID3D11Texture2D* pSourceTexture = { nullptr };
    m_pLUT_Texture->Get_SRV(0)->GetResource(reinterpret_cast<ID3D11Resource**>(&pSourceTexture));

    D3D11_TEXTURE2D_DESC OriginDesc = {};
    pSourceTexture->GetDesc(&OriginDesc);
    
    Safe_Release(pSourceTexture);

    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = OriginDesc.Width;
    TextureDesc.Height = OriginDesc.Height;
    TextureDesc.MipLevels = OriginDesc.MipLevels;
    TextureDesc.ArraySize = m_iNumLUT_Textures;

    TextureDesc.Format = OriginDesc.Format;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;

    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;			// DSV, SRV
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    ID3D11Texture2D* pTexture2D = { nullptr };
    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture2D)))
        CRASH("Shadow Texture");
    
    for (_uint i = 0; i < m_iNumLUT_Textures; ++i)
    {
        ID3D11Texture2D* pCopyTexture = { nullptr };
        m_pLUT_Texture->Get_SRV(i)->GetResource(reinterpret_cast<ID3D11Resource**>( &pCopyTexture));

        UINT DestSubresource = D3D11CalcSubresource(0, i, TextureDesc.MipLevels);

        m_pContext->CopySubresourceRegion(pTexture2D, DestSubresource, 0, 0, 0, pCopyTexture, 0, nullptr);

        Safe_Release(pCopyTexture);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = OriginDesc.Format;
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    SrvDesc.Texture2DArray.MostDetailedMip = 0;
    SrvDesc.Texture2DArray.MipLevels = 1;
    SrvDesc.Texture2DArray.FirstArraySlice = 0;
    SrvDesc.Texture2DArray.ArraySize = m_iNumLUT_Textures;

    if (FAILED(m_pDevice->CreateShaderResourceView(pTexture2D, &SrvDesc, &m_pLUT_SRV)))
        CRASH("LUT SRV");

    Safe_Release(pTexture2D);

    return S_OK;
}

HRESULT CRendererSubResource::Ready_SSAO_SampleVector()
{
    for (_uint i = 0; i < m_iNumKernel; i++)
    {
        _float fX = m_pGameInstance->Rand(-1.f, 1.f);
        _float fY = m_pGameInstance->Rand(-1.f, 1.f);
        _float fZ = m_pGameInstance->Rand_Normal();

        _vector vSample = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));
        
        _float fScale = static_cast<_float>( i ) / static_cast<_float>( m_iNumKernel );
        fScale = 0.1f + ( 0.9f * pow(fScale,2) );

        m_SSAO_SampleVector.push_back(XMVectorScale(vSample, fScale));
    }

    return S_OK;
}

HRESULT CRendererSubResource::Ready_CS_Sampler()
{
    D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
    DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    DefaultSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    DefaultSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    DefaultSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    DefaultSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    DefaultSamplerDesc.MinLOD = 0;
    DefaultSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_pDevice->CreateSamplerState(&DefaultSamplerDesc, &m_pDefaultSampler);
    ASSERT_CRASH(m_pDefaultSampler);

    D3D11_SAMPLER_DESC NoiseSamplerDesc = {};
    NoiseSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    NoiseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    NoiseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    NoiseSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    NoiseSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    NoiseSamplerDesc.MinLOD = 0;
    NoiseSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_pDevice->CreateSamplerState(&NoiseSamplerDesc, &m_pNoiseSampler);
    ASSERT_CRASH(m_pNoiseSampler);

    D3D11_SAMPLER_DESC PointClampDesc = {};
    PointClampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    PointClampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    PointClampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    PointClampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    PointClampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    PointClampDesc.MinLOD = 0;
    PointClampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_pDevice->CreateSamplerState(&PointClampDesc, &m_pPointClampSampler);
    ASSERT_CRASH(m_pPointClampSampler);

    return S_OK;
}

HRESULT CRendererSubResource::Ready_BlurWeights()
{
    m_Weights.resize(m_iNumWeights);

    for (_uint i = 1; i <= m_iNumWeights; ++i)
    {
        _int iRadius = i * 3;

        _float fSum = 0.f;

        for (_int j = -iRadius; j <= iRadius; ++j)
        {
            _float fWeight = expf(-( j * j ) / ( 2.f * i * i ));
            m_Weights[i - 1].second.push_back(fWeight);
            fSum += fWeight;
        }

        for (auto& Weight : m_Weights[i - 1].second)
            Weight /= fSum;

        m_Weights[i - 1].first = iRadius;
        Create_BlurBuffer(m_Weights[i - 1].second, iRadius);
    }

    return S_OK;
}

HRESULT CRendererSubResource::Create_BlurBuffer(const vector<_float>& Weights, _uint iRadius)
{
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = sizeof(_float) * (iRadius * 2 + 1);
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.StructureByteStride = sizeof(_float);
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA Data = {};
    Data.pSysMem = Weights.data();

    ID3D11Buffer* pBuffer = { nullptr };
    m_pDevice->CreateBuffer(&BufferDesc, &Data, &pBuffer);
    ASSERT_CRASH(pBuffer);

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.Buffer.FirstElement = 0;
    SRVDesc.Buffer.NumElements = iRadius * 2 + 1;

    ID3D11ShaderResourceView* pSRV = { nullptr };
    m_pDevice->CreateShaderResourceView(pBuffer, &SRVDesc, &pSRV);
    ASSERT_CRASH(pSRV);

    m_WeightBuffers.push_back(pBuffer);
    m_WeightSRVs.push_back(pSRV);

    return S_OK;
}

CRendererSubResource* CRendererSubResource::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRendererSubResource* pInstance = new CRendererSubResource(pDevice, pContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CRendererSubResource");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRendererSubResource::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    Safe_Release(m_pRampTexture);
    Safe_Release(m_pLUT_Texture);
    Safe_Release(m_pLUT_SRV);
    Safe_Release(m_pNoiseTexture);
    Safe_Release(m_pFogNoiseTexture);
    Safe_Release(m_pHighCloudTexture);

    Safe_Release(m_pDefaultSampler);
    Safe_Release(m_pPointClampSampler);
    Safe_Release(m_pNoiseSampler);

    for (auto& pBuffer : m_WeightBuffers)
        Safe_Release(pBuffer);
    m_WeightBuffers.clear();

    for (auto& pSRV : m_WeightSRVs)
        Safe_Release(pSRV);
    m_WeightSRVs.clear();
}
