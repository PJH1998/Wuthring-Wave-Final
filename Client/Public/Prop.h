#pragma once
#include "PartObject.h"

NS_BEGIN(Client)
class CProp abstract : public CPartObject
{
public:
	typedef struct tagPropDesc : public CPartObject::PART_DESC 
	{
		LEVEL eLevel = { LEVEL::END };
		const _float4x4* pSocketMatrix = { nullptr };
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> computeShaderData = {};
		pair<LEVEL, _wstring> modelData = {};
		pair<LEVEL, _wstring> rigidBodyData = {};
		_string strBoneName = {};
		_string strFolderPath = {};
		WEAPONTYPE eWeaponType = { WEAPONTYPE::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};
	} PROP_DESC;

protected:
	explicit CProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CProp(const CPartObject& Prototype);
	virtual ~CProp() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;

public:
	virtual void Activate(_bool IsActive);
	virtual void Prop_Reset();  // 상태를 초기화합니다.
	virtual void Change_Volume(_uint iVolumeIdx) {};
	virtual void Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer) {};
	virtual void Volume_Activate(_bool IsActive);

	void Set_Visible(_bool IsVisible) { m_IsVisible = IsVisible; }
	_bool Is_Visible() { return m_IsVisible; }
public:
	_bool Is_AnimationEnd() { return m_IsAnimationEnd; }
	virtual void Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false); 
	void Set_SocketMatrix(const _float4x4* pSocketMatrix) { m_pSocketMatrix = pSocketMatrix; }
	void Clear_Animation(const _string& strAnimName);

#pragma region NOTIFY
public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) ;
	virtual void Effect_Active(const _wstring& wStrEffectTag);
	virtual void Object_Func(const _wstring& wStrEffectTag) {}
	void		Collider_Active(_bool isActive);
#pragma endregion

#pragma region CONDITION
public:
	void Add_Condition(_uint iConditionFlag);
	_bool Check_AnyCondition(_uint iConditionFlag);
	_bool Check_AllCondition(_uint iConditionFlag);
	void Remove_Condition(_uint iConditionFlag);
	void Remove_AllCondition();

	void Bind_DissolveTimer();
#pragma endregion


protected:
	LEVEL m_eCurLevel = { LEVEL::END };
	WEAPONTYPE m_eWeaponType = { WEAPONTYPE::END };
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	//class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CTransform* m_pParentTransform = { nullptr };
	class CAttackVolume* m_pMainAttackVolume = { nullptr };
	class CGameObject* m_pOwner = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };
	_float m_fTrackPosition = {};
	_string m_strCurrentAnimName = {};
	_bool m_IsAnimationEnd = { false };
	
	_uint m_iVolumeIdx = {};
	_uint m_iShaderPath = {};
	vector<class CAttackVolume*> m_AttackVolumes;

	_bool m_IsVisible = {};

#pragma region Condition 관리
protected:
	_uint m_iCondition = {}; // Condition;
	_float m_fMaxDissolveTime = { 0.5f };
	_float m_fDissolveTimer = {};

#pragma endregion



protected:
	void Bind_Resources();
	void Register_AllNotifies(const _string& strFolderPath);

public:
	static CProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) = 0;
	virtual	void Free() override;
};
NS_END

