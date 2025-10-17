#pragma once
#include "Base.h"
#include "Effect_Prefab.h"

NS_BEGIN(Editor)

//프리팹 관리 해주고자 함
//프리팹 Desc 추출
//(프리팹 이름, 자식 정보, 자식들의 재생타임 정보)

class CEffect_Controller :public CBase
{
private:
	explicit CEffect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEffect_Controller() = default;

#pragma region 기본
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

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };
	class CParticle_Controller*									m_pParticle_Controller = { nullptr };

	//InputText에서 받을 프리팹 태그
	_char														m_PrefabTag[MAX_PATH];
	_bool														m_bTagFlag = false;

	//InputText에서 받을 자식 태그
	_char														m_ChildrenTag[MAX_PATH];
	_bool														m_bChildrenTagFlag = false;
	_bool														m_bChildrenCreatFlag = false;
	EFFECT_TYPE													m_eChildrenType = EFFECT_TYPE::END;

	//툴에서 현재 선택한 프리팹 정보
	_int														m_iSelectedPrefab = 0;
	_bool														m_bSelectedPrefab = false;
	class CEffect_Prefab*										m_pSelectedPrefab = { nullptr };

	//툴에서 현재 선택한 프리팹의 자식 정보
	_int														m_iSelectedChildren = 0;
	_wstring													m_strChildrenTag = {};
	_bool														m_IsParticle = false;
	_bool														m_IsMeshEffect = false;
	_bool														m_IsTrailMesh = false;

	map<const _wstring, class CEffect_Prefab*>					m_Prefabs = {};
	map<const _wstring, CEffect_Prefab::PREFAB_DESC>			m_PrefabDesc = {};

	// 프리팹 정보 어떻게 뽑아서 만들건지 좀 고민해봐야할거같음.
	// 프리팹이 자식들의 정보를 알아야함. 즉, 클론할 때 Desc가 필요함.
	// 버퍼는 외부에서 Desc를 통해 원형 생성해놓고 (프리팹 만들기 전에 먼저해놔야함)
	// 이후 프리팹 클론 하면서 자식들 생성 할 때 파티클, 매쉬 등의 Desc가 필요

public:
	static CEffect_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END