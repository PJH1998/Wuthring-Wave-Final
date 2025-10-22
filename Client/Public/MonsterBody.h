#pragma once
#include "PartObject.h"
NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimMachine;
NS_END

NS_BEGIN(Client)
class CMonsterBody final : public CPartObject
{
public:
	typedef struct tagMonsterBodyDesc : public CPartObject::PART_DESC
	{
		_uint* pState;
		const _tchar* szPrototypeModelTag;
		const _char* pAnimationTag;
	}MONSTERBODY_DESC;
private:
	explicit CMonsterBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMonsterBody(const CMonsterBody& Prototype);
	virtual ~CMonsterBody() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg)override;
	virtual		void					Priority_Update(_float fTimeDelta)override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta)override;
	virtual		void					Render()override;

private:
	CShader* m_pShaderCom = {nullptr};
	CModel* m_pModelCom = {nullptr};
	CAnimMachine* m_pAnimMachineCom = {nullptr};

	_uint* m_pState = {nullptr};
	_bool					m_isAnimationFinished{};

private:
	void						Ready_Component(MONSTERBODY_DESC* pDesc);

public:
	static CMonsterBody* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
