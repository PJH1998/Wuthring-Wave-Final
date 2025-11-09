#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CRoverDarkWing final : public CProp
{
public:
	typedef struct tagRoverWingDesc : public CProp::PROP_DESC {
		
	} ROVER_PROP_DESC;

protected:
	explicit CRoverDarkWing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRoverDarkWing(const CPartObject& Prototype);
	virtual ~CRoverDarkWing() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;


private:
	vector<_uint> m_ShaderPaths = {};



private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Bind_Resources();

public:
	static CRoverDarkWing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

