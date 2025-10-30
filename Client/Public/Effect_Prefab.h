#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

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
		_string strBoneTag;
		_uint	CurrentLevel;
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
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	CGameObject* Get_Children(_wstring ChildrenTag);
	void Add_Children(const _wstring& ChildrenTag, EFFECT_TYPE eType, _uint CurrentLevel);

public:
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

public:
	void Set_SpawnMatrix(_float4x4 PlayerMatrix, _float4x4 BoneMatrix);
	void Reset_SpawnMatrix();

public:
	void Reset_Prefab_Info();			//툴에서도 소환해줘야해서 일단 Public

private:

	_wstring							 m_strMyTag;	 
	_string								 m_strBoneTag;
	
	//이펙트 소환했을 때 그 시점 뼈 위치기준 행렬 세팅 한 번만 해주기. 
	_float4x4							 m_SpawnMatrix = {};

	_float								 m_fCurrentTime = 0.f;
	_float2								 m_vLifeTime = {};

	//자식들 주소
	map<const _wstring, CGameObject*>	 m_EffectChildren; 

	//자식들정보
	vector<FRAME_DESC>					 m_vFrames;

public:
	static CEffect_Prefab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
