#pragma once
#include "StaticObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject final: public CStaticObject
{
public:
	enum OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, END };

	typedef struct tagMapLoad
	{

		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		OBJECTTYPE eObjectType;
		_uint iLevel = {};
	}MAP_LOAD;

private:
	explicit CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject(const CMapObject& Prototype);
	virtual ~CMapObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold){};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}
	virtual		BoundingBox* Get_BoundingBox()override;

private:
	CShader* m_pShaderCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	vector<CModel*> m_pModelComArray;

	_uint m_iShaderPassIndex = {};
private:
	void						Ready_Component(void* pArg);

public:
	static		CMapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END