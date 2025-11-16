#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CAttackVolume final : public CGameObject
{
public:
	enum COMBINED_TYPE { BONE, PROP};
	typedef struct tagAttackVolumeDesc : public CGameObject::GAMEOBJECT_DESC
	{
		COMBINED_TYPE		eType{ COMBINED_TYPE::BONE };
		//Bone type : 뼈 매트릭스, Prop type : 장비 combined 매트릭스
		const _float4x4*	pSocketMatrix;
		CTransform*			pParenTransform;
		SHAPE				eShape;
		COLLISIONLAYER		eLayer;
		COLLISIONLAYER	eTargetLayer;
		vector<COLLISIONLAYER>	eTargetLayers;
		_float				fAttackDmg;
		_float3				vExtent;
		_float3				vOffsetPos;
		_float3				vOffsetRadian;
		function<void(_uint, void*, const ContactManifold&)> CollisionCallback;
		function<void(_uint, void*, const ContactManifold&, COLLISIONLAYER)> test;
		TEXT_COLOR_TYPE		eDamageType;
		ATTACKVOULME_DIR	eDir{};
		_string				strEffectTag{};
	}ATKVOLUME_DESC;

private:
	explicit CAttackVolume(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAttackVolume(const CAttackVolume& Prototype);
	virtual ~CAttackVolume() = default;

public:
	COLLISIONLAYER Get_Layer() const { return m_eCurrentLayer; }

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

public:
	// 절대 콜백 안에서 선언하지마.
	void TriggerActivate(_bool isActivate);
	void Change_Layer(COLLISIONLAYER eLayer);
	void Change_DIR(ATTACKVOULME_DIR eType);
	void Change_Desc(CALLBACK_CLIENT* pDesc);

private:
	COMBINED_TYPE		m_eType{ COMBINED_TYPE::BONE };
	const _float4x4*	m_pSocketMatrix = { nullptr };
	const _float4x4*	m_pCombinedMatrix = { nullptr };
	CTransform*			m_pParenTransform = { nullptr };
	_float4x4			m_CombinedMatrix{};
	CRigidbody*			m_pRigidBodyCom = { nullptr };
	ATTACKVOULME_DIR	m_eDirType = {};
#ifdef _DEBUG
	_float3			m_vOffsetPos{};
	_float3			m_vOffsetRot{};
#else
	_float4x4		m_OffsetMatrix{};
#endif

	vector<COLLISIONLAYER> m_eTargetLayer;
	COLLISIONLAYER m_eLayer{COLLISIONLAYER::NONE};
	COLLISIONLAYER m_eCurrentLayer{COLLISIONLAYER::NONE};

	CALLBACK_CLIENT			m_CallBack{};
	function<void(_uint, void*, const ContactManifold&)> m_CollisionCallback;
	function<void(_uint, void*, const ContactManifold&, COLLISIONLAYER)> m_test;
private:
	void Ready_Component(ATKVOLUME_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
//#ifdef _DEBUG
//	void OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
//#endif
public:
	static		CAttackVolume*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
