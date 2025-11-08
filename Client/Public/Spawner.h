#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CSpawner final : public CGameObject
{
public:
	typedef struct tagSpanwerDesc
	{
		vector<_wstring> wstrPoolTags;
		vector<_float3>	vSpawnPositions;
		vector<_float3>	vSpawnRotateDegrees;
		_float3 vExtent;
		_float3 vPosition;
		_float	fSpawnTime;

	}SPAWNERDESC;

private:
	explicit CSpawner(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpawner(const CSpawner& Prototype);
	virtual ~CSpawner() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

private:
	CRigidbody*			m_pRigidBodyCom = { nullptr };
	vector<_wstring>	m_wstrPoolTags;
	vector<_float4x4>	m_SpawnMatrix;
	_uint				m_iNumSpawnObjects{};
	_float				m_fTimeAcc{};
	_float				m_fSpawnTime{};

private:
	void Ready_Component(SPAWNERDESC* pDesc);
	void OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CSpawner* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END
