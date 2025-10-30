#pragma once
#include "Base.h"
#include "Effect_Prefab.h"

NS_BEGIN(Editor)



class CEffect_Controller :public CBase
{
private:
	typedef struct tagAnimationActorDesc
	{
		class CModel* pModelCom = { nullptr };
		class CAnimationActor* pAnimActor = { nullptr };
		_string strAnimName = {};
		float fDuration = {};
	}ANIMACTOR_DSEC;

private:
	explicit CEffect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEffect_Controller() = default;

#pragma region 湲곕낯
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion
public:
	void Prefab_Tab();

	void UpdateSelected_PrefabFromIndex();
	void UpdateSelected_ChildrenFromIndex();

	void Reset_TabInfo();


	void Import_AnimationData(const EFFECTACTOR_DESC& effectActorDesc);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };
	class CParticle_Controller*									m_pParticle_Controller = { nullptr };
	class CMesh_Controller*										m_pMesh_Controller = { nullptr };

	//InputText?먯꽌 諛쏆쓣 ?꾨━???쒓렇
	_char														m_PrefabTag[MAX_PATH];
	_bool														m_bTagFlag = false;

	//InputText?먯꽌 諛쏆쓣 ?먯떇 ?쒓렇
	_char														m_ChildrenTag[MAX_PATH];
	_bool														m_bChildrenTagFlag = false;
	_bool														m_bChildrenCreatFlag = false;
	EFFECT_TYPE													m_eChildrenType = EFFECT_TYPE::END;

	//?댁뿉???꾩옱 ?좏깮???꾨━???뺣낫
	_int														m_iSelectedPrefab = 0;
	_bool														m_bSelectedPrefab = false;
	class CEffect_Prefab*										m_pSelectedPrefab = { nullptr };

	//?댁뿉???꾩옱 ?좏깮???꾨━?뱀쓽 ?먯떇 ?뺣낫
	_int														m_iSelectedChildren = 0;
	_wstring													m_strChildrenTag = {};
	_bool														m_IsParticle = false;
	_bool														m_IsMeshEffect = false;
	_bool														m_IsTrailMesh = false;

	ANIMACTOR_DSEC												m_AnimActorDesc = {};
	

	map<const _wstring, class CEffect_Prefab*>					m_Prefabs = {};
	map<const _wstring, CEffect_Prefab::PREFAB_DESC>			m_PrefabDesc = {};

	// ?꾨━???뺣낫 ?대뼸寃?戮묒븘??留뚮뱾嫄댁? 醫 怨좊??대킄?쇳븷嫄곌컳??
	// ?꾨━?뱀씠 ?먯떇?ㅼ쓽 ?뺣낫瑜??뚯븘?쇳븿. 利? ?대줎????Desc媛 ?꾩슂??
	// 踰꾪띁???몃??먯꽌 Desc瑜??듯빐 ?먰삎 ?앹꽦?대넃怨?(?꾨━??留뚮뱾湲??꾩뿉 癒쇱??대넄?쇳븿)
	// ?댄썑 ?꾨━???대줎 ?섎㈃???먯떇???앹꽦 ?????뚰떚?? 留ㅼ돩 ?깆쓽 Desc媛 ?꾩슂

public:
	static CEffect_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END