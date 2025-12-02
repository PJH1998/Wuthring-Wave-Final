#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaFxObject final : public CProp
{
public:
	enum CHILD
	{
		BONE_001 = 0,
		BONE_002,
		BONE_003,
		BONE_004,
		BONE_005,
		BONE_006,
		CHILD_END
	};

public:
	typedef struct tagAugustaFxObjectDesc : public CProp::PROP_DESC {
		
	} AUGUSTA_FXOBJECT_DESC;

private:
	explicit CAugustaFxObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaFxObject(const CPartObject& Prototype);
	virtual ~CAugustaFxObject() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

public:
	virtual void Activate(_bool IsActivate) override;
	// Owner의 게이지 채우기?
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

	void Child_Activate(_bool IsActivate);

private:
	vector<_uint> m_ShaderPaths = {};
	vector<class CAugustaEnergyBlade*> m_EnergyBlades;
	class CRigidbody* m_pRigidbodyCom = { nullptr };

private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Ready_PartObjects(const PROP_DESC* pDesc);
	void Ready_AttackVolumes();
	void Bind_Resources();

public:
	static CAugustaFxObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

