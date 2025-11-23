#pragma once
#include "UI_Image.h"

NS_BEGIN(Client)
class CUI_MobHPBar final : public CUI_Image
{
public:
	//typedef struct tUIMobHPBarDesc {	// 나중에 Transform 은 아닌, 뼈 위치 등 기준으로 변경. matrix, float3 등
	//	// hp와 같은 정보들을 벡터로 받아와야하나? 어떤 정보를 어떻게 받아오지
	//
	//} UI_MOBHP_DESC;

	typedef struct tUIMobRTDesc {
		UI_MOBINFO_DESC		tInfoDesc = {};

		_float		fCurElapsedTime = 0.f;
		_float		fCurStackedTime = 0.f;

		array<_float, 2>	fAtkedElapsedTime = {};
		array<_bool, 2>		isTimerActived = {};
		
		_bool		isUpdatedThisFrame = false;
		_bool		isPendingFlick = false;
	}UI_MOBRT_DESC;

private:
	enum HP_COLOR { HPC_GREEN, HPC_YELLOW, HPC_RED, HRC_BACKGROUND, HPC_END};

public:
	explicit CUI_MobHPBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_MobHPBar(const CUI_MobHPBar& Prototype);
	virtual ~CUI_MobHPBar() = default;

public:
	virtual HRESULT Initialize_Prototype()							override;	
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

public:
	void			Update_MobStatus(const UI_MOBINFO_DESC& tDesc);

private:
	//void			Ready_Presets();
	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

	void			Update_CachedData(_float fTimeDelta);
	void			Update_Instances();

	void			Calc_ApplyTargetPos(CCustom_UI* pTargetUI);
	void			Calc_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance);
	void			Calc_HBEff();

private:
	//CCustom_UI*		m_pUI_HPFrame		= nullptr;
	//CCustom_UI*		m_pUI_HPBar			= nullptr;
	//CCustom_UI*		m_pUI_SAFrame		= nullptr;
	//CCustom_UI*		m_pUI_SABar			= nullptr;
	CCustom_UI*			m_pUI_HB			= nullptr;
	CCustom_UI*			m_pUI_HB_Inv		= nullptr;
	CCustom_UI*			m_pUI_Line			= nullptr;
	CCustom_UI*			m_pUI_Frame			= nullptr;

	vector<UI_MOBINFO_DESC>		m_vecMobInfo = {};			// 프레임에서 받아온 정보들이 담긴 컨테이너
	vector<UI_MOBRT_DESC>		m_vecMobInfo_RT = {};		// 실질적인 몬스터 정보를 들고 있는 컨테이너

	// 0.	이전 프레임의 정보를 담은 컨테이너의 모든 isUpdatedThisFrame 을 false 로 돌림
	// 1.	일단 정보들을 받아옴
	// 2.	모두 받아온 시점에서, 이전 프레임에 받아온 정보들과 objectid를 비교하여 
	//		몹들이 살아있는지, 새로 생성된 몹이 있는지 등의 여부를 확인하고,
	//		isUpdatedThisFrame 값을 true로 바꿈
	// 3.	true가 된 애들만 생존 시간 업데이트, 아닌 애들은 죽었다 치고 제거


	class CGameSystem*			m_pGameSystem = { nullptr };

private:
	// HP 주기 효과용
	const _float			m_fMaxRTTime = 4.f;
	const _float			m_fMinRTTime = 0.5f;

	const _float			m_fMaxAtkedTimer = 0.5f;

	array<_float4, 4>		m_arrColorPresets = {};


public:
	static CUI_MobHPBar*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

NS_END