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

		_float3 vOffsetSize = { 1.f, 1.f, 1.f };
		_float3 vOffsetPos = { 0.f, 0.f, 0.f };
		_float3 vOffsetRot = { 0.f, 0.f, 0.f };
	}FRAME_DESC;

	typedef struct PrefabDesc {
		_wstring strPrefabTag;
		_int	ChildrenCount;
		vector<FRAME_DESC> FrameDesc;

		_bool	IsLoop = false;
		_float2	vLifeTime = { 0.f, 15.f };
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
	void Children_Offset(const FRAME_DESC& Desc, _matrix& OutMatrix, EFFECT_INFO& Info);

	void Check_CameraDistance();
	void UpdateActiveFromFlag(_float fTimeDelta);

public:
	_wstring	Get_MyTag() {
		return m_strMyTag;
	};

public:
	void Set_SpawnMatrix(_float4x4 PlayerMatrix, _float4x4 BoneMatrix);
	void Reset_SpawnMatrix();

private:
	void Reset_Prefab_Info();		
	void Deactivate_AllChildren();

private:

	_wstring							 m_strMyTag;	 
	_string								 m_strBoneTag;
	
	//이펙트 소환했을 때 그 시점 뼈 위치기준 행렬 세팅 한 번만 해줄때 사용할 정보.
	_float4x4							 m_SpawnMatrix = {};

	//이펙트 소환했을 때 뼈에 붙여줄때 사용할 정보.
	const _float4x4*					 m_pBoneMatrixPtr = nullptr;
	const _float4x4*					 m_pObjectMatrixPtr = nullptr;
	_bool*								 m_pActiveFlag = nullptr;

	_float								 m_fCurrentTime = 0.f;
	_float2								 m_vLifeTime = {};
	_bool								 m_IsLoop = false;
	_bool								 m_IsLoopActive = false;

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
