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
    m_iNumKernel = 16;

    m_fRadius = 10.f;
    m_fMaxDistance = 50.f;

    m_fSSAO_MinDepthDistance = 10.f;

    m_vFogDepthDistance = _float2(1000.f, 5000.f);
    m_vFogHeightDistance = _float2(0.f, 100.f);

    m_vFogColor = _float4(1.f, 1.f, 1.f, 1.f);

    if (FAILED(Ready_Shader_Filters()))
        CRASH("Failed Ready Shader Filters");

    if (FAILED(Ready_LUT_SRV()))
        CRASH("Failed Ready LUT_SRV");

    if (FAILED(Ready_SSAO_SampleVector()))
        CRASH("Failed Ready SampleVector");

    if (FAILED(Ready_CS_Sampler()))
        CRASH("Failed Ready Sampler");
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
    //if (FAILED(m_pLUT_Texture->Bind_Shader_Resource(pShader, "g_LUT_Texture")))
    //    CRASH("Failed Bind LUT_Texture");

    _uint iIndex = iLUT_Index >= m_iNumLUT_Textures ? 0 : iLUT_Index;

    if (FAILED(pShader->Bind_Value("g_iLutIndex", &iIndex, sizeof(_uint))))
        CRASH("Failed Bind LUT_Index");

    return S_OK;
}
//
//HRESULT CRendererSubResource::Bind_Noise_Texture(CShader* pShader, const _char* pConstantName)
//{
//    if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(pShader, pConstantName)))
//        CRASH("Failed Bind RampTexture");
//
//    return S_OK;
//}
//
//HRESULT CRendererSubResource::Bind_Sample_Vector(CShader* pShader, const _char* pConstantName)
//{
//    if (FAILED(pShader->Bind_Value(pConstantName, m_SSAO_SampleVector.data(), sizeof(_float4) * m_iNumKernel)))
//        CRASH("Failed Bind Sample Vector");
//
//    return S_OK;
//}

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

    m_fFogTime += 1.f;

    if (FAILED(pShader->Bind_Value("g_fFogTime", &m_fFogTime, sizeof(_float))))
        CRASH("Failed Bind Fog Time");

    return S_OK;
}

HRESULT CRendererSubResource::Add_SSAO_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight)
{
    SSAO_DATA Data = {};
    memcpy(Data.vSampleVector, m_SSAO_SampleVector.data(), sizeof(_float4) * m_iNumKernel);
    Data.CamViewMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);
    Data.CamProjMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);
    Data.ProjMatrixInv = *m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ);
    Data.fWidth = fWidth;
    Data.fHeight = fHeight;
    Data.iSampleSize = m_iNumKernel;
    Data.fSSAO_Radius = m_fRadius;
    Data.fSSAO_MaxDistance = m_fMaxDistance;

    if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "SSAO_DATA", reinterpret_cast<void*>(&Data), sizeof(SSAO_DATA))))
        return E_FAIL;

    m_pDefaultSampler->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);

    m_pContext->CSSetSamplers(1, 1, &m_pDefaultSampler);
    m_pContext->CSSetSamplers(2, 1, &m_pPointClampSampler);
    m_pContext->CSSetSamplers(3, 1, &m_pNoiseSampler);


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

HRESULT CRendererSubResource::Add_SizeData_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight)
{
    SIZE_DATA Data = {};
    Data.vSize = _float2(fWidth, fHeight);
    
    if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "SIZE_DATA", reinterpret_cast<void*>( &Data ), sizeof(SIZE_DATA))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRendererSubResource::Add_SSAO_NoiseTexture(const _wstring& strRCSTag, const _char* pConstantName)
{
    if (FAILED(m_pGameInstance->Add_SRVData(strRCSTag, pConstantName, m_pNoiseTexture->Get_SRV(0))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRendererSubResource::Ready_Shader_Filters()
{
    m_pRampTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/Color_Ramp%d.png"), 2);
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
}
