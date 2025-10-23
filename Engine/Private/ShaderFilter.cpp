#include "EnginePch.h"
#include "ShaderFilter.h"
#include "GameInstance.h"


CShaderFilter::CShaderFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance { CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CShaderFilter::Initialize()
{
    m_iNumLUT_Textures = 5;
    m_iNumKernel = 32;

    if (FAILED(Ready_Shader_Filters()))
        CRASH("Failed Ready Shader Filters");

    if (FAILED(Ready_LUT_SRV()))
        CRASH("Failed Ready LUT_SRV");

    if (FAILED(Ready_SSAO_SampleVector()))
        CRASH("Failed Ready SampleVector");

    return S_OK;
}

HRESULT CShaderFilter::Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName)
{
    if (FAILED(m_pRampTexture->Bind_Shader_Resource(pShader, pConstantName)))
        CRASH("Failed Bind RampTexture");

    return S_OK;
}

HRESULT CShaderFilter::Bind_LUT_Texture(CShader* pShader, const _char* pConstantName, _uint iLUT_Index, const _char* pIndexConstantName)
{
    //if (iLUT_Index >= m_iNumLUT_Textures)
    //    CRASH("Failed Overflow LUT");


    pShader->Bind_Texture(pConstantName, m_pLUT_SRV);
    //if (FAILED(m_pLUT_Texture->Bind_Shader_Resource(pShader, pConstantName)))
    //    CRASH("Failed Bind LUT_Texture");

    if (iLUT_Index >= m_iNumLUT_Textures)
    {
        if (FAILED(pShader->Bind_Value(pIndexConstantName, 0, sizeof(_uint))))
            CRASH("Failed Bind LUT_Index");
    }
    else
    {
        if (FAILED(pShader->Bind_Value(pIndexConstantName, &iLUT_Index, sizeof(_uint))))
            CRASH("Failed Bind LUT_Index");
    }

    return S_OK;
}

HRESULT CShaderFilter::Bind_Noise_Texture(CShader* pShader, const _char* pConstantName)
{
    if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(pShader, pConstantName)))
        CRASH("Failed Bind RampTexture");

    return S_OK;
}

HRESULT CShaderFilter::Bind_Sample_Vector(CShader* pShader, const _char* pConstantName)
{
    if (FAILED(pShader->Bind_Value(pConstantName, m_SSAO_SampleVector.data(), sizeof(_float4) * m_iNumKernel)))
        CRASH("Failed Bind Sample Vector");

    return S_OK;
}

HRESULT CShaderFilter::Ready_Shader_Filters()
{
    m_pRampTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/T_Color_Ramp_320001.png"), 1);
    ASSERT_CRASH(m_pRampTexture);

    m_pLUT_Texture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/LUT_%d.png"), m_iNumLUT_Textures);
    ASSERT_CRASH(m_pLUT_Texture);

    m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/SSAO_Noise.png"), 1);
    ASSERT_CRASH(m_pNoiseTexture);

    return S_OK;
}

HRESULT CShaderFilter::Ready_LUT_SRV()
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

HRESULT CShaderFilter::Ready_SSAO_SampleVector()
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

CShaderFilter* CShaderFilter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CShaderFilter* pInstance = new CShaderFilter(pDevice, pContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CShaderFilter");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CShaderFilter::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pRampTexture);
    Safe_Release(m_pLUT_Texture);
    Safe_Release(m_pLUT_SRV);
    Safe_Release(m_pNoiseTexture);
}
