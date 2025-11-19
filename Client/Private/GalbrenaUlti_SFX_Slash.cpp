#include "ClientPch.h"
#include "GalbrenaUlti_SFX_Slash.h"

CGalbrenaUlti_SFX_Slash::CGalbrenaUlti_SFX_Slash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect { pDevice, pContext }
{
}

CGalbrenaUlti_SFX_Slash::CGalbrenaUlti_SFX_Slash(const CGalbrenaUlti_SFX_Slash& Prototype)
	: CScreenEffect { Prototype }
	, m_vPivot{ Prototype.m_vPivot }
	, m_vDrawRadians { Prototype.m_vDrawRadians }
	, m_vScale { Prototype.m_vScale }
	, m_vColor { Prototype.m_vColor }
	, m_vUpdateTime{ Prototype.m_vUpdateTime }
{
	for (_uint i = 0; i < 5; ++i)
		m_SlashData[i] = Prototype.m_SlashData[i];
}

HRESULT CGalbrenaUlti_SFX_Slash::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vPivot = _float2(m_vWinSize.x * 0.7f, m_vWinSize.y * 0.75f);

	m_vEffectTime = _float2(0.f, 0.5f);
	m_vUpdateTime = _float2(0.f, 0.5f);

	m_vDrawRadians = _float2(XMConvertToRadians(25.f), XMConvertToRadians(15.f));
	m_vScale = _float2(1.f, 0.1f);
	m_vColor = _float3(1.f, 1.f, 1.f);

	Ready_SlashData();

	return S_OK;
}

HRESULT CGalbrenaUlti_SFX_Slash::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Update_Instance();

	m_isActivate = false;

	return S_OK;
}

void CGalbrenaUlti_SFX_Slash::Priority_Update(_float fTimeDelta)
{
}

void CGalbrenaUlti_SFX_Slash::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	if(m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		return;
	}

	_float fRatio = SmoothStep(m_vUpdateTime.x, m_vUpdateTime.y, m_fCurrentTime);

	m_fCurrentRadian = lerp(m_vDrawRadians.x, m_vDrawRadians.y, fRatio);
	m_fCurrentScale = lerp(m_vScale.x, m_vScale.y, fRatio);
}

void CGalbrenaUlti_SFX_Slash::Late_Update(_float fTimeDelta)
{
	Update_Instance();

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CGalbrenaUlti_SFX_Slash::Render()
{
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fMaxRadian", &m_fCurrentRadian, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(ENUM_CLASS(SHADER_SFX_BURST_INSTANCE::GALBRENA_SLASH));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrenaUlti_SFX_Slash::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;
	m_fCurrentRadian = m_vDrawRadians.x;
	m_fCurrentScale = m_vScale.x;

	m_pVIBuffer->Clear();
}

HRESULT CGalbrenaUlti_SFX_Slash::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect_Instance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

void CGalbrenaUlti_SFX_Slash::Ready_SlashData()
{
	m_SlashData[0].vSize = _float2(400.f, 1150.f);
	m_SlashData[0].fRotateRadian = XMConvertToRadians(35.f);

	m_SlashData[1].vSize = _float2(2000.f, 1500.f);
	m_SlashData[1].fRotateRadian = XMConvertToRadians(115.f);

	m_SlashData[2].vSize = _float2(300.f, 700.f);
	m_SlashData[2].fRotateRadian = XMConvertToRadians(135.f);

	m_SlashData[3].vSize = _float2(700.f, 750.f);
	m_SlashData[3].fRotateRadian = XMConvertToRadians(250.f);

	m_SlashData[4].vSize = _float2(400.f, 1000.f);
	m_SlashData[4].fRotateRadian = XMConvertToRadians(315.f);
}

void CGalbrenaUlti_SFX_Slash::Update_Instance()
{
	vector<VTXINSTANCE_RECT> Datas;

	for (_uint i = 0; i < 5; ++i)
		Datas.push_back(Make_Instance(m_vPivot, m_SlashData[i].vSize.x * m_fCurrentScale, m_SlashData[i].vSize.y, m_SlashData[i].fRotateRadian));

	m_pVIBuffer->Update_Buffer(Datas);
}

VTXINSTANCE_RECT CGalbrenaUlti_SFX_Slash::Make_Instance(_float2 vPivot, _float fSizeX, _float fSizeY, _float fRotateZ)
{
	VTXINSTANCE_RECT Rect = {};

	_vector vRotation = XMQuaternionRotationRollPitchYaw(0.f, 0.f, fRotateZ);		// Rotate
	_vector vAnchor = XMVectorSet(0.f, -0.5f * fSizeY, 0.f, 1.f);				// Center Bottom ( Rect Pivot )

	vAnchor = XMVector3Rotate(vAnchor, vRotation);

	Setting_Scale(fSizeX, fSizeY);
	m_pTransformCom->Rotation_Quaternion(vRotation);
	Setting_Pos(vPivot.x - XMVectorGetX(vAnchor), vPivot.y + XMVectorGetY(vAnchor));

	memcpy(&Rect.vRight, m_pTransformCom->Get_WorldMatrixPtr(), sizeof(_matrix));

	return Rect;
}

CGalbrenaUlti_SFX_Slash* CGalbrenaUlti_SFX_Slash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrenaUlti_SFX_Slash* pInstance = new CGalbrenaUlti_SFX_Slash(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrenaUlti_SFX_Slash");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrenaUlti_SFX_Slash::Clone(void* pArg)
{
	CGalbrenaUlti_SFX_Slash* pInstance = new CGalbrenaUlti_SFX_Slash(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrenaUlti_SFX_Slash");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrenaUlti_SFX_Slash::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
}
