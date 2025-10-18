#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CLayer final : public CBase
{
private:
	explicit CLayer();
	virtual ~CLayer() = default;

public:
	HRESULT					Add_GameObject(class CGameObject* pObject);
	class CComponent*	Get_Component(_uint iGameObjectIndex, const _wstring& strComponentTag);
	void						Change_TimeRatio(_float fTimeRatio) { m_fTimeRatio = fTimeRatio; }

public:
	void	Priority_Update(_float fTimeDelta);
	void	Update(_float fTimeDelta);
	void	Late_Update(_float fTimeDelta);

private:
	vector<class CGameObject*>	m_Objects;

	_float									m_fTimeRatio = {};

public:
	static		CLayer*	Create();
	virtual		void		Free() override;
};

NS_END