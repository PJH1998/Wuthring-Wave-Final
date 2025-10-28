#pragma once
#include "PartObject.h"

NS_BEGIN(Client)
class CWeapon abstract : public CPartObject
{
public:
	typedef struct tagWeaponDesc : public CPartObject::PART_DESC 
	{
		const _float4x4* pSocketMatrix = { nullptr }; // ¿Â¬¯«“ ª¿
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> computeShaderData = {};
		pair<LEVEL, _wstring> modelData = {};
		pair<LEVEL, _wstring> rigidBodyData = {};
		_string strBoneName = {};
		WEAPONTYPE eWeaponType = { WEAPONTYPE::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};
	} WEAPON_DESC;

protected:
	explicit CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CWeapon(const CPartObject& Prototype);
	virtual ~CWeapon() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;


public:
	virtual void Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true); // Part Animation¿Ã ¿÷¿ª ∞ÊøÏ,
	void Set_SocketMatrix(const _float4x4* pSocketMatrix) { m_pSocketMatrix = pSocketMatrix; }

protected:
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CTransform* m_pParentTransform = { nullptr };
	WEAPONTYPE m_eWeaponType = { WEAPONTYPE::END };

	const _float4x4* m_pSocketMatrix = { nullptr };

	_float m_fTrackPosition = {};

	_bool m_IsAnimationEnd = { false };
private:
	void Bind_Resources();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) = 0;
	virtual	void Free() override;
};
NS_END

