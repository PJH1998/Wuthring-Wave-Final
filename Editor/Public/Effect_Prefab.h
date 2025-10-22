#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)

class CEffect_Prefab : public CGameObject
{
public:
	typedef struct PrefabDesc {
		_wstring strPrefabTag; //?섏쨷?????쒓렇濡??留곹깭洹?吏?뺥븷嫄곗엫
		_int	ChildrenCount; //?먯떇 ??
		//?먯떇???뺣낫 ?꾩슂
	}PREFAB_DESC;

private:
	CEffect_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Prefab(const CEffect_Prefab& Prototype);
	virtual ~CEffect_Prefab() = default;

public:
	virtual HRESULT Initialize_Prototype()override;
	virtual HRESULT Initialize_Clone(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Render() override;

public:
	void Add_Children(void* pArg, EFFECT_TYPE eType);
	void Remove_Children(_wstring& ChildrenTag);

public:
	_int Get_Children_Count();
	_wstring Get_Children_Tag(_int iIndex);
	CGameObject* Get_Children(_wstring ChildrenTag);
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

private:
	_wstring							m_strMyTag;	 //???꾩떆??
	
	//?먯떇??
	map<const _wstring, CGameObject*>	 m_EffectChildren; 
	
	//?먯떇???ъ깮愿由??대뼸寃??좎? 議곌툑 ??援ъ긽?대킄??醫뗭쓣??
	//map<const _wstring, float>				m_ChildrenDesc; 

public:
	static CEffect_Prefab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
