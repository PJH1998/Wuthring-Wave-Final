#include "ClientPch.h"
#include "UI_Text_Damage.h"

CUI_Text_Damage::CUI_Text_Damage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Text(pDevice, pContext)
{
}

CUI_Text_Damage::CUI_Text_Damage(const CUI_Text_Damage& Prototype)
	: CUI_Text(Prototype)
{
}

HRESULT CUI_Text_Damage::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Text_Damage::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	
	TEXT_UI_TIMED_DESC* pDesc = static_cast<TEXT_UI_TIMED_DESC*>(pArg);
	
	m_isActivate = false;

	m_fLifeTime = pDesc->vLifeTime.y;
	m_iNumText = pDesc->strText.size();
	m_fLifeElapsed = pDesc->vLifeTime.x;
	m_isAutoDeactivate = true;

	return S_OK;
}

void CUI_Text_Damage::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CUI_Text_Damage::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;		 

	Update_LifeTime(fTimeDelta);
	// __super::Update(fTimeDelta);
	__super::Update_Description(fTimeDelta);

	Update_Instances(fTimeDelta);
	CCustom_UI::Update(fTimeDelta);
}

void CUI_Text_Damage::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Late_Update(fTimeDelta);
}

void CUI_Text_Damage::Render()
{
	if (!m_isActivate)
		return;

	__super::Render();
}

void CUI_Text_Damage::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// wake up via pooling. like as initialize
	// initilize 처럼 초기값 갱신에 사용하되, 중복 생성될 여지는 피해야 함.

	__super::Bind_Description(pArg);

	m_fLifeElapsed = 0.f;
	m_fLifeTime = static_cast<TEXT_UI_DESC*>(pArg)->vLifeTime.y;
	m_tTextDesc = *static_cast<TEXT_UI_DESC*>(pArg);
	m_isActivate = true;

 	for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++) // 종종 터짐.
		m_tUIDesc.vecInstanceDescs[i].matExtraData.m[0][0] = 1.f;
}

void CUI_Text_Damage::Update_LifeTime(_float fTimeDelta)
{
	if (m_fLifeTime <= 0.f)
		return;

	m_fLifeElapsed += max(0.f, fTimeDelta);

	if (m_isAutoDeactivate && (m_fLifeElapsed >= m_fLifeTime))
	{
		m_isActivate = false;
		// if needs trigger when its deactive, declare here.
		static _uint iDmgIndex = 0;
		iDmgIndex++;
		std::cout << "[CUI_Text_Damage::Update_LifeTime] Damage Destroyed! : " << iDmgIndex << std::endl;
	}
}

void CUI_Text_Damage::Update_Instances(_float fTimeDelta)
{
	auto& textUIDesc = Get_TextUIDesc();
	auto& vecInstDescs = textUIDesc.vecInstanceDescs;
	_float fStartScale = 2.5f;

	// ksta : 각종 인스턴스 갱신용 정보들 (위치용 행렬, 커스텀 변수용 행렬 등등)
	//		  꺼내와서 가공하고 다시 재할당해주는 식으로 사용하면 됨

	// 6~10프레임 (약 0.1~0.17초) 중으로 인스턴스 하나의 시작 애니메이션이 끝나야 함 
	// 10프레임 (약 0.17초) 중으로 데미지가 전부 사라져야 함
	// 45프레임 (0.75초) 간만 lifetime 주어짐


	_uint iNumInst = vecInstDescs.size();			// 이만큼 루프돌면서 업데이트 필요

	// 이 내에서 인스턴스 변화

	const _float	fStartAnimTime = 0.17f;				// 인스턴스 별 페이드인에 소요되는 시간
	const _float	fFadeOutTime	= 0.1f;				// 사라지기 n초 전부터 페이드아웃
	const _float	fInstIntervalTime = 0.05f;			// 인스턴스 별 페이드인 간격
	
	_float			fLifeTime		= m_fLifeTime; 
	_float			fLifeElapsed	= m_fLifeElapsed;
	
	for (_uint i = 0; i < iNumInst; i++)
	{
		auto& curDesc = vecInstDescs[i];

		_float fAlpha = 1.f;
		_float2 vScale = _float2{ curDesc.vSInstRight.x, curDesc.vSInstUp.y };
		_float2 vPos = _float2{ curDesc.vSInstTrans.x, curDesc.vSInstTrans.y };

		if (curDesc.matExtraData.m[0][3] != 1.0f)
		{
			// 최초 1회, 원본 크기 및 '원본 위치'를 저장
			curDesc.matExtraData.m[0][1] = curDesc.vSInstRight.x;  // s0x (원본 너비)
			curDesc.matExtraData.m[0][2] = curDesc.vSInstUp.y;    // s0y (원본 높이)
			curDesc.matExtraData.m[1][1] = curDesc.vSInstTrans.x; // p0x (원본 위치 X)
			curDesc.matExtraData.m[1][2] = curDesc.vSInstTrans.y; // p0y (원본 위치 Y)
			curDesc.matExtraData.m[0][3] = 1.0f;
		}
		// 매 프레임 '원본' 값들을 읽어옴
		const _float s0x = curDesc.matExtraData.m[0][1];
		const _float s0y = curDesc.matExtraData.m[0][2];
		const _float p0x = curDesc.matExtraData.m[1][1]; // 원본 위치 X
		const _float p0y = curDesc.matExtraData.m[1][2]; // 원본 위치 Y

		const _float tLocal = fLifeElapsed - (fInstIntervalTime * (_float)i);
		// 기본값
		_float alpha = 1.0f; // (기본값 = 안 보임)
		_float sx = s0x * fStartScale;
		_float sy = s0y * fStartScale;

		// 4) 시작 후 스타트 애니메이션: fStartAnimTime 동안 a: 1→0 (페이드인), 스케일: fStartScale→1x (선형)
		if (tLocal >= 0.0f)
		{
			if (tLocal < fStartAnimTime)
			{
				const _float u = tLocal / fStartAnimTime;
				alpha = 1.0f - u;
				
                // [FIXED] 스케일 로직 수정
                // k가 fStartScale에서 1.0으로 선형 보간(Lerp) 되도록 수정
				const _float k = (fStartScale * (1.0f - u)) + (1.0f * u);
				
                sx = s0x * k;
				sy = s0y * k;
			}
			else
			{	// 스타트 이후 (머무르는 상태)
				alpha = 0.0f; // (보임)
				sx = s0x;
				sy = s0y;
			}
		}

		// 5) 객체 전체 페이드아웃: 남은 fFadeOutTime 동안 a: 0→1 (페이드아웃)
		{
			const _float fadeBegin = fLifeTime - fFadeOutTime;
			if (fLifeElapsed >= fadeBegin)
			{
				_float u_fade = (fLifeElapsed - fadeBegin) / fFadeOutTime; // 0..1
				if (u_fade < 0.f) u_fade = 0.f; else if (u_fade > 1.f) u_fade = 1.f;

                // (0.0 -> 1.0 으로 Lerp: 보임 -> 안 보임)
				alpha = alpha * (1.0f - u_fade) + 1.0f * u_fade;
			}
		}

		// 5-1) 변화된 scale에 맞춰 pos 적용
		const _float deltaWidth = sx - s0x;
		const _float deltaHeight = sy - s0y;

		// 최종 위치 = 원본 위치 - (크기 변화량의 절반)
		_float finalPosX = p0x - (deltaWidth / 2.0f);
		_float finalPosY = p0y - (deltaHeight / 2.0f);

		// 6) 적용
		fAlpha = (alpha < 0.f ? 0.f : (alpha > 1.f ? 1.f : alpha));
		vScale.x = sx;
		vScale.y = sy;

		curDesc.matExtraData.m[0][0] = fAlpha;
		curDesc.vSInstRight.x = vScale.x;
		curDesc.vSInstUp.y = vScale.y;
		curDesc.vSInstTrans.x = finalPosX;
		curDesc.vSInstTrans.y = finalPosY;

		//if (i == 0)
		//	cout << vScale.x << ", " << vScale.y << endl;
	}
	m_tUIDesc.vecInstanceDescs = vecInstDescs;		// 인스턴스 반영

	// =======================================================================
}

CUI_Text_Damage* CUI_Text_Damage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Text_Damage* pInstance = new CUI_Text_Damage(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Text_Damage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Text_Damage::Clone(void* pArg)
{
	CUI_Text_Damage* pInstance = new CUI_Text_Damage(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Text_Damage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Text_Damage::Free()
{
	__super::Free();
}
