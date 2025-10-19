#pragma once
#include "EditDummy.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Editor)

class CEditDummy_Augusta final : public CEditDummy
{
public:
	typedef struct tagDuumyAuguDesc : public DUMMY_DESC
	{
		_matrix PreTransformMatrix;
	}DUMMY_AUGU_DESC;

private:
	CEditDummy_Augusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditDummy_Augusta(const CEditDummy_Augusta& Prototype);
	virtual ~CEditDummy_Augusta() = default;

public:
	virtual	HRESULT		Initialize_Prototype() override;
	virtual	HRESULT		Initialize_Clone(void* pArg) override;
	virtual	void		Priority_Update(_float fTimeDelta) override;
	virtual	void		Update(_float fTimeDelta) override;
	virtual	void		Late_Update(_float fTimeDelta) override;
	virtual	void		Render() override;
	virtual	void		Render_Shadow() override;

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

private:
	HRESULT				Ready_Components(_fmatrix PreTransformMatrix);

public:
	static CEditDummy_Augusta*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END