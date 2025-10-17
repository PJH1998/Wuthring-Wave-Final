#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)

class CEffect_Prefab : public CGameObject
{
public:
	typedef struct PrefabDesc {
		_wstring strPrefabTag; //나중에 이 태그로 풀링태그 지정할거임
		_int	ChildrenCount; //자식 수
		//자식들 정보 필요
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
	void Add_Children(void* pArg);
	void Remove_Children(_wstring& ChildrenTag);

public:
	_int Get_Children_Count();
	_wstring Get_Children_Tag(_int iIndex);
	CGameObject* Get_Children(_wstring ChildrenTag);
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

private:
	_wstring							m_strMyTag;	 //툴 임시용
	
	//자식들
	map<const _wstring, CGameObject*>	 m_EffectChildren; 
	
	//자식들 재생관리 어떻게 할지 조금 더 구상해봐도 좋을듯
	//map<const _wstring, float>				m_ChildrenDesc; 

public:
	static CEffect_Prefab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
