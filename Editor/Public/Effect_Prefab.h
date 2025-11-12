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
		_bool	IsRoot = false;

		_float3 vOffsetSize = { 1.f, 1.f, 1.f };
		_float3 vOffsetPos = { 0.f, 0.f, 0.f };
		_float3 vOffsetRot = { 0.f, 0.f, 0.f };
	}FRAME_DESC;

	typedef struct PrefabDesc {
		_wstring strPrefabTag;
		_int	ChildrenCount;
		vector<FRAME_DESC> FrameDesc;

		_float2	vLifeTime = { 0.f, 0.f };
		_string strBoneTag;
		_bool	IsRoot = false;
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
	//void Root_Test();

public:
	_int Get_Children_Count();
	_wstring Get_Children_Tag(_int iIndex);

	CGameObject* Get_Children(_wstring ChildrenTag);
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

public:
	void Bind_FrameDesc(PREFAB_DESC& PrefabDesc);
	void Set_FrameDesc(FRAME_DESC* pFrameDesc);
	void Set_SpawnMatrix(_float4x4 SpawnMatrix);
	void Reset_SpawnMatrix();

public:
	void Reset_Prefab_Info();			//툴에서도 소환해줘야해서 일단 Public

private:
	void Children_Offset(const FRAME_DESC& Desc, _matrix& OutMatrix);
	void FrameDesc_Check(_wstring& ChildrenTag, _bool* bCheck);

private:

	_wstring							 m_strMyTag;	 
	
	//이펙트 소환했을 때 그 시점 뼈 위치기준 행렬 세팅 한 번만 해주기. Reset
	_float4x4							 m_SpawnMatrix = {};

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
