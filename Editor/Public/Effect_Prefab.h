#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)

class CEffect_Prefab : public CGameObject
{
public:
	typedef struct Effect_Frame {
		_wstring strChildrenTag;
		EFFECT_TYPE eChildrenType;
		_float	fActivateTime;
		_bool   bActivated = false;
	}FRAME_DESC;

	typedef struct PrefabDesc {
		_wstring strPrefabTag;
		_int	ChildrenCount;
		vector<FRAME_DESC> FrameDesc;

		_float2	vLifeTime = { 0.f, 0.f };
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
	void Root_Test();

public:
	_int Get_Children_Count();
	_wstring Get_Children_Tag(_int iIndex);

	CGameObject* Get_Children(_wstring ChildrenTag);
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

public:
	void Set_FrameDesc(FRAME_DESC* pFrameDesc);

public:
	void Reset_Prefab_Info();			//툴에서도 소환해줘야해서 일단 Public

private:

	_wstring							 m_strMyTag;	 
	

	const _float4x4*					 m_pRootMatirx = {};

	_float								 m_fCurrentTime = 0.f;
	_float2								 m_vLifeTime = {};


	map<const _wstring, CGameObject*>	 m_EffectChildren; 
	vector<FRAME_DESC>					 m_vFrames;
	//map<const _wstring, float>				m_ChildrenDesc; 

public:
	static CEffect_Prefab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
