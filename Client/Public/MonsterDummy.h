#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)

class CMonsterDummy final : public CGameObject
{
public:
	typedef struct tagMonsterDummyDesc {
		_matrix	PreTransformationMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		_float3	vPos = {};
	}MONSTER_DUMMY_DESC;
private:
	explicit CMonsterDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMonsterDummy(const CMonsterDummy& Prototype);
	virtual ~CMonsterDummy() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;
	virtual		void				Render_Shadow() override;
	virtual		void				Render_OutLine() override;

private:
	CShader*						m_pShaderCom = { nullptr };
	CModel*						m_pModelCom = { nullptr };
	CCollider*					m_pColliderCom = { nullptr };

	_float3						m_vPos = {};

private:
	void							Ready_Component(const _fmatrix& PreTransformMatrix);

public:
	static		CMonsterDummy*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void						Free() override;
};

NS_END