#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CStaticObject abstract : public CGameObject
{
protected:
	explicit CStaticObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStaticObject(const CStaticObject& Prototype);
	virtual ~CStaticObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype() { return S_OK; };
	virtual		HRESULT		Initialize_Clone(void* pArg) { return __super::Initialize_Clone(pArg); };
	virtual		void			Priority_Update(_float fTimeDelta) {};
	virtual		void			Update(_float fTimeDelta) {};
	virtual		void			Late_Update(_float fTimeDelta) {};
	virtual		void			Render(_uint iLOD = 0) {};
	virtual		void			Render_Shadow() {};

protected:
	// LOD °³¼ö
	_uint		m_iNumLOD = {};

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END